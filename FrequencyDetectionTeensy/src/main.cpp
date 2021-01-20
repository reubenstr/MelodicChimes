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
#include <JC_Button.h> // https://github.com/JChristensen/JC_Button

#include "movingAvg.h"

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


Button buttonStep(0);


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
*/

#define PIN_STEPPER_TUNE_STEP 14
#define PIN_STEPPER_TUNE_DIRECTION 15
#define PIN_STEPPER_PICK_STEP 18
#define PIN_STEPPER_PICK_DIRECTION 19
#define PIN_STEPPER_MUTE_STEP 9
#define PIN_STEPPER_MUTE_DIRECTION 8

Chime chime(PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION,
            PIN_STEPPER_PICK_STEP, PIN_STEPPER_PICK_DIRECTION,
            PIN_STEPPER_MUTE_STEP, PIN_STEPPER_MUTE_DIRECTION);

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
    float freqCalibraion[numNotes] = {C, Cs, D, Eb, E, f, Fs, G, Gs, A};

    float targetFrequency = 0;
    signed int noteSelect = 0;
    unsigned long noteStartMillis = 5000;
    bool noteFinishedFlag = true;
    int stepsPrevious = 0;

    int times[10];
    int steps[10];
    for (int i = 0; i < 10; i++)
    {
        times[i] = 0;
        steps[i] = 0;
    }

    while (noteSelect < numNotes + 1)
    {

        if (millis() - noteStartMillis >= 2000)
        {
            noteStartMillis = millis();
            targetFrequency = freqCalibraion[noteSelect];
            noteSelect++;

            chime.Pick();
            noteFinishedFlag = false;
            Serial.printf("New target note (%u) at %3.2f\n", noteSelect, targetFrequency);
        }

        DebugLEDs();

        float detectedFrequency = GetFrequency();
        float frequencyTolerance = 1.0;

        // Give time to string to cool down from pick as harsh picking causing spikes in frequency.
        //if (millis() - noteStartMillis > 50)
        //{
        chime.TuneFrequency(detectedFrequency, targetFrequency);
        //}

        chime.Tick();

        if ((detectedFrequency > (targetFrequency - frequencyTolerance)) && (detectedFrequency < (targetFrequency + frequencyTolerance)))
        {
            if (!noteFinishedFlag)
            {
                noteFinishedFlag = true;
                Serial.printf("Frequency %3.2f to %3.2f within tolerance, elapsed time: %u\n", freqCalibraion[noteSelect - 1], freqCalibraion[noteSelect], millis() - noteStartMillis);

                //Serial.printf("Steps between notes: %u\n", chime._tuneStepper.TotalSteps() - stepsPrevious);

                times[noteSelect] = millis() - noteStartMillis;
                //steps[noteSelect] = chime._tuneStepper.TotalSteps() - stepsPrevious;

                //stepsPrevious = chime._tuneStepper.TotalSteps();
            }
        }
    }

    Serial.printf("\n\n");
    for (int i = 0; i < numNotes; i++)
    {
        if (i != 0)
            Serial.printf("Note %u from %3.2f to %3.2f with time %4ums | Steps: %u\n", i, freqCalibraion[i - 1], freqCalibraion[i], times[i], steps[i]);
    }

    //Serial.printf("Total : %u\n", chime._tuneStepper.TotalSteps());
    Serial.printf("Finished calibration routine.\n\n\n");
}
/**/

void CalibratePick()
{
    bool freqencyDetectedFlag = false;
    while (!freqencyDetectedFlag)
    {
        freqencyDetectedFlag = GetFrequency() > 0;
        chime.CalibratePick(freqencyDetectedFlag);
    }
}

////////////////////////////////////////////////////////////////////////
void setup()
{
    delay(1000); // Give platformio time to switch back to the terminal.

    Serial.begin(115200);
    Serial.println("Teensy starting up.");

    pinMode(LED_BUILTIN, OUTPUT);

    AudioMemory(120);
    notefreq1.begin(.15);
    // notefreq2.begin(.25);

    /*
    ///////////////////////////////////////
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

    ///////////////////////////////////////
    // Calibrate pick
    /*
    CalibratePick();
    Serial.println("Finished");

    unsigned long startx = millis();
    while (1)
    {

        if (millis() - startx > 3000)
        {
            startx = millis();
            chime.Pick();
        }

        chime.Tick();
    }
    */

    /*
    // Back and forth test.
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
    */

    ///////////////////////////////////////
    // Hz per n steps test.
    // 1564 millis from 270hz to 440hz with 387 seconds (all approximate readings).
    // avg of 37.5 rpm (includes accleration and deacceleration)
    /*
    avg.begin();
    buttonStep.begin();
    while (1)
    {
        buttonStep.read();

        if (buttonStep.wasPressed())
        {
            chime.Step(387);
            Serial.println("Step");
        }

        float detectedFrequency = GetFrequency();

        if (detectedFrequency > 0)
        {
            avg.reading(detectedFrequency);
            Serial.printf("Freq: %3.2f | Avg: %3.2f | %u\n", detectedFrequency, avg.getAvg(), millis());
        }

        chime.Tick();
    }
    */

    /*
   // HOLD TARGET TEST
    while (1)
    {
        float targetFrequency = f;
        float detectedFrequency = GetFrequency();
        chime.TuneFrequency(detectedFrequency, targetFrequency); 
        chime.Tick();
    }
    */

    ///////////////////////////////////////
    // CALIBRATION TEST

    while (1)
    {
        CalibrateTiming();
    }

    ///////////////////////////////////////
}

void loop()
{

    static unsigned long startMute = millis();

    static float targetFrequency = E;
    float detectedFrequency = 0;
    detectedFrequency = GetFrequency();

    static unsigned long startPick = millis();

    static bool muteFlag = false;

    // Give time to string to cool down from pick as harsh picking causing spikes in frequency.
    //if (millis() - startPick > 50)
    {
        //startPick = 0;
        //chime.TuneFrequency(detectedFrequency, targetFrequency);
    }

    chime.TuneFrequency(detectedFrequency, targetFrequency);

    FlashOnboardLED();

    chime.Tick();

    if (millis() - startMute > 3000)
    {
        startMute = millis();

        chime.Mute();
    }
}