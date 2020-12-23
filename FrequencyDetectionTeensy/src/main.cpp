#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "TeensyStep.h" // https://github.com/luni64/TeensyStep
//#include <Servo.h>
#include <PWMServo.h> // Built-in

// defined AUDIO_GUITARTUNER_BLOCKS value of 24 changed to 6 for
// faster analysis at the sacrafice of less lower frequency detection
// which is not required for this application.

#define PIN_SERVO_3 6

Stepper motor(A0, A1);  // (STEP Pin  DIR Pin)
StepControl controller; // Use default settings

PWMServo servo3;

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
        if (prob > 0.99)
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

void setup()
{
    delay(1000); // Give platformio time to switch back to the terminal.

    Serial.begin(115200);
    Serial.println("Teensy starting up.");

    pinMode(LED_BUILTIN, OUTPUT);

    AudioMemory(120);
    notefreq1.begin(.15);
    // notefreq2.begin(.25);

    motor.setMaxSpeed(800);            // 800
    motor.setPullInOutSpeed(100, 100); // 100, 100
    motor.setAcceleration(2500);       // 2500
    //motor.setInverseRotation(true);

    servo3.attach(PIN_SERVO_3, 1000, 2000);
}

void loop()
{
    FlashOnboardLED();

    static float targetFrequency = 350;
    static float currentFrequency = 0;
    static int attemptCount = 0;
    static bool activeFrequencyFlag = false;

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

            motor.setTargetRel(correction);
        }
        else
        {
            motor.setTargetRel(0);
            integral = 0;
        }
    }

    controller.moveAsync(motor);

    // delay(3000);
    // Pick();
    //delay(250);
    // Mute();

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

    /* */
}