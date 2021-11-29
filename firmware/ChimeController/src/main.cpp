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
Timer tickTimer; // generate a timer from the pool (Pool: 2xGPT, 16xTMR(QUAD), 20xTCK)

AudioInputAnalogStereo adcs1;
AudioAnalyzeNoteFrequency notefreq2;
AudioAnalyzeNoteFrequency notefreq1;
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);

/*
    DEVELOPMENT NOTES
	defined AUDIO_GUITARTUNER_BLOCKS value of 24 changed to 6 for
    (C:\Users\DrZoidburg\.platformio\packages\framework-arduinoteensy\libraries\Audio)
	faster analysis at the sacrafice of less lower frequency detection
	which is not required for this application.	
	3 blocks : 233hz : 9ms per detection
	4 blocks : 174hz : 12ms per detection
	5 blocks : 139hz : 15ms per detection
    A weak magnet for the coil is has a low signal to noise ratio.
	A stronger magnet provides a high signal to noise ration.
	An over strong magnet dampens the string vibrations reducing the length of the signal.
    Tuning:
    Two types of tuning tested: free tuning, direct steps tuning
        Direct step tuning (using a look up table for steps between notes) was tested to allow for maximuim
        stepper speed due to free tuning tending to deaccelerate early. Testing revealed the steps between
        notes varied between 20% to 50% between each test run (testing the amount of steps between frequencies within tolerance).
        The variation was further confirmed when attemping to tune using the steps to note lookup table. Some occasions, 
        while tuning, only a few steps were needed for final tune correction while on most occasions the steps required
        where relatively many.

    RPM Calcs for system understanding.

    // RPM from steps/sec (accelStepper's speed units).
    //For tick interupt timing optimization.
    (n steps/ms * 1000) / (200 steps/rev * 0.5 halfsteps) * 60 = RPM

    // At 1000us between steps
    (1 * 1000) / (400) * 60 = 150 RPM
    
    // At 250us between steps
    (4 * 1000) / (400) * 60 = 600 RPM

    // 1000 steps / sec
    (5000 steps/sec) / (400) * (60) = 150 RPM

    // 4000 steps / sec
    (4000 steps/sec) / (400) * (60) = 600 RPM

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
#define PIN_STEPPER_MUTE_STEP_A 1
#define PIN_STEPPER_MUTE_DIRECTION_A 7
#define PIN_STEPPER_PICK_STEP_A 2
#define PIN_STEPPER_PICK_DIRECTION_A 8

#define PIN_STEPPER_TUNE_STEP_B 3
#define PIN_STEPPER_TUNE_DIRECTION_B 11
#define PIN_STEPPER_MUTE_STEP_B 4
#define PIN_STEPPER_MUTE_DIRECTION_B 12
#define PIN_STEPPER_PICK_STEP_B 5
#define PIN_STEPPER_PICK_DIRECTION_B 23

#define PIN_ENABLE_STEPPER_DRIVERS 22

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
const char delimiter = ':';

enum class Commands
{
    RestringTighten,
    RestringLoosen,
    SetTargetNote,
    PretuneNote,
    Mute,
    Pick
};

////////////////////////////////////////////////////////////////////////

void FlashOnboardLED()
{
    static unsigned long start = millis();
    if (millis() - start > 250)
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

void ProcessCommand(String command)
{
    int commandInt = getValue(command, delimiter, 0).toInt();
    int chimeId = getValue(command, delimiter, 1).toInt();

    Chime *chime = nullptr;
    if (chimeA.GetChimeId() == chimeId)
        chime = &chimeA;
    else if (chimeB.GetChimeId() == chimeId)
        chime = &chimeB;

    if (chime == nullptr)
    {
        return;
    }

    if (commandInt == int(Commands::RestringTighten))
    {
        Serial.printf("[%u] Command: RestringTighten.\n", chimeId);
        chime->RestringTighten();
    }
    else if (commandInt == int(Commands::RestringLoosen))
    {
        Serial.printf("[%u] Command: RestringLoosen.\n", chimeId);
        chime->RestringLoosen();
    }
    else if (commandInt == int(Commands::SetTargetNote))
    {
        int noteId = getValue(command, delimiter, 2).toInt();
        int vibrato = getValue(command, delimiter, 3).toInt();
        Serial.printf("[%u] Command: SetTargetNote, Note ID: %u, Vibrato: %s.\n", chimeId, noteId, vibrato ? "True" : "False");

        if (chime->IsNoteWithinChimesRange(noteId))
        {
            chime->SetTargetNote(noteId);
            chime->SetVibrato(vibrato);
        }
        else
        {
            Serial.printf("Error: target note with ID %u is outside chime's range.", noteId);
        }
    }
    else if (commandInt == int(Commands::PretuneNote))
    {
        int noteId = getValue(command, delimiter, 2).toInt();
        Serial.printf("[%u] Command: PretuneNote, Note ID: %u.\n", chimeId, noteId);

        if (chime->IsNoteWithinChimesRange(noteId))
        {
            chime->PretuneNote(noteId);
        }
        else
        {
            Serial.printf("Error: pretune target note with ID %u is outside chime's range.", noteId);
        }
    }
    else if (commandInt == int(Commands::Mute))
    {
        Serial.printf("[%u] Command: Mute.\n", chimeId);
        chime->Mute();
    }
    else if (commandInt == int(Commands::Pick))
    {
        Serial.printf("[%u] Command: Pick.\n", chimeId);
        chime->UnMute();
        chime->Pick();
    }
}

void ProcessUart()
{
    static String uartData = "";

    while (Serial2.available() > 0)
    {
        char readChar = Serial2.read();
        uartData += (char)readChar;

        if (readChar == '\n')
        {          
            ProcessCommand(uartData);
            uartData = "";
        }
     
        if (uartData.length() > 64)
        {
            uartData = "";
        }
    }
}

void TickTimerCallback()
{
    FlashOnboardLED();

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
    pinMode(PIN_ENABLE_STEPPER_DRIVERS, OUTPUT);
    digitalWrite(PIN_ENABLE_STEPPER_DRIVERS, LOW);
    
    AudioMemory(120); 
    tickTimer.beginPeriodic(TickTimerCallback, 100); // microseconds.
    chimeA.SetTargetNote(69);
    chimeB.SetTargetNote(69);

    /*
    while (1)
    {
        Serial.println("****** TEST TimeBetweenHighAndLowNotes *******");
        chimeA.TimeBetweenHighAndLowNotes();
    }
    */
    
    /*
    Serial.println("****** TEST CalibrateFrequencyPerStep *******");
    chimeA.CalibrateFrequencyPerStep();
    */
}

void loop()
{   
    ProcessUart(); 
}