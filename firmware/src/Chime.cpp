
#include "Arduino.h"
#include "Chime.h"

Chime::Chime(int chimeId, uint8_t pinStepperTuneStep, uint8_t pinStepperTuneDirection,
             uint8_t pinStepperPickStep, uint8_t pinStepperPickDirection,
             uint8_t pinStepperMuteStep, uint8_t pinStepperMuteDirection)
    : _tuneStepper(AccelStepper::DRIVER, pinStepperTuneStep, pinStepperTuneDirection),
      _pickStepper(AccelStepper::DRIVER, pinStepperPickStep, pinStepperPickDirection),
      _muteStepper(AccelStepper::DRIVER, pinStepperMuteStep, pinStepperMuteDirection)
{
    _chimeId = chimeId;
    SetStepperParameters();
}

void Chime::SetStepperParameters()
{
    _tuneStepper.setPinsInverted(true, false, false);
    _tuneStepper.setMaxSpeed(1000);
    _tuneStepper.setAcceleration(1500);

    _pickStepper.setPinsInverted(true, false, false);
    _pickStepper.setMaxSpeed(10000);
    _pickStepper.setAcceleration(4000);

    _muteStepper.setMaxSpeed(20000);
    _muteStepper.setAcceleration(20000);
}

float Chime::NoteIdToFrequency(int n)
{
    // https://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies
    return 440 * pow(2, (n - 69) / 12);
}

bool Chime::TuneFrequency(float detectedFrequency, float targetFrequency)
{
    static unsigned int detectionCount = 0;
    static unsigned long startTimeNewTarget;

    bool frequencyWithinTolerance = false;
    float frequencyTolerance = 1.0;
    char directionText[5];

    // Return if frequency was not detected.
    if (detectedFrequency == 0)
    {
        return;
    }

    float runTimeCoef = 2;
    float targetPosition = runTimeCoef * (targetFrequency - detectedFrequency);

    int minPos = 2;
    int maxPos = 100;

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
        sprintf(directionText, "  UP");
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + int(targetPosition));
    }
    else if (detectedFrequency > targetFrequency + frequencyTolerance)
    {
        sprintf(directionText, "DOWN");
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + int(targetPosition));
    }
    else
    {
        sprintf(directionText, "STOP");
        startTimeNewTarget = millis();
        targetPosition = 0;
        _tuneStepper.moveTo(_tuneStepper.currentPosition());
        frequencyWithinTolerance = true;
    }

    static unsigned long startTimeBetweenFreqDetections = millis();
    if (millis() - startTimeBetweenFreqDetections > 100)
        detectionCount = 0;

    Serial.printf("%4u (%4ums) | Detected: %3.2f | Target: %3.2f | Delta: % 7.2f | targetPosition: % 7.2f (%3i) | %s | Step Speed % 5.0f\n",
                  detectionCount, millis() - startTimeBetweenFreqDetections, detectedFrequency, targetFrequency, targetFrequency - detectedFrequency, targetPosition, int(targetPosition), directionText, _tuneStepper.speed());

    startTimeBetweenFreqDetections = millis();

    return frequencyWithinTolerance;
}

// Pick the string if the pick motor is not in motion.
void Chime::Pick()
{
    if (!_pickStepper.isRunning())
    {
        _pickStepper.setCurrentPosition(0);
        _pickStepper.moveTo(_stepsPerPick);
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

// Returns true when calibration is complete.
// Call this method contiuously until calibration is complete.
bool Chime::CalibratePick(float detectedFrequency)
{
    if (calibrateDoOnceFlag)
    {
        _pickStepper.setMaxSpeed(125);
        _pickStepper.setAcceleration(1000);
        _pickStepper.setCurrentPosition(0);
        _pickStepper.moveTo(_stepsPerPick * 4);
        calibrateDoOnceFlag = false;
    }

    if (detectedFrequency > 0)
    {
        _pickStepper.moveTo(_pickStepper.currentPosition() + _stepsPerPick / 8);
        _pickStepper.runToPosition();

        calibrateDoOnceFlag = true;
        SetStepperParameters();
        return true;
    }

    _pickStepper.run();
    return false;
}

bool Chime::ResetCalibrateStepsToNotes()
{
    noteId = _lowestNote;
    frequencyDetectionTimeoutMillis = 0;
}

// Call in a loop until the return is true.
bool Chime::CalibrateStepsToNotes(float detectedFrequency)
{
    Tick();

    // Tune string to target frequency.
    // Check if frequency is within tolerance.
    float targetFrequency = NoteIdToFrequency(noteId);
    if (TuneFrequency(detectedFrequency, targetFrequency))
    {
        stepsToNotes[noteId] = _tuneStepper.currentPosition();

        if (noteId++ == _highestNote)
        {
            return true;
        }

        _tuneStepper.setCurrentPosition(0);

        Serial.printf("Target of %3.2f found.\n", targetFrequency);
    }

    // Check if string needs picked.
    if (detectedFrequency > 0)
    {
        frequencyDetectionTimeoutMillis = millis();
    }
    else
    {
        if (millis() - frequencyDetectionTimeoutMillis > frequencyDetectionTimeoutMs)
        {
            frequencyDetectionTimeoutMillis = millis();
            Pick();
        }
    }
}

void Chime::Tick()
{
    _tuneStepper.run();
    _pickStepper.run();
    _muteStepper.run();

    MuteTick();
}
