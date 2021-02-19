#include "Arduino.h"
#include "Chime.h"

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

    // TEMP
    // Manual add steps
    stepsToNotes[60] = 96;
    stepsToNotes[61] = 46;
    stepsToNotes[62] = 39;
    stepsToNotes[63] = 63;
    stepsToNotes[64] = 59;
    stepsToNotes[65] = 68;
    stepsToNotes[66] = 63;
    stepsToNotes[67] = 81;
    stepsToNotes[68] = 108;
    stepsToNotes[69] = 139;

    _freqAverage.begin();

    this->notefreq = &notefreq;
    this->notefreq->begin(0.15);
}

void Chime::SetStepperParameters()
{
    _tuneStepper.setPinsInverted(true, false, false);
    _tuneStepper.setMaxSpeed(4000);
    _tuneStepper.setAcceleration(8000);
    // TEMP FOR RESTRING TEST
    _tuneStepper.setMaxSpeed(1500);
    _tuneStepper.setAcceleration(3000);

    _pickStepper.setPinsInverted(false, false, false);
    _pickStepper.setMaxSpeed(4000);
    _pickStepper.setAcceleration(8000);

    _muteStepper.setMaxSpeed(4000);
    _muteStepper.setAcceleration(8000);
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
    const float noFrequencyDetected = 0.0;
    const float acceptableProbability = 0.995;

    if (notefreq->available())
    {
        if (notefreq->probability() > acceptableProbability)
        {
            return notefreq->read();
        }
    }

    return noFrequencyDetected;
}

/*
// Predicts steps needed to hit the frequency.
bool Chime::TuneNote(float detectedFrequency, int newNoteId)
{
    // Return if frequency was not detected.
    if (detectedFrequency == 0)
    {
        return false;
    }

    // On a new note decide which state to enter.
    if (_currentNoteId != newNoteId)
    {
        if (_tuneState == TuneStates::FreeTune)
        {
            _tuneState = TuneStates::StepTune;
        }
        else if (_tuneState == TuneStates::WaitForStepTune)
        {
            _tuneState = TuneStates::FreeTune;
        }
    }

    // Tune freely using the linear regression equation.
    if (_tuneState == TuneStates::FreeTune)
    {
        float targetFrequency = NoteIdToFrequency(newNoteId);
        //TuneFrequency(detectedFrequency, targetFrequency);
        _currentNoteId = newNoteId;
    }
    // Tune directly by using the steps to note look up table.
    else if (_tuneState == TuneStates::StepTune)
    {
        int targetPosition = 0;

        if (newNoteId < _currentNoteId)
        {
            // Tuning downwards.
            for (int i = _currentNoteId; i > newNoteId + 1; i--)
            {
                targetPosition += stepsToNotes[i];
            }
        }
        else if (newNoteId > _currentNoteId)
        {
            // Tuning upwards.
            for (int i = _currentNoteId + 1; i < newNoteId + 1; i++)
            {
                targetPosition += stepsToNotes[i];
            }
        }

        _tuneStepper.setCurrentPosition(0); // TEMP
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + targetPosition);
        _currentNoteId = newNoteId;
        _tuneState = TuneStates::WaitForStepTune;

        Serial.printf("StepTune: Target position: %i (noteID: %u | freq: %3.2f)\n", targetPosition, newNoteId, NoteIdToFrequency(newNoteId));
    }
    // Wait for steps to complete triggered by TuneStates::StepTune
    else if (_tuneState == TuneStates::WaitForStepTune)
    {
        if (_tuneStepper.distanceToGo() == 0)
        {
            _tuneState = TuneStates::FreeTune;
        }
    }
}
*/

void Chime::PretuneNote(int noteId)
{
    if (_lockedInNoteId != nullNoteId)
    {
        _lockedInNoteId = noteId;
        float targetFrequency = NoteIdToFrequency(noteId);
        float detectedFrequency = NoteIdToFrequency(_lockedInNoteId);
        int targetPosition = _regressionCoef * (targetFrequency - detectedFrequency);
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + targetPosition);
    }
}

// No step prediction, just tune right away (P-controller).
bool Chime::TuneNote(int targetNoteId)
{
    static unsigned int detectionCount = 0;
    static unsigned long startTimeNewTarget = millis();

    bool frequencyWithinTolerance = false;
    float frequencyTolerance = 1.0;
    char directionText[5];

    if (!IsNoteWithinChimesRange(targetNoteId))
    {
        return;
    }

    float detectedFrequency = GetFrequency();

    // Return if frequency was not detected.
    if (detectedFrequency == 0)
    {
        return false;
    }

    //_detectedFrequency = detectedFrequency;

    if (targetNoteId < lowestNote[_chimeId] || targetNoteId > highestNote[_chimeId])
    {
        return false;
    }

    float targetFrequency = NoteIdToFrequency(targetNoteId);

    // Based on linear regression test.
    float targetPosition = int(_regressionCoef * (targetFrequency - detectedFrequency));

    int minPos = 1;
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

    Serial.printf("[%u] | %4u (%4ums) | Detected: %3.2f | Target: %3.2f | Delta: % 7.2f | Target Position: %3i | %s | Step Speed: % 5.0f | Current Position: %i\n",
                  _chimeId, detectionCount, millis() - startTimeBetweenFreqDetections, detectedFrequency, targetFrequency, targetFrequency - detectedFrequency, int(targetPosition), directionText, _tuneStepper.speed(), _tuneStepper.currentPosition());

    startTimeBetweenFreqDetections = millis();

    return frequencyWithinTolerance;
}

