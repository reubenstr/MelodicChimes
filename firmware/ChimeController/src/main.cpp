/****************************************************

Project: 
	Melodic Chimes
	
Description: 
	Auto-tuning string chimes controlled via MIDI file.

Phase:
	Prototype and develement

 Developer Notes:
	#define AUDIO_GUITARTUNER_BLOCKS value of 24 changed to n   
	required faster analysis at the sacrafice of lower frequency detection
	3 blocks : 233hz : 9ms per detection
	4 blocks : 174hz : 12ms per detection
	5 blocks : 139hz : 15ms per detection
    6 blocks : 	
	File/library location: (C:\Users\DrZoidburg\.platformio\packages\framework-arduinoteensy\libraries\Audio\analyze_notefreq.h)
	
*/////////////////////////////////////////////////////

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "Chime.h"           // Local class.
#include "movingAvg.h"       // Local class.
#include "TeensyTimerTool.h" // https://github.com/luni64/TeensyTimerTool
#include "main.h"

using namespace TeensyTimerTool;
Timer tickTimer; // generate a timer from the pool (Pool: 2xGPT, 16xTMR(QUAD), 20xTCK)

AudioInputAnalogStereo adcs1;
AudioAnalyzeNoteFrequency notefreq2;
AudioAnalyzeNoteFrequency notefreq1;
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);

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

bool ProcessCommand(String command)
{
    bool commandValidFlag = true;
    int commandInt = getValue(command, delimiter, 0).toInt();
    int chimeId = getValue(command, delimiter, 1).toInt();

    Chime *chime = nullptr;
    if (chimeA.GetChimeId() == chimeId)
        chime = &chimeA;
    else if (chimeB.GetChimeId() == chimeId)
        chime = &chimeB;
    else return;
   
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
    else if (commandInt == int(Commands::VolumePlus))
    {
        Serial.printf("[%u] Command: VolumePlus.\n", chimeId);
        chime->VolumePlus();
    }
     else if (commandInt == int(Commands::VolumeMinus))
    {
        Serial.printf("[%u] Command: VolumeMinus.\n", chimeId);
        chime->VolumeMinus();
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
    else if (commandInt == int(Commands::Pick))
    {
        Serial.printf("[%u] Command: Pick.\n", chimeId);     
        chime->Pick();
    }
    else
    {
        commandValidFlag = false;
    }

    return commandValidFlag;
}

bool ProcessUart()
{
    bool validCommandProcessedFlag = false;
    static String uartData = "";

    while (Serial2.available() > 0)
    {
        char readChar = Serial2.read();
        uartData += (char)readChar;

        if (readChar == '\n')
        {          
            validCommandProcessedFlag = ProcessCommand(uartData);
            uartData = "";
        }
     
        if (uartData.length() > 64)
        {
            uartData = "";
        }
    }

    return validCommandProcessedFlag;
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
    
    // TEMP
    chimeA.SetTargetNote(69);
    chimeB.SetTargetNote(56);

    chimeA.SetMaxVolume();
    chimeB.SetMaxVolume();

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
