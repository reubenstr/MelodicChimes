
#include "Arduino.h"
#include "Chime.h"

Chime::Chime(uint8_t pinStepperTuneStep, uint8_t pinStepperTuneDirection,
             uint8_t pinStepperPickStep, uint8_t pinStepperPickDirection,
             uint8_t pinStepperMuteStep, uint8_t pinStepperMuteDirection)
    : _tuneStepper(AccelStepper::DRIVER, pinStepperTuneStep, pinStepperTuneDirection),
      _pickStepper(AccelStepper::DRIVER, pinStepperPickStep, pinStepperPickDirection),
      _muteStepper(AccelStepper::DRIVER, pinStepperMuteStep, pinStepperMuteDirection)
{
    SetStepperParameters();
}

void Chime::SetStepperParameters()
{
    _tuneStepper.setMaxSpeed(50000);
    _tuneStepper.setAcceleration(2500);
    _tuneStepper.setMinPulseWidth(3);

    _pickStepper.setMaxSpeed(20000);
    _pickStepper.setAcceleration(20000);
    _pickStepper.setMinPulseWidth(3);

    _muteStepper.setMaxSpeed(20000);
    _muteStepper.setAcceleration(20000);
    _muteStepper.setMinPulseWidth(3);
}

// PID IMPLEMENTATION
/*
void Chime::TuneFrequency(float detectedFrequency, float targetFrequency)
{
    static unsigned int detectionCount = 0;
    static unsigned long startTimeNewTarget;
    char d[5];
    float frequencyTolerance = 1.0;

    // Return if not frequency was detected.
    if (detectedFrequency == 0)
        return;

    detectionCount++;

    float frequencyDelta = targetFrequency - detectedFrequency;

    static float integral = 0;
    float error, kp, ki;
    float targetPosition;
    kp = 1;
    ki = .00;

    error = frequencyDelta;
    integral = integral + error;
    targetPosition = (kp * error) + (ki * integral);

    int minPos = 1;
    int maxPos = 20;

    if (targetPosition > 0 && targetPosition < minPos)
        targetPosition = minPos;
    if (targetPosition < 0 && targetPosition > -minPos)
        targetPosition = -minPos;

    if (targetPosition > maxPos)
        targetPosition = maxPos;
    if (targetPosition < -maxPos)
        targetPosition = -maxPos;

    if (detectedFrequency < targetFrequency - frequencyTolerance)
    {
        sprintf(d, "  UP");
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + int(targetPosition));
        //_tuneMotor.SetPwmSpeed(abs(int(targetPosition)) * 5);
        //_tuneMotor.SetMotorRunTime(DCMotor::Direction::CW, abs(int(targetPosition)));
    }
    else if (detectedFrequency > targetFrequency + frequencyTolerance)
    {
        sprintf(d, "DOWN");
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + int(targetPosition));
        //_tuneMotor.SetPwmSpeed(abs(int(targetPosition)) * 5);
        //_tuneMotor.SetMotorRunTime(DCMotor::Direction::CCW, abs(int(targetPosition)));
    }
    else
    {
        startTimeNewTarget = millis();
        targetPosition = 0;
        //_tuneStepper.stop();
        _tuneMotor.MotorStop();

        sprintf(d, "STOP");

        integral = 0;
    }

    static unsigned long startTimeBetweenFreqDetections = millis();

    if (millis() - startTimeBetweenFreqDetections > 100)
        detectionCount = 0;

    Serial.printf("%4u (%4ums) | Detected: %3.2f | Target: %3.2f | Delta: % 7.2f | targetPosition: % 7.2f (%3i) | %s | Step Speed % 5.0f\n", detectionCount, millis() - startTimeBetweenFreqDetections, detectedFrequency, targetFrequency, frequencyDelta, targetPosition, int(targetPosition), d, _tuneStepper.speed());

    startTimeBetweenFreqDetections = millis();
}
*/

