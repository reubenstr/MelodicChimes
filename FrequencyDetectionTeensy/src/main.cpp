// Melodic Chimes
//
// Auto-tuning string chimes control via MIDI
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

#define SERIAL 0

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
//int delays[numNotes] = {750, 750, 750, 750, 750, 750, 750, 750, 750};
int delays[numNotes] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};

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

#define PIN_SERVO_3 3

#define STEP A0
#define DIR A1

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

movingAvg movingAverage(10);

enum PickSide
{
    Left,
    Right
} pickSide;

//////////////////////////////
// Motor
// Direction of the motor refers to the pitch
// increasing or decreasing.
enum Direction
{
    Up,
    Down
};

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

/**/
float GetFrequency()
{
    static float min = 2048;
    static float max = 0;
    static int count = 0;
    static int timeElapsed1 = millis();

    if (notefreq1.available())
    {
        float note = notefreq1.read();
        float prob = notefreq1.probability();

        //if (prob >= 0.99)
        if (prob > 0.995)
        {
            count++;

            if (note > max)
                max = note;

            if (note < min)
                min = note;

            if (SERIAL)
                Serial.printf("(%ums) %3.2f\n", millis() - timeElapsed1, note, prob);
            // Serial.printf("(%U) %3.2f | Prob: %.2f\n", millis() - timeElapsed1, note, prob);
            // Serial.printf("Min %3.2f | Max: %3.2f | Delta: %3.2f | Count %u\n", min, max, max - min, count);
            // Serial.printf("A. Mem. Max: %u\n", AudioMemoryUsageMax());

            timeElapsed1 = millis();

            return note;
        }

        timeElapsed1 = millis();
    }

    return 0;
}

/**/
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

void Halt()
{
    Serial.println("Halted!");
    while (1)
    {
    }
}

////////////////////////////////////////////////////////////////////////
// DC-Motor code:
void SetMotorDirection(Direction direction)
{
    if (direction == Direction::Up)
        digitalWrite(PIN_MOTOR_PHASE, true);
    if (direction == Direction::Down)
        digitalWrite(PIN_MOTOR_PHASE, false);
}

bool GetMotorDirection()
{
    return digitalRead(PIN_MOTOR_PHASE);
}

void EnableMotor(bool enable)
{
    digitalWrite(PIN_MOTOR_ENABLE, enable);
}

bool MotorState()
{
    return digitalRead(PIN_MOTOR_ENABLE);
}

void SetMotorRunTime(Direction direction, unsigned int rt)
{
    runTime = rt;
    startMillis = millis();
    SetMotorDirection(direction);
    EnableMotor(true);
}

void MotorRun()
{
    if (millis() - startMillis > runTime)
    {
        EnableMotor(false);
    }
}

void MotorStop()
{
    EnableMotor(true);
}

////////////////////////////////////////////////////////////////////////

void setup()
{
    delay(1000); // Give platformio time to switch back to the terminal.

    Serial.begin(115200);
    Serial.println("Teensy starting up.");

    pinMode(LED_BUILTIN, OUTPUT);

    // pinMode(STEP, OUTPUT);
    // pinMode(DIR, OUTPUT);

    digitalWrite(PIN_MOTOR_PHASE, LOW);
    digitalWrite(PIN_MOTOR_ENABLE, LOW);

    pinMode(PIN_MOTOR_PHASE, OUTPUT);
    pinMode(PIN_MOTOR_ENABLE, OUTPUT);

    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_LED_3, OUTPUT);
    pinMode(PIN_LED_4, OUTPUT);

    AudioMemory(120);
    notefreq1.begin(.15);
    // notefreq2.begin(.25);

    servo3.attach(PIN_SERVO_3, 1000, 2000);

    //movingAverage.begin();

    //Pick(true);
    //delay(3000);
}

