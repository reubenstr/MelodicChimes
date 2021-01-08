
#include "Arduino.h"
#include "Chime.h"

Chime::Chime(uint8_t pinTuneStepperStep, uint8_t pinTuneStepperDirection, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinSolenoidMute)
    //: _tuneStepper(pinTuneStepperStep, pinTuneStepperDirection),
    : _muteStepper(AccelStepper::DRIVER,PIN_STEPPER_MUTE_STEP, PIN_STEPPER_MUTE_DIRECTION),
      _tuneStepper(AccelStepper::DRIVER, PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION),
      _pickStepper(AccelStepper::DRIVER, PIN_STEPPER_PICK_STEP, PIN_STEPPER_PICK_DIRECTION)
{
    //_pinMotorPickPhase = pinMotorPickPhase;
    // _pinMotorPickEnable = pinMotorPickEnable;
    //_pinMotorPickLimit = pinMotorPickLimit;
    //_pinSolenoidMute = pinSolenoidMute;

    //digitalWrite(_pinMotorPickPhase, LOW);
    //digitalWrite(_pinMotorPickEnable, LOW);
    //digitalWrite(_pinSolenoidMute, LOW);

    //pinMode(_pinMotorPickPhase, OUTPUT);
    //pinMode(_pinMotorPickEnable, OUTPUT);
    //pinMode(_pinMotorPickLimit, INPUT_PULLUP);
    //pinMode(_pinSolenoidMute, OUTPUT);

    //digitalWrite(_pinMotorPickPhase, HIGH);

    // _tuneStepper.SetSpeed(500);
    //_muteStepper.SetSpeed(500);

    _tuneStepper.setMaxSpeed(10000);
    _tuneStepper.setAcceleration(2500);
    _tuneStepper.setMinPulseWidth(3);

    _pickStepper.setMaxSpeed(10000);
    _pickStepper.setAcceleration(5000);
    _pickStepper.setMinPulseWidth(3);

    //_aStepper.setPinsInverted();
}

void Chime::TuneFrequency(float detectedFrequency, float targetFrequency)
{
    static unsigned int detectionCount = 0;
    static unsigned long startTimeNewTarget;
    //static unsigned int hitTargetCount = 0;
    //static unsigned int targetTimeAcc = 0;
    //static bool newTargetHitFlag = false;

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

    float runTimeCoef = 3; //0.5

    // runTimeCoef is the P in PID
    // a value too high causes overshoot
    // too low causes a delay.

    float targetPosition = frequencyDelta * runTimeCoef;

    if (targetPosition > 0 && targetPosition < 1)
        targetPosition = 1;

    if (targetPosition < 0 && targetPosition > -1)
        targetPosition = -1;

    if (targetPosition > 250)
        targetPosition = 250;

    if (targetPosition < -250)
        targetPosition = -250;

    if (detectedFrequency < targetFrequency - frequencyTolerance)
    {
        sprintf(d, "UP");
        //_tuneStepper.SetCurrentPosition(0);
        //_tuneStepper.SetTargetPosition(int(targetPosition));
        _tuneStepper.setCurrentPosition(0);
        _tuneStepper.moveTo(int(targetPosition));
        //_aStepper.moveTo(_aStepper.currentPosition() + int(targetPosition));
    }
    else if (detectedFrequency > targetFrequency + frequencyTolerance)
    {
        sprintf(d, "DOWN");
        //_tuneStepper.SetCurrentPosition(0);
        //_tuneStepper.SetTargetPosition(int(targetPosition));
        _tuneStepper.setCurrentPosition(0);
        _tuneStepper.moveTo(int(targetPosition));
        //_aStepper.moveTo(_aStepper.currentPosition() - int(targetPosition));
    }
    else
    {
        startTimeNewTarget = millis();
        runTime = 0;
        //_tuneStepper.Stop();
        _tuneStepper.stop();

        sprintf(d, "STOP");
    }

    static unsigned long startTimeBetweenFreqDetections = millis();

    Serial.printf("%4u (%4ums) | Detected: %3.2f | Target: %3.2f | Delta: %3.2f | targetPosition: %3.2f (%i) | %s\n", detectionCount, millis() - startTimeBetweenFreqDetections, detectedFrequency, targetFrequency, frequencyDelta, targetPosition, int(targetPosition), d);

    startTimeBetweenFreqDetections = millis();
}

void Chime::Pick()
{
    _pickStepper.setCurrentPosition(0);
    _pickStepper.moveTo(133);
    /*
    //digitalWrite(_pinMotorPickEnable, HIGH);
    _startPick = millis();
    */
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

/*
// DC MOTOR MUTE
void Chime::Mute()
{

    digitalWrite(12, HIGH);
    digitalWrite(11, HIGH);
    _startMute = millis();
}

void Chime::MuteTick()
{

    if (millis() - _startMute > 75)
    {
        digitalWrite(11, LOW);
    }
    if (millis() - _startMute > 150)
    {
        digitalWrite(12, LOW);
    }
}
*/

/**/
// STEPPER MUTE
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

void Chime::Tick()
{
    //_tuneStepper.Tick();
    _tuneStepper.run();
    _pickStepper.run();

    PickTick();
    MuteTick();
}