void Chime::SetVibrato(bool flag)
{
    _vibrato = flag;
}

void Chime::RestringTighten()
{
    _tuneStepper.moveTo(_tuneStepper.currentPosition() + _stepsPerRestringCommand);
}

void Chime::RestringLoosen()
{
    _tuneStepper.moveTo(_tuneStepper.currentPosition() - _stepsPerRestringCommand);
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

// Mute the string briefly.
void Chime::Mute()
{
    _muteStepper.setCurrentPosition(0);
    _muteStepper.moveTo(_stepsPerMute);

    _muteReturnToOpenFlag = true;
    _startMute = millis();
}

// Unmute string after muting.
void Chime::MuteTick()
{
    if (_muteReturnToOpenFlag && _muteStepper.distanceToGo() == 0)
    {
        _muteReturnToOpenFlag = false;
        _muteStepper.setCurrentPosition(0);
        _muteStepper.moveTo(-_stepsPerMute);
    }
}

void Chime::PrepareCalibrateStepsToNotes()
{
    noteId = _lowestNote - 1;
    frequencyDetectionTimeoutMillis = 0;
    betweenNotesMillis = millis();
}

// Call in a loop until the return is true.
bool Chime::CalibrateStepsToNotes(float detectedFrequency)
{
    /*
    // Tune string to target frequency.
    // Check if frequency is within tolerance.
    float targetFrequency = NoteIdToFrequency(noteId);
    if (TuneFrequency(detectedFrequency, targetFrequency))
    {
        Serial.printf("Target of %3.2f found with %i steps over %u milliseconds.\n", targetFrequency, _tuneStepper.currentPosition(), millis() - betweenNotesMillis);
        betweenNotesMillis = millis();
        stepsToNotes[noteId] = _tuneStepper.currentPosition();
        if (noteId++ == _highestNote)
        {
            Serial.println("CalibrateStepsToNotes complete.");
            for (int i = _lowestNote - 1; i < _highestNote + 1; i++)
            {
                Serial.println(stepsToNotes[i]);
            }
            return true;
        }
        _tuneStepper.setCurrentPosition(0);
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
    Tick();
    */
}

// Takes an averaged frequency reading per n steps.
// Used for linear regression calculation for tuning P-controller coefficient.
void Chime::CalibrateFrequencyPerStep()
{
    _freqPerStepState = FreqPerStepStates::Home;
    _totalReadingsCount = 0;

    _tuneStepper.setMaxSpeed(500);
    _tuneStepper.setAcceleration(250);

    while (1)
    {
        float detectedFrequency = GetFrequency();

        if (_freqPerStepState == FreqPerStepStates::Home)
        {
            if (TuneNote(_lowestNote))
            {
                _freqPerStepState = FreqPerStepStates::Move;
            }
        }
        else if (_freqPerStepState == FreqPerStepStates::Move)
        {
            _tuneStepper.setCurrentPosition(0);
            _tuneStepper.moveTo(_stepsBetweenReadings);
            _freqPerStepState = FreqPerStepStates::WaitForMove;
        }
        else if (_freqPerStepState == FreqPerStepStates::WaitForMove)
        {
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
                frequencyReadings[_totalReadingsCount] = _freqAverage.getAvg();
                _totalReadingsCount++;
                if (detectedFrequency > NoteIdToFrequency(_highestNote))
                {
                    Serial.printf("Chime %u finished with FrequencyPerStep.\n", _chimeId);
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
}

// Calibrate pick to known position.
// Blocking routine until a frequency is detected or timeout.
bool Chime::CalibratePick()
{
    _pickStepper.setMaxSpeed(125);
    _pickStepper.setAcceleration(1000);

    _pickStepper.setCurrentPosition(0);
    _pickStepper.moveTo(_stepsPerPick * 4);

    float detectedFrequency = 0.0;

    while (detectedFrequency == 0)
    {
        detectedFrequency = GetFrequency();

        if (_pickStepper.distanceToGo() == 0)
        {
            return false;
        }
    }

    _pickStepper.moveTo(_pickStepper.currentPosition() + _stepsPerPick / 16);

    SetStepperParameters();

    return true;
}

bool Chime::IsNoteWithinChimesRange(int noteId)
{
    return (noteId <= highestNote[_chimeId] && noteId >= lowestNote[_chimeId]);
}

void Chime::SetTargetNote(int noteId)
{
    _targetNoteId = noteId;
}

void Chime::Tick()
{
    static _previousNoteId;
    if (_previousNoteId != _targetNoteId)
    {
        _previousNoteId = _targetNoteId;
        _lockedInNoteId = nullNoteId;
    }
    if (TuneNote(_targetNoteId))
    {
        _lockedInNoteId = targetNoteId;
    }

    _tuneStepper.run();
    _pickStepper.run();
    _muteStepper.run();

    MuteTick();
}
