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
#include "Chime.h"     // Local class.
#include <JC_Button.h> // https://github.com/JChristensen/JC_Button
#include "movingAvg.h" // Local class.

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

    Tuning:
    Two types of tuning tested: free tuning, direct steps tuning
        Direct step tuning (using a look up table for steps between notes) was tested to allow for maximuim
        stepper speed due to free tuning tending to deaccelerate early. Testing revealed the steps between
        notes varied between 20% to 50% between each test run (testing the amount of steps between frequencies within tolerance).
        The variation was further confirmed when attemping to tune using the steps to note lookup table. Some occasions, 
        while tuning, only a few steps were needed for final tune correction while on most occasions the steps required
        where relatively many.

*/

#define PIN_STEPPER_TUNE_STEP 14
#define PIN_STEPPER_TUNE_DIRECTION 15
#define PIN_STEPPER_PICK_STEP 18
#define PIN_STEPPER_PICK_DIRECTION 19
#define PIN_STEPPER_MUTE_STEP 9
#define PIN_STEPPER_MUTE_DIRECTION 8

const int numChimes = 1;
Chime chimes[1] = {Chime(0, PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION,
                         PIN_STEPPER_PICK_STEP, PIN_STEPPER_PICK_DIRECTION,
                         PIN_STEPPER_MUTE_STEP, PIN_STEPPER_MUTE_DIRECTION)};

AudioInputAnalogStereo adcs1;
AudioAnalyzeNoteFrequency notefreq2;
AudioAnalyzeNoteFrequency notefreq1;
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);

//float detectedFrequencies[numChimes];
volatile float frequencyFromSerial[2];

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
float GetFrequency(int freqNoteSelect)
{
    const float noFrequencyDetected = 0;
    const float acceptableProbability = 0.995;
    static int timeElapsed = millis();

    static float min = 2048;
    static float max = 0;

    if (freqNoteSelect != 0 && freqNoteSelect != 1)
    {
        return noFrequencyDetected;
    }

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

    return noFrequencyDetected;
}

float GetDetectedFrequency(int sourceId)
{
    if (sourceId == 0)
    {
        return GetFrequency(0);
    }

    // detectedFrequencies[1] = GetFrequency(1);
    //cli(); // Disable interrupts;
    //detectedFrequencies[3] = frequencyFromSerial[0];
    //detectedFrequencies[4] = frequencyFromSerial[1];
    //sei();	// Enable interrupts;
}

void CalibratePick()
{
    /*
    bool freqencyDetectedFlag = false;
    while (!freqencyDetectedFlag)
    {
        freqencyDetectedFlag = GetFrequency() > 0;
        chime.CalibratePick(freqencyDetectedFlag);
    }
    */
}

bool IsFrequencyWithinTolerance(float frequency1, float frequency2, float tolerance)
{
    return fabs(frequency1 - frequency1) < tolerance;
}

////////////////////////////////////////////////////////////////////////
void setup()
{
    // Give platformio time to switch back to the terminal.
    delay(1000);

    Serial.begin(115200);
    Serial.println("Teensy starting up.");

    pinMode(LED_BUILTIN, OUTPUT);

    AudioMemory(120);
    notefreq1.begin(.15);
    // notefreq2.begin(.25);

    //////////////////////////    
    int c = 0;
    bool completeFlag = false;
    chimes[c].PrepareFrequencyPerStep();
    while (!completeFlag)
    {
        completeFlag = chimes[c].CalibrateFrequencyPerStep(GetDetectedFrequency(c));
    }

    while (true)
    {
    }
    //////////////////////////

    //////////////////////////
    // Calibration tests
    /*
    int c = 0;
    bool completeFlag = false;

    chimes[c].PrepareCalibratePick();
    while (!completeFlag)
    {
        completeFlag = chimes[c].CalibratePick(GetDetectedFrequency(c));
    }

    completeFlag = false;

    chimes[c].PrepareCalibrateStepsToNotes();
    while (!completeFlag)
    {
        completeFlag = chimes[c].CalibrateStepsToNotes(GetDetectedFrequency(c));
    }
    */
    //////////////////////////

    //////////////////////////
/*
    unsigned long start = millis();
    int c = 0;
    int n = 62;
    bool pickFlag = false;
    unsigned long pickMillis = millis();
    while (true)
    {

        chimes[c].TuneNote(GetDetectedFrequency(c), n);
        chimes[c].Tick();

        if (millis() - start > 2000)
        {
            start = millis();
            n++;
            if (n == 70)
            {
                Serial.println("STOPPED");
                while (true)
                {
                }
            }

            Serial.printf("New note: %u\n", n);
            pickFlag = true;
            pickMillis = millis();
        }

        // Delay pick for n milliseconds.
        if (pickFlag && millis() - pickMillis > 100)
        {
            chimes[c].Pick();
            pickFlag = false;
        }
    }
    //////////////////////////
    */
}

void loop()
{
    FlashOnboardLED();

    /*
    float targetFrequency = C;

    for (int c = 0; c < numChimes; c++)
    {
        chimes[c].TuneFrequency(GetDetectedFrequency(c), targetFrequency);
    }

    for (int c = 0; c < numChimes; c++)
    {
        chimes[c].Tick();
    }
    */
}