#include "Arduino.h"
#include "Chime.h"

Chime::Chime(int chimeId, AudioAnalyzeNoteFrequency &notefreq,
             uint8_t pinStepperTuneStep, uint8_t pinStepperTuneDirection,
              uint8_t pinStepperVolumeStep, uint8_t pinStepperVolumeDirection,
             uint8_t pinStepperPickStep, uint8_t pinStepperPickDirection)            
    : _tuneStepper(AccelStepper::DRIVER, pinStepperTuneStep, pinStepperTuneDirection),
     _volumeStepper(AccelStepper::DRIVER, pinStepperVolumeStep, pinStepperVolumeDirection),
      _pickStepper(AccelStepper::DRIVER, pinStepperPickStep, pinStepperPickDirection)
      
{
    _chimeId = chimeId;
    SetStepperParameters();

    this->notefreq = &notefreq;
    this->notefreq->begin(0.15);
}

void Chime::SetStepperParameters()
{
    _tuneStepper.setMaxSpeed(5000);
    _tuneStepper.setAcceleration(50000);
	_volumeStepper.setMaxSpeed(1250);
    _volumeStepper.setAcceleration(7500);
    _pickStepper.setMaxSpeed(5000);
    _pickStepper.setAcceleration(50000);  
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

float Chime::GetFrequency()
{
    if (notefreq->available())
    {
        if (notefreq->probability() > _acceptableProbability)
        {
            return notefreq->read();
        }
    }

    return _noFrequencyDetected;
}

// Pretuning starts moving the stepper to help counteract
// frequency detection lag caused by picking.
void Chime::PretuneNote(int noteId)
{
    float targetFrequency = NoteIdToFrequency(noteId);
    float detectedFrequency = NoteIdToFrequency(_lockedInNoteId);
    int steps = _regressionCoef[_chimeId] * (targetFrequency - detectedFrequency);
    _tuneStepper.setCurrentPosition(0);
    _tuneStepper.moveTo(_tuneStepper.currentPosition() + steps);
}

// Return true if detected note is within tolerance of target note.
bool Chime::TuneNote(int targetNoteId)
{
    bool frequencyWithinTolerance = false;    
    char directionText[5];

    if (!IsNoteWithinChimesRange(targetNoteId))
    {
        return false;
    }

    float detectedFrequency = GetFrequency();
   
    if (detectedFrequency == _noFrequencyDetected)
    {
        return false;
    }

     if (detectedFrequency <  NoteIdToFrequency(highestNote[_chimeId]) / 2)
    {
        return false;
    }

    detectionCount++;

    float targetFrequency = NoteIdToFrequency(targetNoteId);
    //float targetPosition = _regressionCoef[_chimeId] * (targetFrequency - detectedFrequency);
	
	// Use results of polynomial regression.
    // TODO: move to header, contruct per string.
	float pos1 = 0.0041 * detectedFrequency * detectedFrequency + 0.4195 * detectedFrequency;
	float pos2 = 0.0041 * targetFrequency * targetFrequency + 0.4195 * targetFrequency;	
	float targetPosition = (pos2 - pos1) * _regressionCoef[_chimeId];

    if (detectedFrequency < targetFrequency - _frequencyTolerance)
    {
        sprintf(directionText, "  UP");
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + int(targetPosition));
    }
    else if (detectedFrequency > targetFrequency + _frequencyTolerance)
    {
        sprintf(directionText, "DOWN");
        _tuneStepper.moveTo(_tuneStepper.currentPosition() + int(targetPosition));
    }
    else
    {
        sprintf(directionText, "STOP");     
        targetPosition = 0;
        _tuneStepper.moveTo(0);
        _tuneStepper.setCurrentPosition(0); 
        frequencyWithinTolerance = true;
    }

    if (millis() - _startTimeBetweenFreqDetections > 100)
        detectionCount = 0;

    
    //if (_chimeId == 5)
        Serial.printf("[%u] [%u] | %4u (%4ums) | noteId: %u | Detected: %3.2f | Target: %3.2f | Delta: % 7.2f | Position Adjustment: %3i | %s | Step Speed: % 5.0f | Current Position: %i\n",
                      millis(), _chimeId, detectionCount, millis() - _startTimeBetweenFreqDetections,
                      targetNoteId, detectedFrequency, targetFrequency,
                      targetFrequency - detectedFrequency, int(targetPosition), directionText,
                      _tuneStepper.speed(), _tuneStepper.currentPosition());

    _startTimeBetweenFreqDetections = millis();

    return frequencyWithinTolerance;
}

