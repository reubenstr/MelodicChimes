
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
}

void Chime::SetStepperParameters()
{
    _tuneStepper.setPinsInverted(true, false, false);
    _tuneStepper.setMaxSpeed(4000);
    _tuneStepper.setAcceleration(8000);

    // TEMP FOR RESTRING TEST
        _tuneStepper.setMaxSpeed(1500);
    _tuneStepper.setAcceleration(3000);

    _pickStepper.setPinsInverted(true, false, false);
    _pickStepper.setMaxSpeed(10000);
    _pickStepper.setAcceleration(4000);

    _muteStepper.setMaxSpeed(20000);
    _muteStepper.setAcceleration(20000);
}



// Convert MIDI note number to frequency.
float Chime::NoteIdToFrequency(float noteId)
{
    // https://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies
    return 440 * pow(2, (noteId - 69) / 12);
}

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
        TuneFrequency(detectedFrequency, targetFrequency);
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

// No step prediction, just tune right away (P-controller).
bool Chime::TuneFrequency(float detectedFrequency, float targetFrequency)
{
    static unsigned int detectionCount = 0;
    static unsigned long startTimeNewTarget = millis();

    bool frequencyWithinTolerance = false;
    float frequencyTolerance = 1.0;
    char directionText[5];

    // Return if frequency was not detected.
    if (detectedFrequency == 0)
    {
        return false;
    }

    //float runTimeCoef = 2;
    //float targetPosition = runTimeCoef * (targetFrequency - detectedFrequency);

    // Based on linear regression test.
    float targetPosition = int((targetFrequency - detectedFrequency) * 4.54);

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

    Serial.printf("%4u (%4ums) | Detected: %3.2f | Target: %3.2f | Delta: % 7.2f | Target Position: %3i | %s | Step Speed: % 5.0f | Current Position: %i\n",
                  detectionCount, millis() - startTimeBetweenFreqDetections, detectedFrequency, targetFrequency, targetFrequency - detectedFrequency, int(targetPosition), directionText, _tuneStepper.speed(), _tuneStepper.currentPosition());

    startTimeBetweenFreqDetections = millis();

    return frequencyWithinTolerance;
}

void Chime::Retring(bool direction)
{  
    if (direction)
    {
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + _stepsPerRestringCommand);
    }
    else
    {
        _tuneStepper.moveTo(_tuneStepper.currentPosition() - _stepsPerRestringCommand);
    }
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

void Chime::PrepareCalibrateStepsToNotes()
{
    noteId = _lowestNote - 1;
    frequencyDetectionTimeoutMillis = 0;
    betweenNotesMillis = millis();
}

// Call in a loop until the return is true.
bool Chime::CalibrateStepsToNotes(float detectedFrequency)
{
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
}

void Chime::PrepareFrequencyPerStep()
{
    _freqPerStepState = FreqPerStepStates::Home;
    _totalReadingsCount = 0;
    _tuneStepper.setMaxSpeed(500);
    _tuneStepper.setAcceleration(250);
}

bool Chime::CalibrateFrequencyPerStep(float detectedFrequency)
{

    if (_freqPerStepState == FreqPerStepStates::Home)
    {
        float targetFrequency = NoteIdToFrequency(_lowestNote);
        if (TuneFrequency(detectedFrequency, targetFrequency))
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
                return true;
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

    Tick();

    return false;
}

void Chime::PrepareCalibratePick()
{
    _pickStepper.setMaxSpeed(125);
    _pickStepper.setAcceleration(1000);
    _pickStepper.setCurrentPosition(0);
    _pickStepper.moveTo(_stepsPerPick * 4);
}

// Returns true when calibration is complete.
// Call this method contiuously until calibration is complete.
// Start calling when string is not producing a frequency.
bool Chime::CalibratePick(float detectedFrequency)
{
    if (detectedFrequency > 0)
    {
        _pickStepper.moveTo(_pickStepper.currentPosition() + _stepsPerPick / 8);
        _pickStepper.runToPosition();

        SetStepperParameters();
        return true;
    }

    _pickStepper.run();
    return false;
}

void Chime::Tick()
{
    


   _tuneStepper.run();    
    _pickStepper.run();
    _muteStepper.run();

    MuteTick();
}
