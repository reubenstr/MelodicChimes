#include "arduino.h"

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
    uint8_t _coilPin;
    uint8_t _pickPin;
    uint8_t _stepperStepPin;
    uint8_t _stepperDirectionPin;

    PickSide _pickSide;

    ActionState _actionState;
    ActionPhase _actionPhase;

    bool _phaseComplete;

    int _muteTimer;
    int _pickTimer;

    int _startNoteNumber;
    int _targetNoteNumber;
};