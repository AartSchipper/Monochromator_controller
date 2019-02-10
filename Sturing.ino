/*
 * Sturing voor stappenmotor op monochromator
 * Aart 02-2018
 * 
 */

const int STEP = 3; // pin 
const int DIR = 2;  // pin
const float STEPS_NM = 96.2463; // steps per nm 
const int MIN_stepTime = 400; // min. 400 us between steps

// Communication
const int BYTES = 10; // Max command length

const int UP = 1; // Direction
#define SIGNAL A0 // pin

void setup() { 
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite (DIR, !UP);
  
  pinMode(SIGNAL, INPUT);
  analogReference(DEFAULT); 
  Serial.begin(115200); 
  while (!Serial) { // wait
    } 
  Serial.println(__DATE__);   
  Serial.println("Monochromator control. Use carriage return. Type '?' for help"); 
}

// the loop function runs over and over again forever
void loop() {
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

void processIncoming(char incomingBytes[]) {
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
    
    default: 
    Serial.println("Unknown command"); 
    
  }
  
}

int getValue(char inputArray[], int startAt) {
  char valueArray[BYTES]; 
  float value; 
  for (int i = startAt; i < BYTES; i++) {
      valueArray[i-startAt] = inputArray[i]; 
  }
  value = atof(valueArray); 
  return value * 10; 
}

void echoIncoming (char incomingBytes[]) {
  Serial.print("> ");
  for (int i = 0; i < BYTES; i++) {
     Serial.print(incomingBytes[i]);  
  }
  Serial.println(" "); 
}

void sendHelp() {
  Serial.println(" Available commands: "); 
  Serial.println(" ?  - Show this help"); 
  Serial.println(" E1 - Command echo On"); 
  Serial.println(" E0 - Command echo Off"); 
  Serial.println(" W  - Goto wavelength in nm. Example: W500.0 or W500. Range: 0-1000 nm");
}

void gotoWavelength(int wavelength) {
 
}
  
  
 /*
 for (int i = 0; i < 100; i++) {
   Serial.print("Meting:"); Serial.println(i); 
   
   int current_position = move_steps(-110, 400);  
   Serial.print("Positie: "); Serial.println(current_position); 
   Serial.print("Golflengte: "); Serial.println(current_position / STEPS_NM); 
   
   delay(10); 

   int adc; 
   adc = analogRead(SIGNAL);
   Serial.print(adc); Serial.println(","); 
 }
  while(1); 
  */

int move_steps (int amount, int stepTime) { 
   static int current_position = 200 * STEPS_NM; 

   if (stepTime < MIN_stepTime) return current_position; // do not overspeed
   
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


