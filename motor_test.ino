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

// initial state is rewinding
unsigned int state = REWINDING;
unsigned long revolutions = 0;
unsigned long MAX_REVOLUTIONS = 218000;
//unsigned long MAX_REVOLUTIONS = 5012; // for fast testing

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

      //delay(1000);
      //state = CALIBRATE;
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
  state = ACTIVE;
  digitalWrite(dirPin, DIR_OPEN);
  digitalWrite(speedPin, SPEED_SLOW);
  
  if (revolutions < MAX_REVOLUTIONS) {
    step_motor();
    revolutions += 1;
  }
}

void reset() {
  state = RESET;
  revolutions = 0;
  digitalWrite(stepPin, LOW);
  digitalWrite(dirPin, DIR_OPEN);
  digitalWrite(speedPin, SPEED_SLOW);
}

void rewind() {
  state = REWINDING;
  digitalWrite(dirPin, DIR_CLOSE);
  digitalWrite(speedPin, SPEED_FAST);
  step_motor();  
}

void step_motor() {
  digitalWrite(stepPin, LOW);
  delayMicroseconds(2);
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(1000);
}

void calibrate() {
  for (int i = 0; i < 128; i++) {
    step_motor();
    revolutions += 1;
  }

  //Serial.println("STEP");
  Serial.println(ANGLE_LOOKUP[revolutions]);

  delay(200);
}
