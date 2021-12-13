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
    Chime(int chimeId, AudioAnalyzeNoteFrequency &notefreq, uint8_t pinMotorTunePhase, uint8_t pinMotorTuneEnable, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinSolenoidMute);

    void PrepareCalibrateStepsToNotes();
    bool CalibrateStepsToNotes(float detectedFrequency);
    void CalibrateFrequencyPerStep();

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

    bool IsTargetNoteReached();

    int GetTuneCurrentSteps();

    // Mthods for development assistance.
    void TimeBetweenHighAndLowNotes();

    unsigned long temp;
    int setTime;

private:
    bool TuneNote(int targetNoteId);

    float GetFrequency();
    void SetStepperParameters();   

    // Chime.
    const float noFrequencyDetected = 0.0;
    const float acceptableProbability = 0.995;


    int _chimeId;
    bool _vibrato;
    enum class ChimeState
    {
        Calibrate,
        Pretune,
        Tune
    } _chimeState;

    const int highestNote[5] = {0, 69, 64, 60, 55};
    const int lowestNote[5] = {0, 60, 56, 51, 50};

    // Tuning.
    AudioAnalyzeNoteFrequency *notefreq;
    int _targetNoteId;
    int _previousNoteId;
    int _lockedInNoteId;


    const float _regressionCoef = 0.28;
    const int nullNoteId = 0;

    // Positions.
    const int _stepsPerRestringCommand = (1036 * 2) / 36;
    const int _stepsPerPick = 50;
    const int _stepsPerMute = 2048;

    // Volume.
    const int _stepsToMaxVolume = 825;
    const int _stepsToMinVolume = 0;
    const int _stepsPerAdjustment = 50;
    int _stepsCurrentVolume = 0;

    // Mute.
    bool _muteReturnToOpenFlag;
    bool _muteState;

    // Cailbration.
    unsigned long frequencyDetectionTimeoutMillis = millis();
    const unsigned int frequencyDetectionTimeoutMs = 500;
    int noteId;
    int betweenNotesMillis;

    int _lowestNote = 60;
    int _highestNote = 69;

    AccelStepper _tuneStepper;
    AccelStepper _pickStepper;
    AccelStepper _muteStepper;

    uint8_t _pinMotorPickLimit;
    const int _motorPickIndexActivated = 1;

    uint8_t _pinSolenoidMute;

    unsigned long _startPick;
    unsigned long _startMute;

    // Tuning variables.
    enum TuneStates
    {
        StepTune,
        FreeTune,
        WaitForStepTune,
    };
    TuneStates _tuneState = FreeTune;
    int _currentNoteId = 0;

    // FrequenctPerStep.
    enum FreqPerStepStates
    {
        Home,
        Move,
        WaitForMove,
        WaitForReading
    };

    FreqPerStepStates _freqPerStepState = FreqPerStepStates::Home;
    int _readingsCount;
    int _totalReadingsCount;
    const int _numReadingsToAverage = 10;
    const int _stepsBetweenReadings = 10;
    const int _maxSteps = 700;
    movingAvg _freqAverage = movingAvg(_numReadingsToAverage);
    float frequencyReadings[200];

    int stepsToNotes[80];

    // Development vars.
    unsigned long startTime;
    unsigned long startTimeout;
};