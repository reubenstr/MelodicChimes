#include "Arduino.h"
#include <AccelStepper.h>
#include <Audio.h>
/*

    Pick Stepper:
    200 steps/rev, 1/4 steps, 20/10 gearbox, 3 plectrum/rev
    = 132 steps per pick

*/

class Chime
{

public:
    Chime(int chimeId, uint8_t pinMotorTunePhase, uint8_t pinMotorTuneEnable, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinSolenoidMute);

    float NoteIdToFrequency(int n);
    bool TuneFrequency(float detectedFrequency, float targetFrequency);

    bool CalibratePick(float detectedFrequency);

    bool ResetCalibrateStepsToNotes();
    bool CalibrateStepsToNotes(float detectedFrequency);

    void Pick();
    void Mute();
    void Tick();

    unsigned long temp;
    int setTime;

private:
    void SetStepperParameters();
    void MuteTick();

    int _chimeId;
    int stepsToNotes[80];

    // Cailbration variables.
    unsigned long frequencyDetectionTimeoutMillis = millis();
    const int frequencyDetectionTimeoutMs = 1000;
    int noteId;

    int _lowestNote = 60;
    int _highestNote = 69;

    // Pick Stepper:  200 steps/rev, 1/4 steps, 10/20 gearbox, 6 plectrum/rev
    // 200 * 4 * 0.5 / 6 = 66.666

    const int _stepsPerPick = 66;

    AccelStepper _tuneStepper;
    AccelStepper _pickStepper;
    AccelStepper _muteStepper;

    float _targetFrequency;

    uint8_t _pinMotorPickLimit;
    const int _motorPickIndexActivated = 1;

    uint8_t _pinSolenoidMute;

    unsigned long _startPick;
    unsigned long _startMute;

    bool calibrateDoOnceFlag = true;
};
