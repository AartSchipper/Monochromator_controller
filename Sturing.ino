/*
 * Sturing voor stappenmotor op monochromator
 * Aart 02-2018
 * 
 */

const int STEP = 3; // pin 
const int DIR = 2;  // pin
const float STEPS_NM = 96.2463; // steps per nm 
const int MIN_stepTime = 400; // min. 400 us between steps

const int UP = 1; // Direction
#define SIGNAL A0 // pin

void setup() { 
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite (DIR, !UP);
  
  pinMode(SIGNAL, INPUT);
  analogReference(DEFAULT); 
  Serial.begin(9600); 
  while (!Serial) { // wait
    } 
}

// the loop function runs over and over again forever
void loop() {
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
}

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


