/*
 * Sturing voor stappenmotor op monochromator
 * Aart 02-2018
 * 
 */

const int STEP = 3;
const int DIR = 2; // in Fullstep: 208 nm / 10000 stappen = 0.0208 nm per stap
const int UP = 1; // langere golflengte = DIR 
#define SIGNAL A0

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite (DIR, UP);
  
  pinMode(SIGNAL, INPUT);
  analogReference(DEFAULT); 
  Serial.begin(9600); 
}

// the loop function runs over and over again forever
void loop() {
 int adc; 
 for (int i = 0; i < 2000; i++) {
   move_10(); 
   adc = analogRead(SIGNAL);
   Serial.print(adc); Serial.println(","); 
 }
  while(1); 
}

void move_10() { 
  for (int i = 0; i < 10; i++) {
    digitalWrite (STEP, HIGH);
    delay(10);
    digitalWrite (STEP, LOW);
    delay(10); 
  }
}
