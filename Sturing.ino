/*
 * Sturing voor stappenmotor op monochromator
 * Aart 02-2018
 * 
 */

const int STEP = 3;
const int DIR = 2; 
const float STEPS_NM = 96.2463;

const int UP = 1; // langere golflengte = DIR 
#define SIGNAL A0

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
 int adc; 
 for (int i = 0; i < 100; i++) {
   Serial.print("Meting:"); Serial.println(i); 
   
   int current_position = move_steps(-110);  
   Serial.print("Positie: "); Serial.println(current_position); 
   Serial.print("Golflengte: "); Serial.println(current_position / STEPS_NM); 
   
   delay(10); 
   
   adc = analogRead(SIGNAL);
   Serial.print(adc); Serial.println(","); 
 }
  while(1); 
}


int move_steps (int amount) {
   static int current_position = 200 * STEPS_NM; 
   
   current_position += amount;
   
   while (amount) {
    if (amount > 0) {
      digitalWrite (DIR, UP);
      digitalWrite (STEP, HIGH);
      delay(1);
      digitalWrite (STEP, LOW);
      delay(1); 
      amount--; 
    }

    if (amount < 0) {
      digitalWrite (DIR, !UP);
      digitalWrite (STEP, HIGH);
      delay(1);
      digitalWrite (STEP, LOW);
      delay(1); 
      amount++; 
    }
   }
    
  return current_position; 
}


