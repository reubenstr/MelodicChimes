// Melodic Chimes
//
// Auto-tuning string chimes controlled via MIDI file.
//
// PROTOTYPING

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <Servo.h>    // Built-in
#include <PWMServo.h> // Built-in

#include "movingAvg.h" // Arduino. Modifed to handle floats.

#include "Chime.h"

#include "QuickStepper.h"

#define C 261.6
#define Cs 277.2
#define D 293.7
#define Eb 311.1
#define E 329.6
#define F 349.2
#define Fs 370.0
#define G 392.0
#define Gs 415.3
#define A 440.0

const int numNotes = 9;
float notes[numNotes] = {E, F, G, E, F, D, E, C, D};
//float notesChromatic[10] = {C, Cs, D, Eb, E, F, Fs, G, Gs, A};
// int delays[numNotes] = {750, 750, 750, 750, 750, 750, 750, 750, 750};
// int delays[numNotes] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};

bool pickedFlag = false;

/*

DEVELOPMENT NOTES

defined AUDIO_GUITARTUNER_BLOCKS value of 24 changed to 6 for
faster analysis at the sacrafice of less lower frequency detection
which is not required for this application.

Blocks : lowest frequncy (rough estimate) : milliseconds per note
3 : 233hz : 9
4 : 174hz : 12
5 : 139hz : 15



A weak magnet for the coil is has a low signal to noise ratio.
A stronger magnet provides a high signal to noise ration.
An overly strong magnet over dampens the string vibrations reducing the length of the signal.


TeensyStep async is not working, also forum indicates changing 
parameters on the fly my not be supported

AccelStepper library is awkward and not intuitive when attempting 
to get async movements working. Basically just roll my own driving code
as I need only basic features (speeds and modest acceleration).


DC-Motor

Fast motor (N25 500RPM) : fast but not precise

Slow motor (N20 50RPM) : slow but precise

Precision due to the delay of frequency detections allow for more in sync timing tuning and motor turn off.


*/

#define PIN_MOTOR_TUNE_PWM1 9
#define PIN_MOTOR_TUNE_PWM2 11
#define PIN_MOTOR_TUNE_ENABLE 10

#define PIN_STEPPER_TUNE_STEP 14
#define PIN_STEPPER_TUNE_DIRECTION 15

#define PIN_MOTOR_PICK_PHASE 5
#define PIN_MOTOR_PICK_ENABLE 6

#define PIN_SWITCH_INDEX_MOTOR 2

#define PIN_SOLENOID_MUTE_1 7

#define PIN_LED_1 20
#define PIN_LED_2 21
#define PIN_LED_3 22
#define PIN_LED_4 23

Chime chime(PIN_MOTOR_TUNE_PWM1, PIN_MOTOR_TUNE_PWM2, PIN_MOTOR_PICK_PHASE, PIN_MOTOR_PICK_ENABLE, PIN_SWITCH_INDEX_MOTOR, PIN_SOLENOID_MUTE_1);

bool direction = false;
signed long position = 0;
int speed;

PWMServo servo3;

QuickStepper stepper(PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION);

/**/
AudioInputAnalogStereo adcs1;
AudioAnalyzeNoteFrequency notefreq2;
AudioAnalyzeNoteFrequency notefreq1;
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);

movingAvg movingAverage(5);

enum PickSide
{
    Left,
    Right
} pickSide;

//////////////////////////////
// Motor
// Direction of the motor refers to the pitch
// increasing or decreasing.

unsigned long startMillis;
unsigned int runTime;
bool runFlag;

//////////////////////////////

enum State
{
    sPick,
    sMute,
    sPretune
};

////////////////////////////////////////////////////////////////////////

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

// On error, halt program.
void Halt()
{
    Serial.println("Halted!");
    while (1)
    {
    }
}

