/**
 * Modified from original ArduMower code (www.ardumower.de).
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define NUM_OF_SENSORS 2
#define ZERO_POINT_CYCLES 4
#define TOUCH_POINT_CYCLES 4

const int sensorInPins[] = {A0, A1};  // Analog input pin that the MPX5010 is attached to
const int bumperOutPins[] = {9, 8};
const int LEDCollisionPins[] = {12, 10};
const int LEDActivePin = 11;

int LEDActiveState = LOW;             // ledState used to set the LED

int sensorDiffs[] = {0, 0};  // = sensorDiffSecond - sensorDiffFirst
int sensorDiffFirst[] = {0, 0};        // value for zero Point
int sensorDiffSecond[] = {0, 0};        // value for touch Point
int sensorTrigger[] = {3, 3};  // Sensor-Trigger-Level Sensor 1
int triggerCounter[] = {0, 0}; // Trigger-Counter Sensor 1
byte sensorState[] = {0, 0}; // Sensor-State Sensor 1 zero point or touch point
byte countFirstRead[] = {0, 0}; // Read-Counter for zero point Sensor 1
byte countSecondRead[] = {0, 0}; // Read-Counter for touch point Sensor 1
byte flagAdcRead[] = {0, 0}; // Flag its Time to read ADC Sensor 1

unsigned long previousMillis = 0;        // will store last time Timer was updated
const long interval = 15;           // interval at which ADC was read (milliseconds)

const long intervalLedActive = 250; // interval at which LED was blink (milliseconds)
unsigned long previousMillisLedActive = 0;


void setup() {
  int i;
  Serial.begin(115200);

  for (i = 0; i < NUM_OF_SENSORS; i++) {
    pinMode(LEDCollisionPins[i], OUTPUT);
    pinMode(bumperOutPins[i], OUTPUT);
    digitalWrite (LEDCollisionPins[i], HIGH);
  }
      
  pinMode(LEDActivePin, OUTPUT);
  digitalWrite(LEDActivePin,HIGH);
  
  delay(2000);

  digitalWrite(LEDActivePin,LOW);

  for (i = 0; i < NUM_OF_SENSORS; i++) {
    digitalWrite(LEDCollisionPins[i], LOW);
    digitalWrite(bumperOutPins[i], LOW);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  int index;

  // ----------------------------------------------- active LED blink ---------------
  if (currentMillis - previousMillisLedActive >= intervalLedActive) {
    previousMillisLedActive = currentMillis;

    if (LEDActiveState == LOW) {
      LEDActiveState = HIGH;
    } else {
      LEDActiveState = LOW;
    }

    digitalWrite (LEDActivePin, LEDActiveState);
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    int i;
    for (i = 0; i < NUM_OF_SENSORS; i++) {
      flagAdcRead[i] = 1;
    }
  }
  
  for (index = 0; index < NUM_OF_SENSORS; index++) {    
    // --------------------------------------------------------------------------------
    // ------------------------- Calculate Zero-Point ---------------------------------
    if (sensorState[index] < 1) {
      if ((flagAdcRead[index] == 1) &&  (countFirstRead[index] < ZERO_POINT_CYCLES)) {    // read the analog in value: sensorDiffFirst
        sensorDiffFirst[index] += analogRead (sensorInPins[index]);
        countFirstRead[index]++;
        flagAdcRead[index] = 0;
      }
      if (countFirstRead[index] >= ZERO_POINT_CYCLES - 1) {
        sensorDiffFirst[index] /= ZERO_POINT_CYCLES;
        sensorState[index] = 1;
      }
    }
  
    // --------------------------------------------------------------------------------
    // ------------------------- Calculate Touch-Point --------------------------------
    if (sensorState[index] >= 1) {
      if ((flagAdcRead[index] == 1) && (countSecondRead[index] < TOUCH_POINT_CYCLES)) { // read the analog in value: sensorDiffSecond
        sensorDiffSecond[index] += analogRead(sensorInPins[index]);
        countSecondRead[index]++;
        flagAdcRead[index] = 0;
      }
      if(countSecondRead[index] >= TOUCH_POINT_CYCLES - 1) {
        sensorDiffSecond[index] /= TOUCH_POINT_CYCLES;
        sensorState[index] = 2;
      }
    }
  
    // --------------------------------------------------------------------------------
    // ------------------------- Calculate Zero-Touch-Diff ----------------------------
    if (sensorState[index] == 2) {
      sensorDiffs[index] = sensorDiffSecond[index] - sensorDiffFirst[index];
      sensorDiffSecond[index] = 0;
      countSecondRead[index] = 0;
      sensorState[index] = 3;
    }
  
    // --------------------------------------------------------------------------------
    // -------------------- Check Zero-Touch-Diff for Trigger -------------------------
    if ((sensorDiffs[index] >= sensorTrigger[index]) & (sensorState[index] == 3)) {
      digitalWrite (LEDCollisionPins[index], HIGH);
      digitalWrite (bumperOutPins[index], HIGH);
      triggerCounter[index]++;
      Serial.print ("counter ");
      Serial.print (index + 1);
      Serial.print (" = " );
      Serial.print (triggerCounter[index]);
      Serial.print ("\t sensor dif 1 = ");
      Serial.println (sensorDiffs[index]);
    } else if (sensorState[index] == 3) {
      sensorState[index] = 0;
      sensorDiffFirst[index] = 0;
      sensorDiffSecond[index] = 0;
      countSecondRead[index] = 0;
      countFirstRead[index] = 0;
      digitalWrite (LEDCollisionPins[index], LOW);
      digitalWrite (bumperOutPins[index], LOW);
    }
  }
}

