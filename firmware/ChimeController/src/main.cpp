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
#include "Chime.h"           // Local class.
#include "movingAvg.h"       // Local class.
#include <JC_Button.h>       // https://github.com/JChristensen/JC_Button
#include "TeensyTimerTool.h" // https://github.com/luni64/TeensyTimerTool

using namespace TeensyTimerTool;
Timer t1; // generate a timer from the pool (Pool: 2xGPT, 16xTMR(QUAD), 20xTCK)

AudioInputAnalogStereo adcs1;
AudioAnalyzeNoteFrequency notefreq2;
AudioAnalyzeNoteFrequency notefreq1;
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);

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
	An overly strong magnet dampens the string vibrations reducing the length of the signal.
    Tuning:
    Two types of tuning tested: free tuning, direct steps tuning
        Direct step tuning (using a look up table for steps between notes) was tested to allow for maximuim
        stepper speed due to free tuning tending to deaccelerate early. Testing revealed the steps between
        notes varied between 20% to 50% between each test run (testing the amount of steps between frequencies within tolerance).
        The variation was further confirmed when attemping to tune using the steps to note lookup table. Some occasions, 
        while tuning, only a few steps were needed for final tune correction while on most occasions the steps required
        where relatively many.
*/

// Note: each chime will have a unique id which indicates the note range and EEPROM storage location for config params.
// Select the controller's chimes.
#define CHIME_SET_1_AND_2
// #define CHIME_SET_3_AND_4

#if defined CHIME_SET_1_AND_2
#define CHIME_A_ID 1
#define CHIME_B_ID 2
#elif defined CHIME_SET_3_AND_4
#define CHIME_A_ID 3
#define CHIME_B_ID 4
#endif

#define PIN_STEPPER_TUNE_STEP_A 0
#define PIN_STEPPER_TUNE_DIRECTION_A 6
#define PIN_STEPPER_TUNE_STEP_B 1
#define PIN_STEPPER_TUNE_DIRECTION_B 7
#define PIN_STEPPER_MUTE_STEP_A 2
#define PIN_STEPPER_MUTE_DIRECTION_A 8
#define PIN_STEPPER_MUTE_STEP_B 3
#define PIN_STEPPER_MUTE_DIRECTION_B 11
#define PIN_STEPPER_PICK_STEP_A 4
#define PIN_STEPPER_PICK_DIRECTION_A 12
#define PIN_STEPPER_PICK_STEP_B 5
#define PIN_STEPPER_PICK_DIRECTION_B 23

Chime chimeA = {Chime(CHIME_A_ID, notefreq2,
                      PIN_STEPPER_TUNE_STEP_A, PIN_STEPPER_TUNE_DIRECTION_A,
                      PIN_STEPPER_PICK_STEP_A, PIN_STEPPER_PICK_DIRECTION_A,
                      PIN_STEPPER_MUTE_STEP_A, PIN_STEPPER_MUTE_DIRECTION_A)};

Chime chimeB = {Chime(CHIME_B_ID, notefreq1,
                      PIN_STEPPER_TUNE_STEP_B, PIN_STEPPER_TUNE_DIRECTION_B,
                      PIN_STEPPER_PICK_STEP_B, PIN_STEPPER_PICK_DIRECTION_B,
                      PIN_STEPPER_MUTE_STEP_B, PIN_STEPPER_MUTE_DIRECTION_B)};

//float detectedFrequencies[numChimes];
volatile float frequencyFromSerial[2];

// Serial variables.
String uartData = "";
const char delimiter = ':';

enum class Commands
{
    Restring,
    Tune
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

////////////////////////////////////////////////////////////////////////

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

////////////////////////////////////////////////////////////////////////

void ProcessUart()
{
    while (Serial2.available() > 0)
    {
        char readChar = Serial2.read();
        uartData += (char)readChar;

        if (readChar == '\n')
        {
            int commandInt = getValue(uartData, delimiter, 0).toInt();

            if (commandInt == int(Commands::Tune))
            {
            }
            else if (commandInt == int(Commands::Restring))
            {
                int chimeId = getValue(uartData, delimiter, 1).toInt();
                int direction = getValue(uartData, delimiter, 2).toInt();

                Serial.printf("Command received: Calibrate | Chime: %u, Direction: %u\n", chimeId, direction);

                if (chimeId == 1 || chimeId == 3)
                {
                    chimeA.ReString(direction);
                }
                else if (chimeId == 2 || chimeId == 4)
                {
                    chimeB.ReString(direction);
                }
            }

            uartData = "";
        }

        // Prevent runaway memory allocation.
        if (uartData.length() > 100)
        {
            uartData = "";
        }
    }
}

void TimerCallback()
{
    chimeA.Tick();
    chimeB.Tick();
}

////////////////////////////////////////////////////////////////////////
void setup()
{
    // Give platformio time to switch back to the terminal.
    delay(1000);

    // Serial connection to terminal for debugging.
    Serial.begin(115200);
    Serial.println("Chime controller startup.");

    // Serial connection to main controller.
    Serial2.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);

    AudioMemory(120);

    t1.beginPeriodic(TimerCallback, 250); // microseconds.

    //////////////////////////
    /*
	
	OLD TUNE NOTE HELPER TEST
	
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

    ProcessUart();

    float targetFrequency;
    static unsigned long start = millis();
    static bool toggle = false;
    if (millis() - start > 2000)
    {
        start = millis();
        toggle = !toggle;

        targetFrequency = toggle ? 146.8 : 196.0;
        chimeB.SetTargetFrequency(targetFrequency);
    }
}