/*
 * Sturing voor stappenmotor op monochromator
 * Aart 02-2018
 * 
 */

const int STEP = 3; // pin 
const int DIR = 2;  // pin
const float STEPS_NM = 96.2463; // steps per nm 
const int MIN_stepTime = 390; // min. 400 us between steps
const int NORM_stepTime = 400; 

const int HOME = 9; // switch pin 
const float HomePos = 255.6; // in nm
const int homeStepTime = 500; // slow homing

const long minPos = HomePos * STEPS_NM; 
const long maxPos = 900 * STEPS_NM; 

// Communication
const int BYTES = 10; // Max command length

const int UP = 1; // Direction
#define SIGNAL A0 // pin

void setup() { 
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite (DIR, !UP);
  pinMode (HOME, INPUT_PULLUP); 
  
  pinMode(SIGNAL, INPUT);
  analogReference(DEFAULT); 
  Serial.begin(115200); 
  while (!Serial) { // wait for connection
    } 
  Serial.println(__DATE__);   
  Serial.println("Moving to home... "); 
  move_toHome(); 
  Serial.println("Monochromator controller. Use carriage return. Type '?' for help"); 
  
}

// the loop function runs over and over again forever
void loop() {
  readSerial(); 
  takeSamples(0); 
  doRun(0,0,0,0); 
}

void doRun(int beginWavelength, int endWavelength, int stepWavelength, int start) {
  static int state = 0; 
  static int localBeginWavelength, localEndWavelength, localStepWavelength, currentWavelength; 
  long samples;  
  
  if (beginWavelength > 0) localBeginWavelength = beginWavelength; 
  if (endWavelength > 0) localEndWavelength = endWavelength; 
  if (stepWavelength > 0) localStepWavelength = stepWavelength; 
  
  switch (state) {
    case 0: // wacht
      if (start) {
        if (localStepWavelength > 0 && localBeginWavelength > 0 && localEndWavelength > 0 ) {
          state = 1;
        } else {
           Serial.print("Run error: Not all parameters are set");
        }
      }
    break; 
    
    case 1: // ga naar begin
      gotoWavelength(localBeginWavelength); 
      currentWavelength = localBeginWavelength; 
      state = 2; 
    break; 
    
    case 2: // scan
      gotoWavelength(currentWavelength); 
      for (int i = 0; i < 10; i++ ) {
            samples += analogRead(SIGNAL);  
          }
      Serial.print(move_steps(0,0) / STEPS_NM); Serial.print(", "); Serial.println(samples / 10); 
      samples = 0; 
      currentWavelength += localStepWavelength; 
      if (currentWavelength > endWavelength) state = 3; 
    break; 

    case 3: // reset
      gotoWavelength(localBeginWavelength); 
      currentWavelength = localBeginWavelength; 
      state = 0; 
    break;  
  }
}

void takeSamples(int setSampleTime) {
    static int sampleTime = 0; 
    static long lastTime; 
    int samples; 
    if (setSampleTime > 0) sampleTime = setSampleTime; 
    if (setSampleTime < 0) sampleTime = 0; 
    if ( sampleTime > 0) {
       if (millis() - lastTime > sampleTime) {
          lastTime = millis();
          for (int i = 0; i < 10; i++ ) {
            samples += analogRead(SIGNAL);  
          }
          Serial.print(move_steps(0,0) / STEPS_NM); Serial.print(", "); Serial.println(samples / 10); 
          samples = 0; 
       }
    }
    return sampleTime; 
}

void readSerial() { 
  static char incomingBytes[BYTES];
  static int bytePos = 0;  
  if (Serial.available() > 0) {
    incomingBytes[bytePos] = Serial.read();
    bytePos++;
    if (bytePos > BYTES - 1) bytePos = 0;    
    if (incomingBytes[bytePos - 1] == '\r') {
      processIncoming(incomingBytes); 
      bytePos = 0; 
      for (int i = 0; i < BYTES; i++) {
        incomingBytes[i] = '\0';
      }
    }
  }  
}

