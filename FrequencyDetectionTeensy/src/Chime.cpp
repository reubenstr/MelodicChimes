
#include "Arduino.h"
#include "Chime.h"

Chime::Chime(uint8_t pinPhase, uint8_t pinEnable) : _dcMotor(pinPhase, pinEnable)
{
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

    static float oldTarget;
    if (oldTarget != targetFrequency)
    {
        oldTarget = targetFrequency;
        startTimeNewTarget = millis();
        newTargetHitFlag = false;
    }

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

    if (detectedFrequency < targetFrequency - frequencyTolerance)
    {
        runTime = abs(frequencyDelta * runTimeCoef);
        _dcMotor.SetMotorRunTime(DCMotor::Direction::Up, runTime);
    }
    else if (detectedFrequency > targetFrequency + frequencyTolerance)
    {
        runTime = abs(frequencyDelta * runTimeCoef);
        _dcMotor.SetMotorRunTime(DCMotor::Direction::Down, runTime);
    }
    else
    {
        // Target hit.
        if (newTargetHitFlag == false)
        {
            hitTargetCount++;
            targetTimeAcc += millis() - startTimeNewTarget;
            Serial.printf("Target hit: %u | Elapsed time: %u | Average time to hit targets: %u\n", hitTargetCount, millis() - startTimeNewTarget, targetTimeAcc / hitTargetCount);
        }

        newTargetHitFlag = true;
        startTimeNewTarget = millis();

        runTime = 0;
        _dcMotor.MotorStop();
    }

    Serial.printf("%4u | Detected: %3.2f | Target: %3.2f | Delta: %3.2f | Run Time: %u\n", detectionCount, detectedFrequency, targetFrequency, frequencyDelta, runTime);
}

void Chime::Tick()
{

    _dcMotor.Tick();
}