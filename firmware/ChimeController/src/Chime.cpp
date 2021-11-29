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
    _tuneStepper.setPinsInverted(false, false, false);

    // TEMP TEST
    _tuneStepper.setMaxSpeed(1000);
    _tuneStepper.setAcceleration(10000);

    _pickStepper.setPinsInverted(false, false, false);
    _pickStepper.setMaxSpeed(4000);
    _pickStepper.setAcceleration(8000);

    _muteStepper.setPinsInverted(true, false, false);
    _muteStepper.setMaxSpeed(8000);
    _muteStepper.setAcceleration(25000);
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

// helper for dev/testing
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

    // TEMP: EMI is causing false freq detections at ~140hz.
    if (detectedFrequency < 230)
    {
        return false;
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

    Serial.printf("[%u] | %4u (%4ums) | noteId: %u | Detected: %3.2f | Target: %3.2f | Delta: % 7.2f | Target Position: %3i | %s | Step Speed: % 5.0f | Current Position: %i\n",
                  _chimeId, detectionCount, millis() - startTimeBetweenFreqDetections,
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
    //_chimeState = ChimeState::Calibrate;
    _tuneStepper.moveTo(_tuneStepper.currentPosition() + _stepsPerRestringCommand);
}

void Chime::RestringLoosen()
{
   // _chimeState = ChimeState::Calibrate;
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

void Chime::Mute()
{
    if (!_muteState)
    {
        _muteState = true;
        _muteStepper.setCurrentPosition(0);
        _muteStepper.moveTo(_stepsPerMute);
    }

    /*
    _muteStepper.setCurrentPosition(0);
    _muteStepper.moveTo(_stepsPerMute);

    _muteReturnToOpenFlag = true;
    _startMute = millis();
    */
}

void Chime::UnMute()
{
    if (_muteState)
    {
        _muteState = false;
        _muteStepper.setCurrentPosition(0);
        _muteStepper.moveTo(-_stepsPerMute);
    }
}

void Chime::MuteTick()
{
    /*
    if (_muteReturnToOpenFlag && _muteStepper.distanceToGo() == 0)
    {
        _muteReturnToOpenFlag = false;
        _muteStepper.setCurrentPosition(0);
        _muteStepper.moveTo(-_stepsPerMute);
    }
    */
}

void Chime::PrepareCalibrateStepsToNotes()
{
    noteId = _lowestNote - 1;
    frequencyDetectionTimeoutMillis = 0;
    betweenNotesMillis = millis();
}

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

    SetTargetNote(GetLowestNote());

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

    MuteTick();
}

// Methods for development testing.
bool Chime::IsTargetNoteReached()
{
    return (_lockedInNoteId == _targetNoteId);
}

int Chime::GetTuneCurrentSteps()
{
    return _tuneStepper.currentPosition();
}

void Chime::TimeBetweenHighAndLowNotes()
{
    SetTargetNote(GetLowestNote());

    startTimeout = millis();

    //Pick();

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
