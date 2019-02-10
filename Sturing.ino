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

void processIncoming(char incomingBytes[BYTES]) {
  int value;
  char valueArray[10]; 
  
  Serial.print("> "); // Echo input
  for (int i = 0; i < BYTES; i++) {
     Serial.print(incomingBytes[i]);  
  }
  Serial.println(" "); 
  
  for (int i = 1; i < BYTES; i++) {
    valueArray[i-1] = incomingBytes[i]; 
  }
  value = atoi(valueArray); 
  Serial.println(value); 
  
  switch (incomingBytes[0]) {
    case '?': 
      sendHelp(); 
    break; 
    case 'W':
      gotoWavelength(); 
    break; 
    default: 
    Serial.println("Meh"); 
    
  }
  
}

void sendHelp() {
  Serial.println(" Commands: "); 
  Serial.println(" ? - Show this help"); 
  Serial.println(" W - Goto wavelength. W500.0 or W500");
}

void gotoWavelength() {
 
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


