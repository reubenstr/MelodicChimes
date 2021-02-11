#include "Arduino.h"
#include <AccelStepper.h>
#include <Audio.h>
#include "movingAvg.h"


/*
    Pick Stepper:
    200 steps/rev, 1/2 steps, 20:30 pulley (20 teeth on drive, 30 teeth on pick), 8 plectrum/rev
    = 75 steps per pick
*/

class Chime
{

public:
    Chime(int chimeId, AudioAnalyzeNoteFrequency &notefreq, uint8_t pinMotorTunePhase, uint8_t pinMotorTuneEnable, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinSolenoidMute);

    float NoteIdToFrequency(float noteId);

    bool TuneNote(float detectedFrequency, int noteId);
    bool TuneFrequency(float targetFrequency);

    void PrepareCalibrateStepsToNotes();
    bool CalibrateStepsToNotes(float detectedFrequency);

    void PrepareFrequencyPerStep();
    bool CalibrateFrequencyPerStep(float detectedFrequency);

    void PrepareCalibratePick();
    bool CalibratePick(float detectedFrequency);

    void Retring(bool direction);

    void Pick();
    void Mute();
    void Tick();

    unsigned long temp;
    int setTime;

private:
    float GetFrequency();
    void SetStepperParameters();
    void MuteTick();
    
    AudioAnalyzeNoteFrequency *notefreq;
   

    int _chimeId;
    int stepsToNotes[80];

    // Cailbration variables.
    unsigned long frequencyDetectionTimeoutMillis = millis();
    const int frequencyDetectionTimeoutMs = 1000;
    int noteId;
    int betweenNotesMillis;

    int _lowestNote = 60;
    int _highestNote = 69;

    const int _stepsPerRestringCommand = 800;

    const int _stepsPerPick = 75;

    AccelStepper _tuneStepper;
    AccelStepper _pickStepper;
    AccelStepper _muteStepper;

    float _targetFrequency;

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

    // FrequenctPerStep variables
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
    const int _numReadingsToAverage = 5;
    const int _stepsBetweenReadings = 20;
    movingAvg _freqAverage = movingAvg(_numReadingsToAverage);
    float frequencyReadings[200];
};
