// Melodic Chimes
//
// Auto-tuning string chimes control via MIDI
// 
// PROTOTYPING


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <PWMServo.h> // Built-in

/*

DEVELOPMENT NOTES

defined AUDIO_GUITARTUNER_BLOCKS value of 24 changed to 6 for
faster analysis at the sacrafice of less lower frequency detection
which is not required for this application.

A weak magnet for the coil is has a low signal to noise ratio.
A stronger magnet provides a high signal to noise ration.
An overly strong magnet over dampens the string vibrations reducing the length of the signal.


TeensyStep async is not working, also forum indicates changing 
parameters on the fly my not be supported

AccelStepper library is awkward and not intuitive when attempting 
to get async movements working. Basically just roll my own driving code
as I need only basic features (speeds and modest acceleration).

*/

#define PIN_MOTOR_PHASE 0
#define PIN_MOTOR_ENABLE 1

#define PIN_LED_1 20
#define PIN_LED_2 21
#define PIN_LED_3 22
#define PIN_LED_4 23

#define PIN_SERVO_3 6

#define STEP A0
#define DIR A1

bool direction = false;
signed long position = 0;
int speed;

PWMServo servo3;

/**/
AudioInputAnalogStereo adcs1;
AudioAnalyzeNoteFrequency notefreq2;
AudioAnalyzeNoteFrequency notefreq1;
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);

enum PickSide
{
    Left,
    Right
} pickSide;

// Direction of the motor refers to the pitch
// increasing or decreasing.
enum Direction
{
    Up,
    Down
};

/**/
float GetFrequency()
{
    static float min = 2048;
    static float max = 0;
    static int count = 0;
    static int timeElapsed1 = millis();

    if (notefreq1.available())
    {
        float note = notefreq1.read();
        float prob = notefreq1.probability();

        //if (prob >= 0.99)
        if (prob > 0.995)
        {
            count++;

            if (note > max)
                max = note;

            if (note < min)
                min = note;

            Serial.printf("Note 1 (%U): %3.2fhz | Probability: %.2f\n", millis() - timeElapsed1, note, prob);
            // Serial.printf("Min %3.2f | Max: %3.2f | Delta: %3.2f | Count %u\n", min, max, max - min, count);
            timeElapsed1 = millis();

            return note;
        }

        timeElapsed1 = millis();
    }

    return 0;
}

/**/
void Pick()
{
    if (pickSide == Left)
    {
        pickSide = Right;
        servo3.write(60);
    }
    else if (pickSide == Right)
    {
        pickSide = Left;
        servo3.write(100);
    }
}

void Mute()
{
    if (pickSide == Left)
    {
        servo3.write(80);
    }
    else if (pickSide == Right)
    {
        servo3.write(80);
    }
}

// Flash onboard LED to show activity
void FlashOnboardLED()
{
    static unsigned long start = millis();
    if (millis() - start > 50)
    {
        start = millis();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}

void Halt()
{
    Serial.println("Halted!");
    while (1)
    {
    }
}

// DC-Motor code:
void SetMotorDirection(Direction direction)
{
    if (direction == Direction::Up)
        digitalWrite(PIN_MOTOR_PHASE, true);
    if (direction == Direction::Down)
        digitalWrite(PIN_MOTOR_PHASE, false);
}

void EnableMotor(bool enable)
{
    digitalWrite(PIN_MOTOR_ENABLE, enable);
}

/*
STEPPER CODE
void SetDirection(bool dir)
{
    direction = dir;
    digitalWrite(DIR, direction);
}

bool Run()
{
    static unsigned long startMicros = micros();

    //int delayMicros = (1.0 / 100.0) * 1000000;
    int delayMicros = 5000;

    if (micros() - startMicros > delayMicros)
    {
        startMicros = micros();

        if (direction)
        {
            position++;
        }
        else
        {
            position--;
        }

        digitalWrite(STEP, HIGH);
        delayMicroseconds(5);
        digitalWrite(STEP, LOW);

        return true;
    }

    return false;
}
*/

void setup()
{
    delay(1000); // Give platformio time to switch back to the terminal.

    Serial.begin(115200);
    Serial.println("Teensy starting up.");

    pinMode(LED_BUILTIN, OUTPUT);

    // pinMode(STEP, OUTPUT);
    // pinMode(DIR, OUTPUT);

    digitalWrite(PIN_MOTOR_PHASE, LOW);
    digitalWrite(PIN_MOTOR_ENABLE, LOW);

    pinMode(PIN_MOTOR_PHASE, OUTPUT);
    pinMode(PIN_MOTOR_ENABLE, OUTPUT);

    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_LED_3, OUTPUT);
    pinMode(PIN_LED_4, OUTPUT);

    AudioMemory(120);
    notefreq1.begin(.15);
    // notefreq2.begin(.25);

    servo3.attach(PIN_SERVO_3, 1000, 2000);

    position = 0;
    //SetDirection(true);
}

