
#include "Arduino.h"
#include "Chime.h"

Chime::Chime(uint8_t pinMotorTunePhase, uint8_t pinMotorTuneEnable, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinSolenoidMute)
    : _tuneMotor(DCMotor::DriverMode::IN1_IN2, pinMotorTunePhase, pinMotorTuneEnable),
      _pickMotor(DCMotor::DriverMode::ENABLE_PHASE, pinMotorPickPhase, pinMotorPickEnable),
      _stepper(14, 15)
{

    _pinMotorPickLimit = pinMotorPickLimit;
    pinMode(pinMotorPickLimit, INPUT_PULLUP);

    _pinSolenoidMute = pinSolenoidMute;
    pinMode(pinSolenoidMute, OUTPUT);
}

void Chime::TuneFrequency(float detectedFrequency, float targetFrequency)
{
    static unsigned int detectionCount = 0;
    static unsigned long startTimeNewTarget;
    static unsigned int hitTargetCount = 0;
    static unsigned int targetTimeAcc = 0;
    static bool newTargetHitFlag = false;

    float frequencyTolerance = 1.0;

    // Return if not frequency was detected.
    if (detectedFrequency == 0)
        return;

    float frequencyDelta = targetFrequency - detectedFrequency;
    int runTime;

    detectionCount++;
    // Based on Hz/milliseconds(of motor turning)
    // (which is a function motor RPM/torque). TODO: might need an algorithm approch, y = mx+b
    //float runTimeCoef = 0.9;

    // TBD: use either the directly detected frequency or use a moving average (or other type of average).
    // float comparisonFrequency = detectedFrequency;                       // Target hit: 10 | Elapsed time: 459 | Average time to hit targets: 521
    // float comparisonFrequency = movingAverage.getAvg(); (20 readings)    // Target hit:  8 | Elapsed time: 675 | Average time to hit targets: 657
    // float comparisonFrequency = movingAverage.getAvg(); //(10 readings)  // Target hit: 10 | Elapsed time: 402 | Average time to hit targets: 640
    // float comparisonFrequency = movingAverage.getAvg(); //(5 readings)   // Target hit:  9 | Elapsed time: 604 | Average time to hit targets: 550

    char d[5];

    float runTimeCoef = 0.5;
    // runTimeCoef is the P in PID
    // a value too high causes overshoot
    // too low causes a delay.

    float targetPosition = frequencyDelta * runTimeCoef;

    if (targetPosition > 0 && targetPosition < 1)
    targetPosition = 1;

        if (targetPosition < 0 && targetPosition > -1)
    targetPosition = -1;

    if (targetPosition > 50)
    targetPosition = 50;

     if (targetPosition < -50)
    targetPosition = -50;



    if (detectedFrequency < targetFrequency - frequencyTolerance)
    {
        sprintf(d, "UP");
        _stepper.SetCurrentPosition(0);
        _stepper.SetTargetPosition(int(targetPosition));
    }
    else if (detectedFrequency > targetFrequency + frequencyTolerance)
    {
        sprintf(d, "DOWN");
        _stepper.SetCurrentPosition(0);
        _stepper.SetTargetPosition(int(targetPosition));
    }
    else
    {
        startTimeNewTarget = millis();
        runTime = 0;
        _stepper.Stop();       
    }

    static unsigned long startTimeBetweenFreqDetections = millis();

    Serial.printf("%4u (%4ums) | Detected: %3.2f | Target: %3.2f | Delta: %3.2f | targetPosition: %3.2f (%i) | %s\n", detectionCount, millis() - startTimeBetweenFreqDetections, detectedFrequency, targetFrequency, frequencyDelta, targetPosition, int(targetPosition), d);

    startTimeBetweenFreqDetections = millis();
}

void Chime::Pick()
{
    _pickMotor.SetMotorRunTime(DCMotor::Direction::CW, 500); // TODO: timing analysis for specific motor.
    _startPick = millis();
}

void Chime::PickTick()
{
    // Give pick motor time to clear limit switch;
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
    digitalWrite(_pinSolenoidMute, HIGH);
    _startMute = millis();
}

void Chime::MuteTick()
{
    if (millis() - _startMute > 100)
    {
        digitalWrite(_pinSolenoidMute, LOW);
    }
}

void Chime::Tick()
{
    _stepper.Tick();
    _pickMotor.Tick();

    PickTick();
    MuteTick();

    /*
    static unsigned long start;
    if (millis() - start > 1000)
    {
        start = millis();
        _pickMotor.SetMotorRunTime(DCMotor::Direction::Up, 500);
    }
    */
}