void DebugLEDs()
{
    /**/
    // Debug LEDs: show motor direction when motor is in motion.

    digitalWrite(PIN_LED_1, digitalRead(PIN_STEPPER_TUNE_STEP));
    digitalWrite(PIN_LED_2, digitalRead(PIN_STEPPER_TUNE_DIRECTION));
}
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
float GetFrequency()
{
    float acceptableProbability = 0.995;
    static int timeElapsed = millis();

    static float min = 2048;
    static float max = 0;

    if (notefreq1.available())
    {
        float frequency = notefreq1.read();
        float probability = notefreq1.probability();

        if (probability > acceptableProbability)
        {
            if (frequency > max)
                max = frequency;

            if (frequency < min)
                min = frequency;

            // Serial.printf("Detected: %3.2f | Probability %1.2f | Time: %ums \n", note, prob, millis() - timeElapsed);
            // Serial.printf("Min %3.2f | Max: %3.2f | Delta: %3.2f | Count %u\n", min, max, max - min, count);

            timeElapsed = millis();

            return frequency;
        }

        timeElapsed = millis();
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////

// Blocking routine to aquire timing parameters.
void CalibrateTiming()
{

    const int numNotes = 10;
    float freqCalibraion[numNotes] = {C, Cs, D, Eb, E, F, Fs, G, Gs, A};

    float targetFrequency;
    int noteSelect = 0;
    unsigned long noteStartMillis = 5000;
    bool noteFinishedFlag = true;
    int stepsPrevious = 0;

    int times[10];
    for (int i = 0; i < 10; i++)
    {
        times[i] = 0;
    }

    int steps[10];
    for (int i = 0; i < 10; i++)
    {
        steps[i] = 0;
    }

    while (noteSelect < numNotes + 1)
    {

        if (millis() - noteStartMillis >= 2000)
        {
            noteStartMillis = millis();
            targetFrequency = freqCalibraion[noteSelect];

            // Pick();

            noteFinishedFlag = false;

            Serial.printf("New target note (%u) at %3.2f\n", noteSelect, targetFrequency);

            noteSelect++;
        }

        DebugLEDs();

        float detectedFrequency = GetFrequency();
        float frequencyTolerance = 1.0;

        // Give time to string to cool down from pick as harsh picking causing spikes in frequency.
        if (millis() - noteStartMillis > 50)
        {
            chime.TuneFrequency(detectedFrequency, targetFrequency);
        }

        chime.Tick();

        if ((detectedFrequency > (targetFrequency - frequencyTolerance)) && (detectedFrequency < (targetFrequency + frequencyTolerance)))
        {
            if (!noteFinishedFlag)
            {
                noteFinishedFlag = true;
                Serial.printf("Frequency (%u) within tolerance, elapsed time: %u\n", noteSelect, millis() - noteStartMillis);

                Serial.printf("Steps between notes: %u\n", chime._stepper.TotalSteps() - stepsPrevious);

                times[noteSelect] = millis() - noteStartMillis;
                steps[noteSelect] = chime._stepper.TotalSteps() - stepsPrevious;

                stepsPrevious = chime._stepper.TotalSteps();
            }
        }
    }

    Serial.printf("\n\n");
    for (int i = 0; i < numNotes; i++)
    {
        if (i != 0)
            Serial.printf("Note %u from %3.2f to %3.2f with time %4ums | Steps: %u\n", i, freqCalibraion[i - 1], freqCalibraion[i], times[i], steps[i]);
    }

      Serial.printf("Total : %u\n", chime._stepper.TotalSteps());
    Serial.printf("Finished calibration routine.\n\n\n");
}

////////////////////////////////////////////////////////////////////////
void setup()
{
    delay(1000); // Give platformio time to switch back to the terminal.

    Serial.begin(115200);
    Serial.println("Teensy starting up.");

    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_LED_3, OUTPUT);
    pinMode(PIN_LED_4, OUTPUT);

    AudioMemory(120);
    notefreq1.begin(.15);
    // notefreq2.begin(.25);

    /*
    // Manual motor test: tune motor
    analogWriteFrequency(PIN_MOTOR_TUNE_ENABLE, 500);
    //analogWriteFrequency(PIN_MOTOR_TUNE_PWM1, 1000);
    //analogWriteFrequency(PIN_MOTOR_TUNE_PWM2, 1000);
    pinMode(PIN_MOTOR_TUNE_PWM1, OUTPUT);
     pinMode(PIN_MOTOR_TUNE_PWM2, OUTPUT);

    // TESTING 3 pin configuration: EN (PWM) / IN(1 or 0) / IN(1  or 0)
    while (1)
    {

        analogWrite(PIN_MOTOR_TUNE_ENABLE, 80);
        digitalWrite(PIN_MOTOR_TUNE_PWM1, HIGH);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, LOW);
        DebugLEDs();
        delay(1000);

        analogWrite(PIN_MOTOR_TUNE_ENABLE, 200);
        digitalWrite(PIN_MOTOR_TUNE_PWM1, LOW);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, HIGH);
        DebugLEDs();
        delay(1000);
    }
    */

    /*
    while (1)
    {
   
        digitalWrite(PIN_MOTOR_TUNE_PWM1, HIGH);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, LOW);
        //digitalWrite(PIN_MOTOR_TUNE_PWM1, LOW);
        //digitalWrite(PIN_MOTOR_TUNE_PWM2, HIGH);

        DebugLEDs();
        delay(5);

        digitalWrite(PIN_MOTOR_TUNE_PWM1, HIGH);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, HIGH);
        DebugLEDs();
        delay(500);

    /*
        digitalWrite(PIN_MOTOR_TUNE_PWM1, LOW);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, HIGH);
        DebugLEDs();
        delay(50);

        digitalWrite(PIN_MOTOR_TUNE_PWM1, LOW);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, LOW);
        DebugLEDs();
        delay(500);
        
    }
    */

    /*   */
    while (1)
    {
        CalibrateTiming();
    }

    //////////////////////////////////////////////////////
    // STEPS TO FREQ TEST
    /*
90° - 221hz to 280hz = delta 59hz
280 334 = 54
332 373 = 41
371 401 = 30
starting to skips steps at 1/2 step 1000us step delay

// 1/2 step with 100 steps
262 - 309 = 47 hz -> 0.94 steps per hz
309 - 365 = 56 hz -> 1.12 steps per hz
364 - 315 = 49 hz -> 0.98 steps per hz
314 - 353 = 39 hz -> 0.78 steps per hz

About one hz per full step.

*/

    /* 
    QuickStepper stepper(PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION);

    stepper.SetCurrentPosition(0);
    stepper.SetTargetPosition(0);
    int count = 0;

    while (1)
    {
        float detectedFrequency = GetFrequency();
        if (detectedFrequency != 0)
        {
            Serial.printf("Frequency: %3.2f\n", detectedFrequency);

            if (count++ == 10)
            {
                stepper.SetTargetPosition(100);
                Serial.println("**** Stepper start");
            }
        }
        // Prevent over tension.
        if (detectedFrequency > 475)
        {
            stepper.Stop();
        }
        stepper.Tick();
    }
   */

    //////////////////////////////////////////////////////
    /* 
    // SPEED TEST between two frequencies
    bool flag = true;
    while (flag)
    {
        float detectedFrequency = GetFrequency();
        static float targetFrequency = 300;

        static unsigned long start = millis();
        static bool toggle = false;
        if (millis() - start > 4000)
        {
            start = millis();
            toggle = !toggle;

            if (toggle)
            {
                targetFrequency = 340;
            }
            else
            {
                targetFrequency = 440;
            }
        }

        chime.TuneFrequency(detectedFrequency, targetFrequency);
        chime.Tick();
    }
    */

    /////////////////////////////
    // STEPPER TEST
    /*
    while (1)
    {

        static unsigned long start = millis();
        static bool toggle = false;
        if (millis() - start > 2000)
        {
            start = millis();
            toggle = !toggle;

            stepper.SetCurrentPosition(0);

            if (toggle)
            {
                Serial.println("- position");
                stepper.SetTargetPosition(-81);
            }
            else
            {
                Serial.println("+ position");
                stepper.SetTargetPosition(81);
            }
        }

        digitalWrite(PIN_LED_1, digitalRead(PIN_STEPPER_TUNE_STEP));
        digitalWrite(PIN_LED_2, digitalRead(PIN_STEPPER_TUNE_DIRECTION));

        stepper.Tick();
    }
*/
    /////////////////////////////
}

void loop()
{

    //////////////////////////////////////////////////////////////////////////////////////////
    // NOTES AREA
    //////////////////////////////////////////////////////////////////////////////////////////
    /* 
    Turn tests: start, turn, test, turn, end
    400.03 - full - 246.60 - full - 399.67
    427.02 - full - 272.89 - full - 429.36

    A full rotation roughly spans 8 notes.

    Estimated Hz per rotation: 153.78
                    per degre: 0.4271

    0.12815 hz / millisecond (estimated from full turn test)
    0.05714 hz / millisecond (estimated from note to note test (with pickStall)) ~350ms motor time per note.


    TEST MOTOR:
    6v, 50RPM (no load), 24oz/in (crude test), 1200ms per revolution. 
    Pololu's N20 6v (low power) 54RPM has a oz/in of 24 which is on par with out test motor.

    The current torque is capable of turning the peg (of our current test string (high E)).

    Test indicate time between notes ~350ms (average), but high notes (A440) take 900ms
    meaning the current motor does not have enough torque.

    ---
    POSSIBLE MOTOR:


    */

    //////////////////////////////////////////////////////////////////////////////////////////
    // DC-Motor TEST AREA
    //////////////////////////////////////////////////////////////////////////////////////////

    static bool doOnce = true;
    if (doOnce)
    {
        doOnce = false;
        // CalibrateTiming();
    }

    static float targetFrequency = 329.6;

    /*      */
    static int times[10];
    for (int i = 0; i < 10; i++)
    {
        times[i] = 0;
    }

    static int noteSelect = 0;
    static unsigned long noteStartMillis;
    static bool noteFinishedFlag = true;

    if (millis() - noteStartMillis >= 2000)
    {
        noteStartMillis = millis();
        targetFrequency = notes[noteSelect];
        //Pick();
        Serial.printf("New target: %3.2f\n", targetFrequency);
        noteSelect++;
        if (noteSelect == numNotes)
        {
            noteSelect = 0;
        }
    }

    DebugLEDs();

    /*
// PICK MOTOR TEST

    static unsigned long startindex = millis();
    static unsigned long startDebounce;
    static bool indexFlag = false;
    static bool indexResetFlag = false;

    if (digitalRead(PIN_SWITCH_INDEX_MOTOR) == MOTOR_PICK_INDEX_ACTIVATED)
    {

        if (indexResetFlag)
        {
            Serial.printf("Index elapsed %u\n", millis() - startindex);

            indexResetFlag = false;
            startDebounce = millis();
            startindex = millis();
        }
    }

    if (millis() - startDebounce > 200)
    {
        if (digitalRead(PIN_SWITCH_INDEX_MOTOR) != MOTOR_PICK_INDEX_ACTIVATED)
        {
            indexResetFlag = true;
        }
    }
    */

    float detectedFrequency = GetFrequency();

    static unsigned long startPick = millis();
    if (millis() - startPick > 1000)
    {
        startPick = millis();
        // chime.Pick();
    }

    static unsigned long startMute = millis();
    if (millis() - startMute > 1000)
    {
        startMute = millis();
        // chime.Mute();
    }

    // Give time to string to cool down from pick as harsh picking causing spikes in frequency.
    //if (millis() - noteStartMillis > 50)
    {
        chime.TuneFrequency(detectedFrequency, targetFrequency);
    }

    chime.Tick();

    FlashOnboardLED();
}