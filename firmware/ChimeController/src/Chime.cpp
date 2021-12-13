#include "Arduino.h"
#include "Chime.h"

/*
NOTES:

    // RPM from steps/sec (accelStepper's speed units):
    (n steps/ms * 1000) / (200 steps/rev * 0.5 halfsteps) * 60 = RPM

    // At 1000us between steps
    (1 * 1000) / (400) * 60 = 150 RPM
    
    // At 250us between steps
    (4 * 250) / (400) * 60 = 600 RPM

*/

Chime::Chime(int chimeId, AudioAnalyzeNoteFrequency &notefreq,
             uint8_t pinStepperTuneStep, uint8_t pinStepperTuneDirection,
             uint8_t pinStepperPickStep, uint8_t pinStepperPickDirection,
             uint8_t pinStepperMuteStep, uint8_t pinStepperMuteDirection)
    : _tuneStepper(AccelStepper::DRIVER, pinStepperTuneStep, pinStepperTuneDirection),
      _pickStepper(AccelStepper::DRIVER, pinStepperPickStep, pinStepperPickDirection),
      _muteStepper(AccelStepper::DRIVER, pinStepperMuteStep, pinStepperMuteDirection)
{
    _chimeId = chimeId;
    SetStepperParameters();

    this->notefreq = &notefreq;
    this->notefreq->begin(0.15);
}

void Chime::SetStepperParameters()
{
    _tuneStepper.setMaxSpeed(1000);
    _tuneStepper.setAcceleration(10000);

    _pickStepper.setMaxSpeed(2500);
    _pickStepper.setAcceleration(20000);

    _muteStepper.setMaxSpeed(1250);
    _muteStepper.setAcceleration(7500);
}

// Convert MIDI note number to frequency.
// https://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies
float Chime::NoteIdToFrequency(float noteId)
{
    return 440 * pow(2, (noteId - 69) / 12);
}

int Chime::GetLowestNote()
{
    return lowestNote[_chimeId];
}

int Chime::GetHighestNote()
{
    return highestNote[_chimeId];
}

int Chime::GetChimeId()
{
    return _chimeId;
}

/*
bool IsFrequencyWithinTolerance(float frequency1, float frequency2, float tolerance)
{
    return fabs(frequency1 - frequency1) < tolerance;
}
*/

float Chime::GetFrequency()
{
    if (notefreq->available())
    {
        if (notefreq->probability() > acceptableProbability)
        {
            return notefreq->read();
        }
    }

    return noFrequencyDetected;
}

void Chime::PretuneNote(int noteId)
{
    if (_lockedInNoteId != nullNoteId)
    {
        _chimeState = ChimeState::Pretune;
        float targetFrequency = NoteIdToFrequency(noteId);
        float detectedFrequency = NoteIdToFrequency(_lockedInNoteId);
        int positionDelta = _regressionCoef * (targetFrequency - detectedFrequency);

        Serial.printf("[Pretune] Locked: %3.2f | Target: %3.2f | positionDelta: %i\n",
                      detectedFrequency, targetFrequency, positionDelta);

        _tuneStepper.moveTo(_tuneStepper.currentPosition() + positionDelta);
        _lockedInNoteId = noteId;
    }
}

// No step prediction, just tune right away (P-controller).
// Return true of detected note is within tolerance of target note.
bool Chime::TuneNote(int targetNoteId)
{
    static unsigned int detectionCount = 0;
    static unsigned long startTimeNewTarget = millis();

    bool frequencyWithinTolerance = false;
    float frequencyTolerance = 1.0;
    char directionText[5];

    if (!IsNoteWithinChimesRange(targetNoteId))
    {
        return false;
    }

    float detectedFrequency = GetFrequency();

    // Return if frequency was not detected.
    if (detectedFrequency == 0) // TODO CHANBGE TO CONST already delcared nofreqdetected
    {
        return false;
    }

    // TODO: check for acceptable freq range for chime

    // TEMP: EMI is causing false freq detections at ~140hz.
    if (detectedFrequency < 230)
    {
        //  return false;
    }

    //_detectedFrequency = detectedFrequency;

    float targetFrequency = NoteIdToFrequency(targetNoteId);

    // Based on linear regression test.
    float targetPosition = int(_regressionCoef * (targetFrequency - detectedFrequency));

    int minPos = 1;
    int maxPos = 1000;

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

    Serial.printf("[%u] [%u] | %4u (%4ums) | noteId: %u | Detected: %3.2f | Target: %3.2f | Delta: % 7.2f | Target Position: %3i | %s | Step Speed: % 5.0f | Current Position: %i\n",
                  millis(), _chimeId, detectionCount, millis() - startTimeBetweenFreqDetections,
                  targetNoteId, detectedFrequency, targetFrequency,
                  targetFrequency - detectedFrequency, int(targetPosition), directionText,
                  _tuneStepper.speed(), _tuneStepper.currentPosition());

    startTimeBetweenFreqDetections = millis();

    return frequencyWithinTolerance;
}

void Chime::SetVibrato(bool flag)
{
    _vibrato = flag;
}

void Chime::RestringTighten()
{
    _chimeState = ChimeState::Calibrate;
    _tuneStepper.moveTo(_tuneStepper.currentPosition() + _stepsPerRestringCommand);
}

void Chime::RestringLoosen()
{
    _chimeState = ChimeState::Calibrate;
    _tuneStepper.moveTo(_tuneStepper.currentPosition() - _stepsPerRestringCommand);
}