void processIncoming(char incomingBytes[]) { // commmand processing
  int value;
  static int echo = 1; 
  
  if (echo) echoIncoming(incomingBytes); 
  
  switch (incomingBytes[0]) {
    case '?': 
      sendHelp(); 
    break; 
    
     case 'E':
      switch (incomingBytes[1]) { 
        case '1':
          Serial.println("Command echo on");
          echo = 1;
        break; 
        case '0':
          Serial.println("Command echo off");
          echo = 0;
        break; 
        default: 
        Serial.println("Unknown echo setting");
      }
    break; 
    
    case 'W':
      value = getValue (incomingBytes, 1); 
      Serial.print("Moving to: "); Serial.print((float) value / 10); Serial.println(" nm"); 
      gotoWavelength(value); 
    break; 

    case 'S':
       value = getValue (incomingBytes, 1) / 10; 
       Serial.print("Sample every: "); Serial.print(value); Serial.println(" ms"); 
       if (value < 1) value = -1; 
       takeSamples(value); 
    break;

    case 'R': // doRun(int beginWavelength, int endWavelength, int stepWavelength, int start)
       value = getValue (incomingBytes, 2); 
      switch (incomingBytes[1]) { 
        case 'B':
          doRun( value, 0,0,0);  
          Serial.print("Run starts at: "); Serial.print((float) value / 10); Serial.println(" nm");
        break; 
        case 'E': 
          doRun( 0, value,0,0);  
          Serial.print("Run ends at: "); Serial.print((float) value / 10); Serial.println(" nm");
        break;
        case 'S': 
          doRun( 0, 0,value,0);  
          Serial.print("Run step: "); Serial.print((float) value / 10); Serial.println(" nm");
        break; 
        case 'R':
          doRun(0,0,0,1); 
        break; 
      }
    break; 
    
    default: 
    Serial.println("Unknown command"); 
    
  }
  
}

int getValue(char inputArray[], int startAt) { // Return the value from part of an array with one decimal
  char valueArray[BYTES]; 
  float value; 
  for (int i = startAt; i < BYTES; i++) {
      valueArray[i-startAt] = inputArray[i]; 
  }
  value = atof(valueArray); 
  return value * 10; 
}

void echoIncoming (char incomingBytes[]) { // Echo commands
  Serial.print("> ");
  for (int i = 0; i < BYTES; i++) {
     Serial.print(incomingBytes[i]);  
  }
  Serial.println(" "); 
}

void sendHelp() {
  Serial.println(" Available commands: "); 
  Serial.println(" ?          - Show this help"); 
  Serial.println(" E1         - Command echo On"); 
  Serial.println(" E0         - Command echo Off"); 
  Serial.println(" Wnnn.n     - Goto wavelength in nm. Example: W500.0 or W500. Range: 0-1000 nm");
  Serial.println(" Snnn       - Take samples every nnn ms on one wavelength");  
  Serial.println("              One output sample is the average of 10 samples taken 1 ms apart for mains frequency suppression");  
  Serial.println(" S          - Stop stampling");  
  Serial.println(" RBnnn.n    - Set Run Begin wavelength in nm. Default 900"); 
  Serial.println(" REnnn.n    - Set Run End wavelength in nm. Default 300"); 
  Serial.println(" RSnnn.n    - Set Run step size in nm. Default 1"); 
  Serial.println(" R          - Run once and reset"); 
}

void gotoWavelength(int wavelength) {
  long currentPosition = move_steps(0,0);  
  move_steps( (( wavelength * STEPS_NM / 10) - currentPosition), NORM_stepTime );   
  Serial.print("Arrived at: "); Serial.print(move_steps(0,0) / STEPS_NM); Serial.println(" nm"); 
}

void move_toHome() {
  while (!digitalRead(HOME)) {
      digitalWrite (DIR, !UP);
      digitalWrite (STEP, HIGH);
      delayMicroseconds(homeStepTime);
      digitalWrite (STEP, LOW);
      delayMicroseconds(homeStepTime); 
  }
  while (digitalRead(HOME)) {
      digitalWrite (DIR, UP);
      digitalWrite (STEP, HIGH);
      delayMicroseconds(2 * homeStepTime);
      digitalWrite (STEP, LOW);
      delayMicroseconds(2 * homeStepTime); 
  }
}

long move_steps (long amount, int stepTime) { 
   static long current_position = minPos; 
   
   // Serial.print("Movement in steps: "); Serial.println(amount); 
   if (stepTime < MIN_stepTime) return current_position; // do not overspeed
   if ((current_position + amount) < minPos && amount < 0 )   return current_position; // do not underrun
   if ((current_position + amount) > maxPos && amount > 0 )   return current_position; // do not overrun
   
   current_position += amount;
   
   while (amount) {
    if (amount > 0) {
      digitalWrite (DIR, UP);
      digitalWrite (STEP, HIGH);
      delayMicroseconds(stepTime);
      digitalWrite (STEP, LOW);
      delayMicroseconds(stepTime); 
      amount--; 
    }

    if (amount < 0) {
      digitalWrite (DIR, !UP);
      digitalWrite (STEP, HIGH);
      delayMicroseconds(stepTime);
      digitalWrite (STEP, LOW);
      delayMicroseconds(stepTime); 
      amount++; 
    }
   }
    
  return current_position; 
}


