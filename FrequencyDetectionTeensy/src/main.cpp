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

const int numNotes = 10;
//float notes[numNotes] = {E, F, G, E, F, D, E, C, D};
float notesChromatic[10] = {C, Cs, D, Eb, E, F, Fs, G, Gs, A};
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

*/

#define PIN_MOTOR_PHASE 0
#define PIN_MOTOR_ENABLE 1

#define PIN_LED_1 20
#define PIN_LED_2 21
#define PIN_LED_3 22
#define PIN_LED_4 23

//#define PIN_SERVO_3 3

#define PIN_MOTOR_PICK_PHASE 5
#define PIN_MOTOR_PICK_ENABLE 6
#define PIN_SWITCH_INDEX_MOTOR 2

#define MOTOR_PICK_INDEX_ACTIVATED 1

Chime chime(PIN_MOTOR_PHASE, PIN_MOTOR_ENABLE);

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
}
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
void Pick(bool homeFlag = false)
{

    if (homeFlag)
    {
        pickSide = Left;
        servo3.write(100);
        return;
    }

    pickedFlag = true;

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

    static int times[10];
    for (int i = 0; i < 10; i++)
    {
        times[i] = 0;
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
            chime.FrequencyToMotor(detectedFrequency, targetFrequency);
        }

        chime.Tick();

        if ((detectedFrequency > (targetFrequency - frequencyTolerance)) && (detectedFrequency < (targetFrequency + frequencyTolerance)))
        {
            if (!noteFinishedFlag)
            {
                noteFinishedFlag = true;
                Serial.printf("Frequency (%u) within tolerance, elapsed time: %u\n", noteSelect, millis() - noteStartMillis);

                times[noteSelect] = millis() - noteStartMillis;
            }
        }
    }

    Serial.printf("\n\n");
    for (int i = 0; i < numNotes; i++)
    {
        if (i != 0)
            Serial.printf("Note %u from %3.2f to %3.2f with time %4u\n", i, freqCalibraion[i - 1], freqCalibraion[i], times[i]);
    }

    Serial.printf("Finished calibration routine.\n\n\n");
}

////////////////////////////////////////////////////////////////////////
void setup()
{
    delay(1000); // Give platformio time to switch back to the terminal.

    Serial.begin(115200);
    Serial.println("Teensy starting up.");

    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(PIN_MOTOR_PHASE, LOW);
    digitalWrite(PIN_MOTOR_ENABLE, LOW);

    pinMode(PIN_MOTOR_PHASE, OUTPUT);
    pinMode(PIN_MOTOR_ENABLE, OUTPUT);

    pinMode(PIN_MOTOR_PICK_PHASE, OUTPUT);
    pinMode(PIN_MOTOR_PICK_ENABLE, OUTPUT);
    pinMode(PIN_SWITCH_INDEX_MOTOR, INPUT_PULLUP);

    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_LED_3, OUTPUT);
    pinMode(PIN_LED_4, OUTPUT);

    AudioMemory(120);
    notefreq1.begin(.15);
    // notefreq2.begin(.25);

    //servo3.attach(PIN_SERVO_3, 1000, 2000);

    movingAverage.begin();

    // Start in known position.
    //Pick(true);
    //delay(2000);

    /*

    digitalWrite(PIN_MOTOR_PICK_PHASE, HIGH);
    digitalWrite(PIN_MOTOR_PICK_ENABLE, HIGH);

    while (1)
    {
        if (digitalRead(PIN_SWITCH_INDEX_MOTOR) == MOTOR_PICK_INDEX_ACTIVATED)
        {
            digitalWrite(PIN_MOTOR_PICK_ENABLE, LOW);
            delay(500);
            digitalWrite(PIN_MOTOR_PICK_ENABLE, HIGH);
            delay(150);
        }
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

    /*  
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
        Pick();    
        Serial.printf("New target: %3.2f\n", targetFrequency);
        noteSelect++;
        if (noteSelect == numNotes)
        {
             noteSelect = 0;
        }
    }
    */

    DebugLEDs();

    digitalWrite(PIN_MOTOR_PICK_PHASE, HIGH);
    digitalWrite(PIN_MOTOR_PICK_ENABLE, HIGH);

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
    
 

    float detectedFrequency = GetFrequency();
    float frequencyTolerance = 1.0;

    // Give time to string to cool down from pick as harsh picking causing spikes in frequency.
    //if (millis() - noteStartMillis > 50)
    {
        chime.FrequencyToMotor(detectedFrequency, targetFrequency);
    }

    chime.Tick();

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////

    FlashOnboardLED();
}