void Chime::VolumePlus()
{

    if (_muteStepper.currentPosition())

    _muteStepper.moveTo(_muteStepper.currentPosition() + _stepsPerAdjustment);
}

void Chime::VolumeMinus()
{
    _muteStepper.moveTo(_muteStepper.currentPosition() - _stepsPerAdjustment);
}

void Chime::SetMaxVolume()
{
    _muteStepper.setCurrentPosition(-_stepsToMaxVolume);
    _muteStepper.moveTo(_stepsToMaxVolume);
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

bool Chime::IsNoteWithinChimesRange(int noteId)
{
    return (noteId <= highestNote[_chimeId] && noteId >= lowestNote[_chimeId]);
}

void Chime::SetTargetNote(int noteId)
{
    _chimeState = ChimeState::Tune;

    if (_targetNoteId != noteId)
    {
        _targetNoteId = noteId;
        _lockedInNoteId = nullNoteId;
    }
}

void Chime::Tick()
{
    if (_chimeState == ChimeState::Tune)
    {
        if (TuneNote(_targetNoteId))
        {
            _lockedInNoteId = _targetNoteId;
        }
    }

    _tuneStepper.run();
    _pickStepper.run();
    _muteStepper.run();
}

bool Chime::IsTargetNoteReached()
{
    return (_lockedInNoteId == _targetNoteId);
}

int Chime::GetTuneCurrentSteps()
{
    return _tuneStepper.currentPosition();
}

// Calibrate pick to known position.
// Blocking routine until a frequency is detected or timeout.
bool Chime::CalibratePick()
{
    _pickStepper.setMaxSpeed(125);
    _pickStepper.setAcceleration(1000);

    _pickStepper.setCurrentPosition(0);
    _pickStepper.moveTo(_stepsPerPick * 4);

    while (GetFrequency() == 0)
    { 
        if (_pickStepper.distanceToGo() == 0)
        {
            return false;
        }
    }

    _pickStepper.moveTo(_pickStepper.currentPosition() + _stepsPerPick / 16);

    SetStepperParameters();

    return true;
}

// Detect time between low note and high note for development and calibration.
void Chime::TimeBetweenHighAndLowNotes()
{
    SetTargetNote(GetLowestNote());

    Pick();

    while (true)
    {
        delay(1);

        if (IsTargetNoteReached())
        {
            Serial.println("**** TARGET REACHED ****");

            if (_targetNoteId == GetHighestNote())
            {
                break;
            }
            else if (_targetNoteId != GetHighestNote())
            {
                startTime = millis();
                SetTargetNote(GetHighestNote());
            }
        }
    }

    PretuneNote(GetLowestNote());

    Serial.printf("\nTime between lowest note (%u) and highest note (%u) on chime (%u): %ums\n\n",
                  GetLowestNote(), GetHighestNote(), _chimeId, millis() - startTime);
}

// Takes an averaged frequency reading per n steps.
// Used for linear regression calculation for tuning P-controller coefficient.
void Chime::CalibrateFrequencyPerStep()
{
    _freqPerStepState = FreqPerStepStates::Home;
    _totalReadingsCount = 0;

    // _tuneStepper.setMaxSpeed(500);
    // _tuneStepper.setAcceleration(250);

    SetTargetNote(GetLowestNote());
    
    _freqAverage.begin();

    Serial.println("**** CALCULATE FREQUENCY PER STEPS ****");

    while (1)
    {
        float detectedFrequency = GetFrequency();

        if (_freqPerStepState == FreqPerStepStates::Home)
        {
            if (IsTargetNoteReached())
            {
                _tuneStepper.setCurrentPosition(0);
                _chimeState = ChimeState::Calibrate;
                _freqPerStepState = FreqPerStepStates::Move;
            }
        }
        else if (_freqPerStepState == FreqPerStepStates::Move)
        {
            _tuneStepper.move(_stepsBetweenReadings);
            _freqPerStepState = FreqPerStepStates::WaitForMove;
        }
        else if (_freqPerStepState == FreqPerStepStates::WaitForMove)
        {
            if (_tuneStepper.currentPosition() > _maxSteps)
            {
                Serial.println("**** MAX STEPS REACHED, HALTING ****");
                break;
            }
            if (_tuneStepper.distanceToGo() == 0)
            {
                _readingsCount = 0;
                _freqPerStepState = FreqPerStepStates::WaitForReading;
            }
        }
        else if (_freqPerStepState == FreqPerStepStates::WaitForReading)
        {
            if (detectedFrequency > 0)
            {
                _freqAverage.reading(detectedFrequency);
                _readingsCount++;
            }
            if (_readingsCount == _numReadingsToAverage)
            {
                Serial.printf("Average frequency per step set: %3.2f, Step count: %u.\n", _freqAverage.getAvg(), _tuneStepper.currentPosition());
                frequencyReadings[_totalReadingsCount] = _freqAverage.getAvg();
                _totalReadingsCount++;

                if (detectedFrequency > NoteIdToFrequency(_highestNote))
                {
                    Serial.printf("\n**** Chime %u finished with FrequencyPerStep. ****\n", _chimeId);
                    for (int i = 0; i < _totalReadingsCount; i++)
                    {
                        Serial.printf("%3.2f\n", frequencyReadings[i]);
                    }
                    // TODO: save values if used for actual tuning.
                    break;
                }
                _freqPerStepState = FreqPerStepStates::Move;
            }
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

    _chimeState = ChimeState::Tune;
}
