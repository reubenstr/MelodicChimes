
#include "Arduino.h"
#include "DCMotor.h"

#include "QuickStepper.h"

class Chime
{

public:
    Chime(uint8_t pinMotorTunePhase, uint8_t pinMotorTuneEnable, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinSolenoidMute);
    void TuneFrequency(float detectedFrequency, float targetFrequency);
    void Tick();

    void Pick();
    void Mute();


     QuickStepper _stepper; //TEMP

private:
    void PickTick();
    void MuteTick();

    DCMotor _tuneMotor;
    DCMotor _pickMotor;

   

    float _targetFrequency;

    uint8_t _pinMotorPickLimit;
    const int _motorPickIndexActivated = 1;

    uint8_t _pinSolenoidMute;

    unsigned long _startPick;
    unsigned long _startMute;
};