void Chime::SetVibrato(bool flag)
{
    _vibrato = flag;
}

void Chime::RestringTighten()
{
    _chimeState = ChimeState::Calibrate;
    _tuneStepper.moveTo(_tuneStepper.currentPosition() + _stepsPerRestring);
}

void Chime::RestringLoosen()
{
    _chimeState = ChimeState::Calibrate;
    _tuneStepper.moveTo(_tuneStepper.currentPosition() - _stepsPerRestring);
}

void Chime::VolumePlus()
{
    if (_volumeStepper.currentPosition())
        _volumeStepper.moveTo(_volumeStepper.currentPosition() + _stepsPerAdjustment);
}

void Chime::VolumeMinus()
{
    _volumeStepper.moveTo(_volumeStepper.currentPosition() - _stepsPerAdjustment);
}

void Chime::SetMaxVolume()
{
    _volumeStepper.setCurrentPosition(-_stepsToMaxVolume);
    _volumeStepper.moveTo(_stepsToMaxVolume);
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

        if (_lockedInNoteId != nullNoteId)
        {
            PretuneNote(_targetNoteId);
        }

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
			
			 if (_vibrato)
			 {
				 _chimeState = ChimeState::Vibrato;				 
				 _tuneStepper.setCurrentPosition(0);
				 _tuneStepper.moveTo(_tuneStepper.currentPosition());
			 }			 
        }	
    }	
	else if (_chimeState == ChimeState::Vibrato)
	{
		if (_tuneStepper.distanceToGo() == 0)
		{					
			_vibratoToggle = !_vibratoToggle;
			 _tuneStepper.moveTo(_vibratoToggle ? _vibratoSteps : -_vibratoSteps);
		}		
	}

    _tuneStepper.run();
    _pickStepper.run();
    _volumeStepper.run();
}


int Chime::GetTuneCurrentSteps()
{
    return _tuneStepper.currentPosition();
}

// Calibrate pick to known position.
// Blocking routine until a frequency is detected or timeout.
bool Chime::CalibratePick()
{
    _pickStepper.setMaxSpeed(300);
    _pickStepper.setAcceleration(2500);

    _pickStepper.setCurrentPosition(0);
    _pickStepper.moveTo(_stepsPerPick * 4);

    while (GetFrequency() == _noFrequencyDetected )
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
void Chime::CaptureTimeFromLowToHighNote()
{
    unsigned long startTime;

    SetTargetNote(GetLowestNote());

    Pick();

    while (true)
    {
        delay(1);

        if (_lockedInNoteId == _targetNoteId)
        {
            Serial.println("*** TARGET REACHED ***");

            if (_lockedInNoteId == GetHighestNote())
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

    _chimeState = ChimeState::Calibrate;

    Serial.printf("\nTime between lowest note (%u) and highest note (%u) on chime (%u): %ums\n\n",
                  GetLowestNote(), GetHighestNote(), _chimeId, millis() - startTime);
}

// Takes an averaged frequency reading per n steps.
// Used for linear regression calculation for tuning P-controller coefficient.
void Chime::CaptureFrequencyPerStep()
{
   

    enum FreqPerStepStates
    {
        Home,
        Move,
        WaitForMove,
        WaitForReading
    };

    FreqPerStepStates _freqPerStepState = FreqPerStepStates::Home;
    int _readingsCount;
    int _totalReadingsCount = 0;
    const int _numReadingsToAverage = 10;
    const int _stepsBetweenReadings = 10;
    const int _maxSteps = 700;
    movingAvg _freqAverage = movingAvg(_numReadingsToAverage);
    float frequencyReadings[200];

    // _tuneStepper.setMaxSpeed(500);
    // _tuneStepper.setAcceleration(250);

    _lockedInNoteId = nullNoteId;
    SetTargetNote(GetLowestNote());
    

    _freqAverage.begin();

    Serial.println("*** CALCULATE FREQUENCY PER STEPS ***");

    while (1)
    {
        float detectedFrequency = GetFrequency();

        if (_freqPerStepState == FreqPerStepStates::Home)
        {
            if (_lockedInNoteId == GetLowestNote())
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
                Serial.println("*** ERROR: MAX STEPS REACHED, HALTING ***");
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

                if (detectedFrequency > NoteIdToFrequency(GetHighestNote()))
                {
                    Serial.printf("\n*** Chime %u finished with FrequencyPerStep. (%u steps between samples) ***\n", _chimeId, _stepsBetweenReadings);
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

     _chimeState = ChimeState::Calibrate;
    
}