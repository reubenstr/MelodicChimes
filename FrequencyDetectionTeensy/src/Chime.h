
#include "Arduino.h"
#include "DCMotor.h"



class Chime
{



public:
    Chime(uint8_t pinMotorTunePhase, uint8_t pinMotorTuneEnable, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinSolenoidMute);
    void FrequencyToMotor(float detectedFrequency, float targetFrequency);
    void Tick();

    void Pick();
    void Mute();
    

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