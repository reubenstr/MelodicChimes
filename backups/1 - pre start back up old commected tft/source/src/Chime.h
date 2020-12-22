#include "arduino.h"
#include "SimpleStepper.h"
#include "arduinoFFT.h"

class Chime
{

public:
    // Constructor
    Chime(uint8_t coilPin, uint8_t pickPin, uint8_t stepperStepPin, uint8_t stepperDirectionPin);

    // Destructor.
    ~Chime(void);

    struct ChimeInit
    {
        uint8_t coilPin;
        uint8_t pickPin;
        uint8_t stepperStepPin;
        uint8_t stepperDirectionPin;
        uint8_t minNote;
        uint8_t maxNote;
    };

    enum ChimeResult
    {
        NoError,
        FrequencyNotDetected,
        StepperMovementHasNoRepsonse
    };

    enum PickSide
    {
        Left = 0,
        Right = 1
    };

    // Functions.
    void PlayNote(uint8_t noteNumber);
    void PlayGlissando(uint8_t noteNumber, float velocity);
    void PlayPortamento(uint8_t startNoteNumber, uint8_t targetNoteNumber, float velocity);
    bool Update();

    float SampleCoilForFrequency(uint8_t _coilPin);

private:
    // Functions.
    bool CalibrateNotePosition();
    bool MuteString(PickSide _pickSide);
    bool PickString(PickSide &_pickSide);
    bool MoveIntoPosition(uint8_t targetNoteNumber);
    bool Tune(uint8_t targetNoteNumber);


    // Simple helper functions.
    void InvertPickSide(PickSide &_pickSide);
    void ResetMuteTimer();
    void ResetPickTimer();

    enum ActionState
    {
        Mute,
        Note,
        Glissando,
        Portamento
    };

    enum ActionPhase
    {
        MuteStringPhase,
        MoveIntoPositionPhase,
        PickStringPhase,
        TunePhase
    };

    // Variables.

    SimpleStepper *stepper;

    uint8_t _coilPin;
    uint8_t _pickPin;

    PickSide _pickSide;

    ActionState _actionState;
    ActionPhase _actionPhase;

    bool _phaseComplete;

    unsigned long _muteTimer;
    unsigned long _pickTimer;

    int _startNoteNumber;
    int _targetNoteNumber;

    // FFT variables.

    static const int samples = 128;             //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno.
    static const int samplingFrequency = 1024; //Ts = Based on Nyquist, must be 2 times the highest expected frequency.
    arduinoFFT FFT = arduinoFFT();

    unsigned int _samplingPeriod;
    unsigned long _microSeconds;

    double vReal[samples]; //create vector of size SAMPLES to hold real values
    double vImag[samples]; //create vector of size SAMPLES to hold imaginary values
};