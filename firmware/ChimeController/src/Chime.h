#include "Arduino.h"
#include <AccelStepper.h>
#include <Audio.h>
#include "movingAvg.h"

// Notes conform to MIDI note numbers, example: https://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies

/*
    Pick Stepper:
        200 steps/rev, 1/2 microsteps, plectrum with 8 points
        200 * 1/2 / 8 = 50 steps per pick
    Volume Stepper: 28byj-48
        2048 steps/rev, 1/2 microsteps
*/

class Chime
{

public:
    Chime(int chimeId, AudioAnalyzeNoteFrequency &notefreq, uint8_t pinStepperTuneStep, uint8_t pinStepperTuneDirection, uint8_t pinStepperVolumeStep, uint8_t pinStepperVolumeDirection, uint8_t pinStepperPickStep, uint8_t pinStepperPickDirection);

    void PrepareCalibrateStepsToNotes();
    bool CalibrateStepsToNotes(float detectedFrequency);

    void RestringTighten();
    void RestringLoosen();
    void VolumePlus();
    void VolumeMinus();
    void SetMaxVolume();
    void SetTargetNote(int noteId);
    void PretuneNote(int noteId);
    void SetVibrato(bool flag);

    float NoteIdToFrequency(float noteId);

    void Pick();
    void Tick();
    bool CalibratePick();

    int GetLowestNote();
    int GetHighestNote();

    int GetChimeId();
    bool IsNoteWithinChimesRange(int noteId);
    int GetTuneCurrentSteps();

    // Developing methods.
    void CaptureTimeFromLowToHighNote();
    void CaptureFrequencyPerStep();

private:
    bool TuneNote(int targetNoteId);

    float GetFrequency();
    void SetStepperParameters();

    // Chime.
    const float _noFrequencyDetected = 0.0;
    const float _acceptableProbability = 0.99;

    int _chimeId;
    bool _vibrato;
    enum class ChimeState
    {
        Calibrate,
        Tune,
        Vibrato
    } _chimeState;

    const int highestNote[3] = {69, 64, 60};
    const int lowestNote[3] = {60, 56, 51};

    // Tuning.
    AudioAnalyzeNoteFrequency *notefreq;
    int _targetNoteId;
    int _lockedInNoteId;
    const float _frequencyTolerance = 1.0;

    const float _secondOrderCoef[3] = {0.0041, 0.0039, 0.00155};
    const float _firstOrderCoef[3] = {0.4195, 0.0104, 1.19004};
    const float _manualCoef[3] = {0.5, 0.5, 0.5};

    const int _nullNoteId = 0;
    bool _vibratoToggle;
    const int _vibratoSteps = 10;

    // Positions.
    const int _stepsPerRestring = (1036 * 2) / 36;
    const int _stepsPerPick = 200 * 8 / 8;

    // Volume.
    const int _stepsToMaxVolume = 825;
    const int _stepsToMinVolume = 0;
    const int _stepsPerAdjustment = 50;
    int _stepsCurrentVolume = 0;

    AccelStepper _tuneStepper;
    AccelStepper _pickStepper;
    AccelStepper _volumeStepper;

    unsigned long _startPick;
    unsigned long _startMute;

    // Tuning development and logging
    unsigned long _frequencyDetectionTimeoutMillis = millis();
    const unsigned int _frequencyDetectionTimeoutMs = 500;
    unsigned long _startTimeBetweenFreqDetections = millis();
    unsigned int _detectionCount = 0;
};