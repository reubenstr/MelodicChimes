
#include "Arduino.h"
#include "Chime.h"

Chime::Chime(uint8_t pinMotorTunePhase, uint8_t pinMotorTuneEnable, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinMotorMutePhase, uint8_t pinMotorMuteEnable)
    : _tuneMotor(pinMotorTunePhase, pinMotorTuneEnable), _pickMotor(pinMotorPickPhase, pinMotorPickEnable), _muteMotor(pinMotorMutePhase, pinMotorMuteEnable)
{
    _pinMotorPickLimit = pinMotorPickLimit;
    pinMode(pinMotorPickLimit, INPUT_PULLUP);
}

void Chime::FrequencyToMotor(float detectedFrequency, float targetFrequency)
{
    static unsigned int detectionCount = 0;
    static unsigned long startTimeNewTarget;
    static unsigned int hitTargetCount = 0;
    static unsigned int targetTimeAcc = 0;
    static bool newTargetHitFlag = false;
    float frequencyTolerance = 1.0;

    if (detectedFrequency == 0)
        return;

    float frequencyDelta = targetFrequency - detectedFrequency;
    int runTime;

    detectionCount++;
    // Based on Hz/milliseconds(of motor turning)
    // (which is a function motor RPM/torque). TODO: needs an algorithm approch, y = mx+b
    float runTimeCoef = 6;

    // TBD: use either the directly detected frequency or use a moving average (or other type of average).
    // float comparisonFrequency = detectedFrequency;                       // Target hit: 10 | Elapsed time: 459 | Average time to hit targets: 521
    // float comparisonFrequency = movingAverage.getAvg(); (20 readings)    // Target hit:  8 | Elapsed time: 675 | Average time to hit targets: 657
    // float comparisonFrequency = movingAverage.getAvg(); //(10 readings)  // Target hit: 10 | Elapsed time: 402 | Average time to hit targets: 640
    // float comparisonFrequency = movingAverage.getAvg(); //(5 readings)   // Target hit:  9 | Elapsed time: 604 | Average time to hit targets: 550

    runTime = abs(frequencyDelta * runTimeCoef);
    if (runTime < 20)
        runTime = 20;

    if (detectedFrequency < targetFrequency - frequencyTolerance)
    {
        _tuneMotor.SetMotorRunTime(DCMotor::Direction::Up, runTime);
    }
    else if (detectedFrequency > targetFrequency + frequencyTolerance)
    {
        _tuneMotor.SetMotorRunTime(DCMotor::Direction::Down, runTime);
    }
    else
    {
        startTimeNewTarget = millis();
        runTime = 0;
        _tuneMotor.MotorStop();
    }

    // Serial.printf("%4u | Detected: %3.2f | Target: %3.2f | Delta: %3.2f | Run Time: %u\n", detectionCount, detectedFrequency, targetFrequency, frequencyDelta, runTime);
}

void Chime::Pick()
{
    _pickMotor.SetMotorRunTime(DCMotor::Direction::Up, 500); // TODO: timing analysis for specific motor.
    _startPick = millis();
}

void Chime::PickTick()
{
    if (millis() - _startPick > 100)
    {
        if (digitalRead(_pinMotorPickLimit) == _motorPickIndexActivated)
        {
            _pickMotor.MotorStop();
        }
    }
}

void Chime::Mute()
{
    _muteMotor.SetMotorRunTime(DCMotor::Direction::Up, 300); // TODO: timing analysis for specific motor.
    _muteFlag = true;
}

void Chime::Tick()
{

    _tuneMotor.Tick();
    _pickMotor.Tick();
    _muteMotor.Tick();

    PickTick();

    if (_muteFlag && !_muteMotor.IsRunning())
    {
        _muteFlag = false;
        _muteMotor.SetMotorRunTime(DCMotor::Direction::Down, 300); // TODO: timing analysis for specific motor.
    }

    /*
    static unsigned long start;
    if (millis() - start > 1000)
    {
        start = millis();
        _pickMotor.SetMotorRunTime(DCMotor::Direction::Up, 500);
    }
    */
}