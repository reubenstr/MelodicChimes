#include "Arduino.h"
#include <AccelStepper.h>

#include "ChimeStepper.h"

#define PIN_STEPPER_TUNE_STEP 14
#define PIN_STEPPER_TUNE_DIRECTION 15

#define PIN_STEPPER_MUTE_STEP 18
#define PIN_STEPPER_MUTE_DIRECTION 19

#define PIN_STEPPER_PICK_STEP 10
#define PIN_STEPPER_PICK_DIRECTION 9

class Chime
{

public:
    Chime(uint8_t pinMotorTunePhase, uint8_t pinMotorTuneEnable, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinSolenoidMute);
    void TuneFrequency(float detectedFrequency, float targetFrequency);
    void Tick();

    void Pick();
    void Mute();


     
private:
    void PickTick();
    void MuteTick();

    //DCMotor _tuneMotor;
    //DCMotor _pickMotor;

    AccelStepper _muteStepper;
    AccelStepper _tuneStepper;
    AccelStepper _pickStepper;

    float _targetFrequency;

    uint8_t _pinMotorPickLimit;
    const int _motorPickIndexActivated = 1;

    uint8_t _pinSolenoidMute;

    unsigned long _startPick;
    unsigned long _startMute;
};