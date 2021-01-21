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

Button buttonStep(2);

movingAvg avg(20);

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
    float notes[numNotes] = {C, Cs, D, Eb, E, f, Fs, G, Gs, A};

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
            targetFrequency = notes[noteSelect];
            noteSelect++;

            //chime.Pick();
            noteFinishedFlag = false;
            Serial.printf("New target note (%u) at %3.2f\n", noteSelect, targetFrequency);
        }

        float detectedFrequency = GetFrequency();
        float frequencyTolerance = 1.0;

        // Give time to string to cool down from pick as harsh picking causing spikes in frequency.
        //if (millis() - noteStartMillis > 50)
        {
            chime.TuneFrequency(detectedFrequency, targetFrequency);
        }

        chime.Tick();

        if ((detectedFrequency > (targetFrequency - frequencyTolerance)) && (detectedFrequency < (targetFrequency + frequencyTolerance)))
        {
            if (!noteFinishedFlag)
            {
                noteFinishedFlag = true;
                Serial.printf("Frequency %3.2f to %3.2f within tolerance, elapsed time: %u\n", notes[noteSelect - 1], notes[noteSelect], millis() - noteStartMillis);

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
            Serial.printf("Note %u from %3.2f to %3.2f with time %4ums | Steps: %u\n", i, notes[i - 1], notes[i], times[i], steps[i]);
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

bool IsFrequencyWithinTolerance(float frequency1, float frequency2, float tolerance)
{
    return fabs(frequency1 - frequency1) < tolerance;
}

void CalibrateSteps()
{

    /*
    200 steps/rev, 1/2 step, 2:1 pulley ratio 
    200 * 2 * 0.5

*/

    Serial.println("Calibrate Steps between notes...");

    AccelStepper stepper(AccelStepper::DRIVER, PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION);
    stepper.setPinsInverted(true, false, false);
    stepper.setMaxSpeed(100);
    stepper.setAcceleration(100);

    // Tune starting frequency.
    Serial.println("Tune starting frequency.");
    float targetFrequency = C;
    float detectedFrequency = 0;
    float frequencyTolerance = 1;

    const int msBetweenSteps = 50;
    unsigned long start = millis() + 1000;
    unsigned long frequencyDetectionTimeoutMillis = 0;
    unsigned long stepMillis = 0;

    movingAvg averageFreq(5);
    averageFreq.begin();

    int noteCount = 0;
    while (noteCount < numNotes)
    {

        targetFrequency = notes[noteCount];
        Serial.printf("Target frequency: %3.2f.\n", targetFrequency);

        while (fabs(detectedFrequency - targetFrequency) > frequencyTolerance)
        {

            if (millis() - frequencyDetectionTimeoutMillis > 1000)
            {
                frequencyDetectionTimeoutMillis = millis();
                //chime.Pick();
            }

            /*
            if (millis() - stepMillis > msBetweenSteps)
            {
                stepMillis = millis();

                stepper.setCurrentPosition(0);
                stepper.moveTo(1);
                stepper.runToPosition();

                Serial.printf("Detected frequency: %3.2f\n", detectedFrequency);
            }
            */

            detectedFrequency = GetFrequency();

            if (detectedFrequency > 0)
            {
                frequencyDetectionTimeoutMillis = millis();
            }

            chime.TuneFrequency(detectedFrequency, targetFrequency);
            chime.Tick();
        }

        Serial.printf("Target frequency of %3.2f met with detected frequency of %3.2f\n", targetFrequency, detectedFrequency);

        noteCount++;
    }

    Serial.printf("Test complete\n");

    while (1)
    {
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

    ///////////////////////////////////////
    // CALIBRATION TEST
    /*
    while (1)
    {
        CalibrateTiming();
    }
*/

    /*    ///////////////////////////////////////
        // MOVE TO POSITION 
    AccelStepper stepper(AccelStepper::DRIVER, PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION);
    
    stepper.setMaxSpeed(50000);
    stepper.setAcceleration(10000);

    int count = 0;
    bool toggle = false;

    stepper.setCurrentPosition(0);
    stepper.moveTo(40);

    unsigned long start = millis();
    int position = 0;

    while (stepper.isRunning())
    {       

        if (position != stepper.currentPosition())
        {
            position = stepper.currentPosition();
            Serial.println(stepper.speed());
        }


        stepper.run();
    }   
   
   Serial.print("Elasped time: ");
   Serial.println(millis() - start);

   while(1)
   {
         float detectedFrequency = GetFrequency();
         if (detectedFrequency > 0)
         {
             Serial.println(detectedFrequency);
         } 
   }
   */

    ///////////////////////////////////////
    // Hz per n steps test.
    // 1564 millis from 270hz to 440hz with 387 seconds (all approximate readings).
    // avg of 37.5 rpm (includes accleration and deacceleration)
    /*
    AccelStepper stepper(AccelStepper::DRIVER, PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION);

    stepper.setMaxSpeed(50000);
    stepper.setAcceleration(10000);

    avg.begin();
    buttonStep.begin();
    int stepsTaken = 0;
    while (1)
    {
        buttonStep.read();

        if (buttonStep.wasPressed())
        {
            const int stepsToTake = 1;
            stepsTaken += stepsToTake;
            stepper.moveTo(stepper.currentPosition() + stepsToTake);
            Serial.printf("Steps taken: %u | Steps to take: %u\n", stepsTaken, stepsToTake);
        }

        float detectedFrequency = GetFrequency();

        if (detectedFrequency > 0)
        {
            avg.reading(detectedFrequency);
            Serial.printf("Freq: %3.2f | Avg: %3.2f | %u\n", detectedFrequency, avg.getAvg(), millis());
        }

        stepper.run();
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

    CalibrateSteps();
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