// STEPPER IMPLEMENTION TEST
void Chime::TuneFrequency(float detectedFrequency, float targetFrequency)
{
    static unsigned int detectionCount = 0;
    static unsigned long startTimeNewTarget;

    float frequencyTolerance = 1.0;

    // Return if not frequency was detected.
    if (detectedFrequency == 0)
        return;

    float frequencyDelta = targetFrequency - detectedFrequency;
    int runTime;
    char d[5];

    float runTimeCoef = 0.5;
    float posExponent = 1.25;
    float targetPosition = runTimeCoef * powf(fabs(frequencyDelta), posExponent);
    int minPos = 1;
    int maxPos = 200;

    detectionCount++;

    if (targetPosition > 0 && targetPosition < minPos)
        targetPosition = minPos;
    if (targetPosition < 0 && targetPosition > -minPos)
        targetPosition = -minPos;

    if (targetPosition > maxPos)
        targetPosition = maxPos;
    if (targetPosition < -maxPos)
        targetPosition = -maxPos;

    if (detectedFrequency < targetFrequency - frequencyTolerance)
    {
        sprintf(d, "  UP");
        //_tuneStepper.setCurrentPosition(0);
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + int(targetPosition));
    }
    else if (detectedFrequency > targetFrequency + frequencyTolerance)
    {
        sprintf(d, "DOWN");

        //_tuneStepper.setCurrentPosition(0);
        _tuneStepper.moveTo(_tuneStepper.currentPosition() - int(targetPosition));
    }
    else
    {
        startTimeNewTarget = millis();
        runTime = 0;
        targetPosition = 0;
        //_tuneStepper.stop();

        sprintf(d, "STOP");
    }

    static unsigned long startTimeBetweenFreqDetections = millis();
    if (millis() - startTimeBetweenFreqDetections > 100)
        detectionCount = 0;

    Serial.printf("%4u (%4ums) | Detected: %3.2f | Target: %3.2f | Delta: % 7.2f | targetPosition: % 7.2f (%3i) | %s | Step Speed % 5.0f\n",
                  detectionCount, millis() - startTimeBetweenFreqDetections, detectedFrequency, targetFrequency, frequencyDelta, targetPosition, int(targetPosition), d, _tuneStepper.speed());

    startTimeBetweenFreqDetections = millis();
}

// Pick the string if not currently in a picking motion.
void Chime::Pick()
{    
    if (!_pickStepper.isRunning())
    {
        _pickStepper.setCurrentPosition(0);
        _pickStepper.moveTo(_stepsPerPick);
    }
}

void Chime::PickTick()
{
    // Give pick motor time to clear limit switch;
    if (millis() - _startPick > 150)
    {
        if (digitalRead(_pinMotorPickLimit) == _motorPickIndexActivated)
        {
            _pickStepper.stop();
        }
    }
}

void Chime::Mute()
{
    /*
    _muteStepper.SetCurrentPosition(0);
    _muteStepper.SetTargetPosition(400);

    _muteReturnToOpenFlag = true;

    _startMute = millis();
    */
}

void Chime::MuteTick()
{
    /*
    _muteStepper.Tick();

    if (_muteReturnToOpenFlag && _muteStepper.IsAtPosition())
    {
        _muteReturnToOpenFlag = false;
        _muteStepper.SetCurrentPosition(0);
        _muteStepper.SetTargetPosition(-400);
    }
    */
}

void Chime::CalibratePick(bool freqencyDetectedFlag)
{

    if (calibrateDoOnceFlag)
    {
        _pickStepper.setMaxSpeed(125);
        _pickStepper.setAcceleration(1000);
        _pickStepper.setCurrentPosition(0);
        _pickStepper.moveTo(300);
        calibrateDoOnceFlag = false;
    }

    if (freqencyDetectedFlag)
    {
        _pickStepper.stop();
        // _pickStepper.setCurrentPosition(0);
        _pickStepper.moveTo(_pickStepper.currentPosition() + _stepsPerPick / 2);
        _pickStepper.runToPosition();
        calibrateDoOnceFlag = true;
        SetStepperParameters();
    }
    else
    {
        _pickStepper.run();
    }
}

void Chime::Tick()
{

    _tuneStepper.run();
    _pickStepper.run();
    _muteStepper.run();

    PickTick();
    MuteTick();
}
