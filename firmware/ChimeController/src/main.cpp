/****************************************************
Project: 
	Melodic Chimes
	02-2020 - 01-2021
	Reuben Strangelove
	
Description: 
	Auto-tuning string chimes controlled via MIDI file.
	
Phase:
	Final development!
	
 Developer Notes: 
	#define AUDIO_GUITARTUNER_BLOCKS value of 24 changed to n   
	required for faster analysis at the sacrifice of reduced lower frequency detection
	blocks : lowest detection frequency : time between detections
	3 : 233hz : 9ms
	4 : 174hz : 12ms
	5 : 139hz : 15ms
    6 :       : 18ms
	File/library location: (C:\Users\DrZoidburg\.platformio\packages\framework-arduinoteensy\libraries\Audio\analyze_notefreq.h)
	
*/
////////////////////////////////////////////////////

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Chime.h>           // Local class.
#include <movingAvg.h>       // Local class.
#include <TeensyTimerTool.h> // https://github.com/luni64/TeensyTimerTool
#include <main.h>

// Select the controller's chimes:
#define CHIME_0_AND_1
//#define CHIME_2

using namespace TeensyTimerTool;
Timer tickTimer; // generate a timer from the pool (Pool: 2xGPT, 16xTMR(QUAD), 20xTCK)

AudioInputAnalogStereo adcs1;
AudioAnalyzeNoteFrequency notefreq2;
AudioAnalyzeNoteFrequency notefreq1;
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);

#define PIN_STEPPER_M1_STEP 0
#define PIN_STEPPER_M1_DIRECTION 6
#define PIN_STEPPER_M2_STEP 1
#define PIN_STEPPER_M2_DIRECTION 7
#define PIN_STEPPER_M3_STEP 2
#define PIN_STEPPER_M3_DIRECTION 8
#define PIN_STEPPER_M4_STEP 3
#define PIN_STEPPER_M4_DIRECTION 11
#define PIN_STEPPER_M5_STEP 4
#define PIN_STEPPER_M5_DIRECTION 12
#define PIN_STEPPER_M6_STEP 5
#define PIN_STEPPER_M6_DIRECTION 23

#define PIN_STEPPER_DRIVERS_ENABLE 22

#if defined CHIME_0_AND_1
Chime chime0 = {Chime(0, notefreq2,
                      PIN_STEPPER_M1_STEP, PIN_STEPPER_M1_DIRECTION,
                      PIN_STEPPER_M2_STEP, PIN_STEPPER_M2_DIRECTION,
                      PIN_STEPPER_M3_STEP, PIN_STEPPER_M3_DIRECTION)};

Chime chime1 = {Chime(1, notefreq1,
                      PIN_STEPPER_M4_STEP, PIN_STEPPER_M4_DIRECTION,
                      PIN_STEPPER_M5_STEP, PIN_STEPPER_M5_DIRECTION,
                      PIN_STEPPER_M6_STEP, PIN_STEPPER_M6_DIRECTION)};
#elif defined CHIME_2
Chime chime2 = {Chime(2, notefreq2,
                      PIN_STEPPER_M1_STEP, PIN_STEPPER_M1_DIRECTION,
                      PIN_STEPPER_M2_STEP, PIN_STEPPER_M2_DIRECTION,
                      PIN_STEPPER_M3_STEP, PIN_STEPPER_M3_DIRECTION)};
#endif

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

void SendCommand(Commands command, int chime)
{
    char buf[16];
    sprintf(buf, "%u:%u\n", int(command), chime);
    Serial.printf("Sending command from chime %u: %s", chime, buf);
    Serial2.print(buf);
}

bool ProcessCommand(String command)
{
    bool commandValidFlag = true;
    int commandInt = getValue(command, delimiter, 0).toInt();
    int chimeId = getValue(command, delimiter, 1).toInt();

    Chime *chime = nullptr;
#if defined CHIME_0_AND_1
    if (chimeId == 0)
        chime = &chime0;
    else if (chimeId == 1)
        chime = &chime1;
    else
        return false;
#elif defined CHIME_2
    if (chimeId == 2)
        chime = &chime2;
    else
        return false;
#endif

    if (commandInt == int(Commands::Enable))
    {
        // No action required, chimes will enable upon command received.
    }
    else if (commandInt == int(Commands::Tighten))
    {
        Serial.printf("[%u] Command: RestringTighten.\n", chimeId);
        chime->RestringTighten();
    }
    else if (commandInt == int(Commands::Loosen))
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
        Serial.printf("[%u] Command: SetTargetNote, Note ID: %u, Vibrato: %s\n", chimeId, noteId, vibrato ? "True" : "False");

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
    // FlashOnboardLED();

#if defined CHIME_0_AND_1
    chime0.Tick();
    chime1.Tick();
#elif defined CHIME_2
    chime2.Tick();
#endif
}

void SendStepperStatus(bool status)
{
    Commands command = status ? Commands::StatusEnabled : Commands::StatusDisabled;
#if defined CHIME_0_AND_1
    SendCommand(command, 0);
    SendCommand(command, 1);
#elif defined CHIME_2
    SendCommand(command, 2);
#endif
}

void EnableSteppers(bool enable)
{
    if (enable)
    {
        if (digitalRead(PIN_STEPPER_DRIVERS_ENABLE) == HIGH)
        {
            digitalWrite(PIN_STEPPER_DRIVERS_ENABLE, LOW);
            SendStepperStatus(true);
        }
    }
    else
    {
        if (digitalRead(PIN_STEPPER_DRIVERS_ENABLE) == LOW)
        {
            digitalWrite(PIN_STEPPER_DRIVERS_ENABLE, HIGH);
            SendStepperStatus(false);
        }
    }
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
    pinMode(PIN_STEPPER_DRIVERS_ENABLE, OUTPUT);

    AudioMemory(120);
    tickTimer.beginPeriodic(TickTimerCallback, 100); // microseconds.

// Check if succesful, send error to main controller if unsucessful.
#if defined CHIME_0_AND_1
    chime0.CalibratePick();
    chime1.CalibratePick();
#elif defined CHIME_2
    chime2.CalibratePick();
#endif

    /*
	#if defined CHIME_0_AND_1
	chime0.SetMaxVolume();
    chime1.SetMaxVolume();
	#elif defined CHIME_2
	chime2.SetMaxVolume();    
	#endif
    */

    /*    
    while (1)
    {
        Serial.println("*** TEST TimeBetweenHighAndLowNotes ***");
        chimeA.CaptureTimeFromLowToHighNote();
        delay(5000);
    }
    */

    /*
    while (1)
    {
        Serial.println("*** TEST CalibrateFrequencyPerStep ***");
        chime2.CaptureFrequencyPerStep();
        delay(5000);
    }
    */
}

void loop()
{
    static unsigned long startTimeout = millis();
    if (ProcessUart())
    {
        digitalWrite(LED_BUILTIN, HIGH);
        EnableSteppers(true);
        startTimeout = millis();
    }
    else if (millis() - startTimeout > stepperTimeoutDelayMs)
    {
        startTimeout = millis();
        digitalWrite(LED_BUILTIN, LOW);
        EnableSteppers(false);
    }
}