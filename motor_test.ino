#define stepPin 2
#define dirPin 3
#define enablePin 4
#define speedPin 5
#define calibratePin 6
#define resetPin 7

#define RESET 0
#define REWINDING 1
#define ACTIVE 2
#define CALIBRATE 3

#define DIR_OPEN HIGH
#define DIR_CLOSE LOW
#define SPEED_SLOW HIGH
#define SPEED_FAST LOW

#import "lookup.h"

unsigned long MAX_STEPS = 218000;

// initial state is rewinding
unsigned int state = REWINDING;
unsigned long steps = 0;
unsigned long start_time = 0;
//unsigned long MAX_STEPS = 5012; // for fast testing

void setup()
{
  // then we set it HIGH so that the board is disabled until we
  // get into a known state.
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);

  Serial.begin(9600);
  Serial.println("Starting stepper exerciser.");

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(speedPin, OUTPUT);
  
  pinMode(calibratePin, INPUT);
  digitalWrite(calibratePin, HIGH); // pull up resistor
  
  pinMode(resetPin, INPUT);
  digitalWrite(resetPin, HIGH); // pull up resistor
}

void loop()
{  
  // set the enablePin low so that we can now use our stepper driver.
  // wait a few microseconds for the enable to take effect
  // (That isn't in the spec sheet I just added it for sanity.)
  digitalWrite(enablePin, LOW);

  int calibrationButtonState = digitalRead(calibratePin);
  int resetButtonState = digitalRead(resetPin);

  //Serial.println("LOOP");
  //Serial.println(calibrationButtonState);
  //Serial.println(resetButtonState);

  switch(state) {
    case REWINDING:
      // when the calibration button is low, we've hit the end
      if (calibrationButtonState == LOW) {
        Serial.println("rewind -> reset");
        reset();
      } else {
        rewind();
      }
      break;

    case RESET:
      if (resetButtonState == LOW) {
        Serial.println("reset -> active");
        advance();
        delay(1);
      }
      break;

    case ACTIVE:
      if (resetButtonState == LOW) {
        Serial.println("active -> rewind");
        rewind();
      } else {
        advance();
      }
      break;
     
    case CALIBRATE:
      calibrate();
  }
}

void advance() {
  if (state != ACTIVE) {
    state = ACTIVE;
    start_time = millis();
  }

  digitalWrite(dirPin, DIR_OPEN);
  digitalWrite(speedPin, SPEED_SLOW);
  
  if (steps < MAX_STEPS) {
    unsigned long delta = millis() - start_time;
    unsigned int index = floor(steps / STEPS_PER_ENTRY);
    unsigned int remainder = steps % STEPS_PER_ENTRY;
    unsigned int degrees_in_current_index = ANGLE_LOOKUP[index + 1] - ANGLE_LOOKUP[index];

    Serial.println("delta: ");
    Serial.println(delta);

    Serial.println("index: ");
    Serial.println(index);

    Serial.println("taken: ");
    Serial.println(steps);
  
    Serial.println("remainder: ");
    Serial.println(remainder);

    Serial.println("degrees_in_current_index: ");
    Serial.println(degrees_in_current_index);

    double degrees_traveled = double(ANGLE_LOOKUP[index]) + degrees_in_current_index * double(remainder) / double(STEPS_PER_ENTRY);
    double degrees_needed = delta * degrees_per_millisecond - degrees_traveled;
    
    unsigned int steps_needed = floor((degrees_needed / degrees_in_current_index) * double(STEPS_PER_ENTRY));

    Serial.println("traveled: ");
    Serial.println(degrees_traveled);

    Serial.println("needed: ");
    Serial.println(degrees_needed);

    Serial.println("steps to take: ");
    Serial.println(steps_needed);
    
    //delay(1000);

    step_motor(steps_needed);
  }
}

void reset() {
  state = RESET;
  steps = 0;
  digitalWrite(stepPin, LOW);
  digitalWrite(dirPin, DIR_OPEN);
  digitalWrite(speedPin, SPEED_SLOW);
}

void rewind() {
  state = REWINDING;
  digitalWrite(dirPin, DIR_CLOSE);
  digitalWrite(speedPin, SPEED_FAST);
  step_motor(1);  
}

void step_motor(unsigned int step_count) {
  for (unsigned int i = 0; i < step_count; i++) {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(2);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    steps += 1;
  }
}

void calibrate() {
  step_motor(128);
  Serial.println("STEP");
  Serial.println(ANGLE_LOOKUP[steps]);
  delay(200);
}