void loop()
{

/*
    static float targetFrequency = 329.63;
    static float currentFrequency = 0;
    static int attemptCount = 0;
    static bool activeFrequencyFlag = false;
    static unsigned int noteCount;
    static float errorAcc = 0;
    static int accCount = 0;
    static float errorMax = 0;
    static float errorMin = 5000.0;

    static float freqAcc = 0;
    static float freqMax = 0;
    static float freqMin = 5000.0;
    */

    //////////////////////////////////////////////////////////////////////////////////////////
    // NOTES AREA
    //////////////////////////////////////////////////////////////////////////////////////////
    /* 
    Turn tests: start, turn, test, turn, end
    400.03 - full - 246.60 - full - 399.67
    427.02 - full - 272.89 - full - 429.36

    Estimated Hz per rotation: 153.78
                    per degre: 0.4271

    0.12815 hz / millisecond


    TEST MOTOR:
    6v, 50RPM (no load), 24oz/in (crude test), 1200ms per revolution. 
    Pololu's N20 6v (low power) 54RPM has a oz/in of 24 which is on par with out test motor.

    The current torque is capable of turning the peg (of our current test string (high E)).
    ---
    POSSIBLE MOTOR:
    Pololu: N20, 6v, 220RPM, 28oz/in |  273ms per revolution
    Pololu: N20, 6v, 310RPM, 24oz/in |  194ms per revolution

    */
    //////////////////////////////////////////////////////////////////////////////////////////
    // TEMP TEST AREA
    //////////////////////////////////////////////////////////////////////////////////////////

    /*
    GetFrequency();
  
    EnableMotor(true);
    SetMotorDirection(Up);
       */

    //////////////////////////////////////////////////////////////////////////////////////////
    // DC-Motor TEST AREA
    //////////////////////////////////////////////////////////////////////////////////////////

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
    ////

    /*  
    // Attempt a melody.
    static unsigned long startTarget = millis();
    static int noteSelect = 0;
    static unsigned int delay = 0;

  

    static State state = sPick;

    static bool preTuneCompletedFlag;

    if (noteSelect < numNotes)
    {

        if (state == State::sPick)
        {
            if (millis() - startTarget >= delay)
            {
                startTarget = millis();

                Pick();
                delay = delays[noteSelect] / 2;
                targetFrequency = notes[noteSelect];
                Serial.printf("Target frequency: %3.2f | Note Index: %u\n", targetFrequency, noteSelect);
                noteSelect++;
                state = State::sMute;
            }
        }
        else if (state == State::sMute)
        {
            if (millis() - startTarget >= delay)
            {
                startTarget = millis();
                Mute();
                preTuneCompletedFlag = false;
                state = State::sPretune;
            }
        }
        else if (state == State::sPretune)
        {

            static unsigned long pretuneMillis = 0;
            static unsigned int preTuneTime = 0;

            if (millis() - startTarget >= delay)
            {
                startTarget = millis();
                state = State::sPick;
            }

            if (preTuneCompletedFlag == false)
            {
                preTuneCompletedFlag = true;
                pretuneMillis = millis();
                float deltaFreq = notes[noteSelect] - notes[noteSelect - 1];
                preTuneTime = abs(deltaFreq * 10);

                if (deltaFreq > 0)
                    SetMotorDirection(Direction::Up);
                if (deltaFreq <= 0)
                    SetMotorDirection(Direction::Down);

                Serial.printf("\t\t\tFreq Delta: %3.2f | Delay (ms): %u\n", deltaFreq, preTuneTime);
                char motState[12];
                sprintf(motState, "%s", MotorState() ? "On" : "Off");
                char dirStr[12];
                sprintf(dirStr, "%s", MotorState() == 0 ? "" : GetMotorDirection() ? "(Up)" : "(Down)");
                Serial.printf("\t\t\tNext Target %3.2f | Current: %3.2f | Motor state %s %s \n", notes[noteSelect], notes[noteSelect - 1], motState, dirStr);
        
                EnableMotor(true);
            }

            if (millis() - pretuneMillis > preTuneTime)
            {
                EnableMotor(false);
            }
        }
    }
    */

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////

    // NEW AREA, NEW MOTOR CODE

    /*
    static int noteSelect = 0;
    static unsigned long noteStartMillis;

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

    MotorRun();

    static float targetFrequency = 329.63;
    float frequencyTolerance = 1.0;
    float detectedFrequency = GetFrequency();

    if (detectedFrequency > 0)
    {
        float frequencyDelta = targetFrequency - detectedFrequency;
        int runTime;
        int runTimeCoef = 5; // Based on Hz/milliseconds (a function motor RPM/torque). TODO: needs an algorithm approch, y = mx+b

        if (detectedFrequency < targetFrequency - frequencyTolerance)
        {
            runTime = abs(frequencyDelta * runTimeCoef);
            SetMotorRunTime(Direction::Up, runTime);
        }
        else if (detectedFrequency > targetFrequency + frequencyTolerance)
        {
            runTime = abs(frequencyDelta * runTimeCoef);
            SetMotorRunTime(Direction::Down, runTime);
        }
        else
        {
            runTime = 0;
            MotorStop();
        }

        Serial.printf("Detected: %3.2f | Target: %3.2f | Delta: %3.2f | Run Time: %u\n", detectedFrequency, targetFrequency, frequencyDelta, runTime);
    }

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////

    /*

    static unsigned int noFreqCount = 0;
    static unsigned long start = millis();

    unsigned int delayBetweenRetrievingNotes = 1;
    unsigned int noFreqDetectionTimeout_NoteCycles = 20; //

    // Delay clearing pickFlag for x milliseconds.
    static unsigned long pickMillis = millis();
    if (pickedFlag)
    {
        if (millis() - pickMillis > 50)
        {
            pickedFlag = false;
        }
    }
    else
    {
        pickMillis = millis();
    }

    // After a pick wait X milliseconds to allow servo PWM to kick in
    // as well as allow the string to settle for a more accurate frequency detection.
    // frequency tends to spike after a harse pick and our picking system harshly picks.
    if (millis() - start >= delayBetweenRetrievingNotes && pickedFlag == false)
    {
        start = millis();

        float returnFrequency = GetFrequency();
        if (returnFrequency > 0)
        {
            noteCount++;
            noFreqCount = 0;
            currentFrequency = returnFrequency;
            activeFrequencyFlag = true;
        }
        else // No frequency detected
        {
            noFreqCount++;
            if (noFreqCount >= noFreqDetectionTimeout_NoteCycles)
            {
                if (activeFrequencyFlag)
                {
                    if (SERIAL)
                        Serial.println("activeFrequencyFlag = false");
                    if (SERIAL)
                        Serial.printf("\t\tAverage Error: %3.2f | Min: %3.2f | Max: %3.2f | Error Spread: %3.2f | Num. Samples: %u\n", errorAcc / accCount, errorMin, errorMax, errorMax - errorMin, accCount);
                    if (SERIAL)
                        Serial.printf("\t\tAverage Freq.: %3.2f | Min: %3.2f | Max: %3.2f | Freq. Spread: %3.2f | Num. Samples: %u\n", freqAcc / accCount, freqMax, freqMin, freqMax - freqMin, accCount);
                    if (SERIAL)
                        Serial.printf("\t\tMovAvg. Freq.: %3.2f with %d samples.\n", movingAverage.getAvg(), movingAverage.getCount());
                    errorMin = 5000.0;
                    errorMax = 0.0;
                    errorAcc = 0;
                    accCount = 0;
                    freqAcc = 0;
                    freqMax = 0;
                    freqMin = 5000.5;
                    movingAverage.reset();
                }

                activeFrequencyFlag = false;
            }
        }

        if (activeFrequencyFlag)
        {
            float frequencyTolerance = 1.0;

            movingAverage.reading(currentFrequency);

            if (currentFrequency < targetFrequency - frequencyTolerance)
            {
                EnableMotor(true);
                SetMotorDirection(Direction::Up);
            }
            else if (currentFrequency > targetFrequency + frequencyTolerance)
            {
                EnableMotor(true);
                SetMotorDirection(Direction::Down);
            }
            else
            {
                EnableMotor(false);
            }

            float error = currentFrequency - targetFrequency;

            accCount++;

            freqAcc += currentFrequency;
            if (currentFrequency > freqMax)
                freqMax = currentFrequency;
            if (currentFrequency < freqMin)
                freqMin = currentFrequency;

            errorAcc += error;
            if (error > errorMax)
                errorMax = error;
            if (error < errorMin)
                errorMin = error;

            char motState[12];
            sprintf(motState, "%s", MotorState() ? "On" : "Off");
            char dirStr[12];
            sprintf(dirStr, "%s", MotorState() == 0 ? "" : GetMotorDirection() ? "(Up)" : "(Down)");
            // Serial.printf("\t%5u: Target %3.2f | Current: %3.2f | Error: %3.2f | Motor state %s %s \n", noteCount, targetFrequency, currentFrequency, error, motState, dirStr);
        }
        else
        {

            EnableMotor(false);
        }
    }
    */

    FlashOnboardLED();
}