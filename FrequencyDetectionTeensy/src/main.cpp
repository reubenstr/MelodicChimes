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

#include "Chime.h"

#include "ChimeStepper.h"

#define C 261.6
#define Cs 277.2
#define D 293.7
#define Eb 311.1
#define E 329.6
#define f 349.2
#define Fs 370.0
#define G 392.0
#define Gs 415.3
#define A 440.0

const int numNotes = 9;
float notes[numNotes] = {E, f, G, E, f, D, E, C, D};
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

#define PIN_STEPPER_TUNE_STEP 14
#define PIN_STEPPER_TUNE_DIRECTION 15

#define PIN_STEPPER_MUTE_STEP 18
#define PIN_STEPPER_MUTE_DIRECTION 19

#define PIN_MOTOR_PICK_PHASE 5
#define PIN_MOTOR_PICK_ENABLE 6
#define PIN_SWITCH_INDEX_MOTOR 2

#define PIN_SOLENOID_MUTE_1 7

#define PIN_LED_1 20
#define PIN_LED_2 21
#define PIN_LED_3 22
#define PIN_LED_4 23

#define MOTOR_PICK_INDEX_ACTIVATED LOW

Chime chime(PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION, PIN_MOTOR_PICK_PHASE, PIN_MOTOR_PICK_ENABLE, PIN_SWITCH_INDEX_MOTOR, PIN_SOLENOID_MUTE_1);

AudioInputAnalogStereo adcs1;
AudioAnalyzeNoteFrequency notefreq2;
AudioAnalyzeNoteFrequency notefreq1;
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);

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
/*
void CalibrateTiming()
{
    const int numNotes = 10;
    float freqCalibraion[numNotes] = {C, Cs, D, Eb, E, f, Fs, G, Gs, A};

    float targetFrequency = 0;
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

            chime.Pick();
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

                Serial.printf("Steps between notes: %u\n", chime._tuneStepper.TotalSteps() - stepsPrevious);

                times[noteSelect] = millis() - noteStartMillis;
                steps[noteSelect] = chime._tuneStepper.TotalSteps() - stepsPrevious;

                stepsPrevious = chime._tuneStepper.TotalSteps();
            }
        }
    }

    Serial.printf("\n\n");
    for (int i = 0; i < numNotes; i++)
    {
        if (i != 0)
            Serial.printf("Note %u from %3.2f to %3.2f with time %4ums | Steps: %u\n", i, freqCalibraion[i - 1], freqCalibraion[i], times[i], steps[i]);
    }

    Serial.printf("Total : %u\n", chime._tuneStepper.TotalSteps());
    Serial.printf("Finished calibration routine.\n\n\n");
}
*/

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


    ///////////////////////////////////////
    /*
    ChimeStepper stepper(PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION);

    stepper.SetCurrentPosition(0);
    stepper.SetTargetPosition(0);
    int count = 0;
    bool toggle = false;

    while (1)
    {

        if (stepper.IsAtPosition())
        {
            toggle = !toggle;
            if (toggle)
            {
                stepper.SetCurrentPosition(0);
                stepper.SetTargetPosition(100);
            }
            else
            {
                stepper.SetCurrentPosition(0);
                stepper.SetTargetPosition(-100);
            }
        }

        stepper.Tick();
    }
    /*
    ///////////////////////////////////////

/*
    while (1)
    {
        CalibrateTiming();
    }
    */

    /*  
    // Mute STEPPER TEST.
    ChimeStepper _stepper(PIN_STEPPER_MUTE_STEP, PIN_STEPPER_MUTE_DIRECTION);
    _stepper.SetCurrentPosition(0);
    _stepper.SetTargetPosition(50);
    _stepper.SetSpeed(2000);

    bool toggle = false;

    while (1)
    {
        if (_stepper.IsAtPosition())
        {

            toggle = !toggle;

            if (toggle)
            {
                _stepper.SetTargetPosition(200);
            }
            else
            {
                _stepper.SetTargetPosition(-200);
            }
        }

        _stepper.Tick();
    }
     */
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
    
      // STEPPER TESTS, STEPS TO FREQ
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

        About one hz per full step (in middle range notes).

    */

    static unsigned long startMute = millis();

    static float targetFrequency = E;
    float detectedFrequency = 0;
    detectedFrequency = GetFrequency();

    static unsigned long startPick = millis();

    static bool muteFlag = false;
    /* 
 
    if (millis() - startPick > 2000)
    {
        startPick = millis();
        Serial.printf("\nstartPick: *******\n");
        chime.Pick();

        startMute = millis();
        muteFlag = true;
    }

    if (millis() - startMute > 500)
    {
        startMute = millis();
        if (muteFlag)
        {
            muteFlag = false;
            chime.Mute();
        }
    }
  */
    //if (detectedFrequency > 0)
    {
        //Serial.printf("startPick: %u \t", millis() - startPick);
    }

    // Give time to string to cool down from pick as harsh picking causing spikes in frequency.
    //if (millis() - startPick > 50)
    {
        //startPick = 0;
        //chime.TuneFrequency(detectedFrequency, targetFrequency);
    }

    chime.TuneFrequency(detectedFrequency, targetFrequency);

    DebugLEDs();

    FlashOnboardLED();

    chime.Tick();

    if (millis() - startMute > 3000)
    {
        startMute = millis();

        chime.Mute();
    }
}