void loop()
{

    static float targetFrequency = 329.63;
    static float currentFrequency = 0;
    static int attemptCount = 0;
    static bool activeFrequencyFlag = false;
    static unsigned int noteCount;

    /*    

    Turn tests: start, turn, test, turn, end
    400.03 - full - 246.60 - full - 399.67
    427.02 - full - 272.89 - full - 429.36

    Estimated Hz per rotation: 153.78
                    per degre: 0.4271
    */

    GetFrequency();

    //////////////////////////////////////////////////////////////////////////////////////////
    // DC-Motor TEST AREA
    //////////////////////////////////////////////////////////////////////////////////////////

    /*
   // Debug LEDs: show motor direction when motor is in motion.
    if (digitalRead(PIN_MOTOR_ENABLE))
    {
        digitalWrite(PIN_LED_1, digitalRead(PIN_MOTOR_PHASE));
        digitalWrite(PIN_LED_2, !digitalRead(PIN_MOTOR_PHASE));
    }
    else
    {
         digitalWrite(PIN_LED_1, LOW);
        digitalWrite(PIN_LED_2, LOW);
    }
    

    static unsigned long startPick = millis();
    if (millis() - startPick >= 500)
    {
        if (millis() - startPick != 500)
        {
            Serial.printf("********* Pick : %u\n", millis() - startPick);
        }

        startPick = millis();
        Pick();
    }

    static int noFreqCount = 0;
    static unsigned long start = millis();
    if (millis() - start >= 20)
    {
        start = millis();

        float returnFrequency = GetFrequency();
        if (returnFrequency > 0)
        {
           noteCount++;     
            noFreqCount = 0;
            currentFrequency = returnFrequency;
            activeFrequencyFlag = true;
        }
        else // No frequency detected
        {
            noFreqCount++;
            if (noFreqCount != 0)
            {
                if (activeFrequencyFlag)
                {
                    Serial.println("activeFrequencyFlag = false");
                }

                activeFrequencyFlag = false;
            }
        }

        if (activeFrequencyFlag)
        {
            EnableMotor(true);

            Serial.printf("%5u: Target %3.2f | Current: %3.2f | Error: %3.2f \n", noteCount, targetFrequency, currentFrequency, targetFrequency - currentFrequency);

            if (currentFrequency < targetFrequency)
            {
                SetMotorDirection(Direction::Up);
            }
            else if (currentFrequency > targetFrequency)
            {
                SetMotorDirection(Direction::Down);
            }
        }
        else
        {
            EnableMotor(false);
        }
    }

*/

    //////////////////////////////////////////////////////////////////////////////////////////
    // STEPPER TEST AREA
    //////////////////////////////////////////////////////////////////////////////////////////

    /* 
    if (position < 200)
    {
        if (Run())
        {
            Serial.printf("Position: %i\n", position);
        }
    } */

    /*
    float returnFrequency = GetFrequency();
    if (returnFrequency > 0)
    {
        activeFrequencyFlag = true;
        attemptCount = 0;
        currentFrequency = returnFrequency;
    }
    else
    {
        if (attemptCount++ > 10)
        {
            //Halt();
            if (activeFrequencyFlag)
                Serial.println("activeFrequencyFlag = false");
            activeFrequencyFlag = false;
        }
    }
    */

    /*
        Stepper PI Loop

        Driver microstepping at: 1/4 step

    // PI controller
    // motor 1 is the master
    // motor 2 is the slave
    // Traverse: motor 2 corrects to motor encoder ticks
    // Turn: motor 2 corrects to error in yaw sensor
    float error = 0;
    static float integral = 0;
    static signed int correction = 0;
    float pTerm = 0.80;
    float iTerm = 0.10;

    static int samCount = 0;
    static double acc = 0;

    static unsigned long start = millis();
    if (millis() - start > 20)
    {
        start = millis();

        float returnFrequency = GetFrequency();
        if (returnFrequency > 0)
        {
            currentFrequency = returnFrequency;
            activeFrequencyFlag = true;
        }
        else
        {
            if (activeFrequencyFlag)
            {
                Serial.println("activeFrequencyFlag = false");
            }

            activeFrequencyFlag = false;
        }

        if (activeFrequencyFlag)
        {
            error = currentFrequency - targetFrequency;
            integral = integral + error;
            correction = (pTerm * error) + (iTerm * integral);
            Serial.printf("Target %3.2f | Current: %3.2f | Error: %3.2f (%3.0f) | Integral: %3.0f (%3.0f) | Correction %i\n", targetFrequency, currentFrequency, error, pTerm * error, integral, integral * iTerm, correction);

            acc += currentFrequency;
            samCount++;
            Serial.printf("Avg: %3.2f\n", acc / samCount);
            //motor.setTargetRel(correction);
        }
        else
        {
            //motor.setTargetRel(0);
            integral = 0;

            acc = 0;
            samCount = 0;
        }
    }
      */

    //////////////////////////////////////////////////////////////////////////////////////////
    // GENERAL TEST AREA
    //////////////////////////////////////////////////////////////////////////////////////////

    /*
  static int timeElapsed2 = millis();
  if (notefreq2.available())
  {
    float note2 = notefreq2.read();
    float prob2 = notefreq2.probability();
    //Serial.printf("Note 2 (%U): %3.2fhz | Probability: %.2f\n", millis() - timeElapsed2, note2, prob2);
    timeElapsed2 = millis();
  }
  */

    FlashOnboardLED();
}