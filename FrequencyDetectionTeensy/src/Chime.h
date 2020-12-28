
#include "Arduino.h"
#include "DCMotor.h"

class Chime
{

public:
    Chime(uint8_t pinMotorTunePhase, uint8_t pinMotorTuneEnable, uint8_t pinMotorPickPhase, uint8_t pinMotorPickEnable, uint8_t pinMotorPickLimit, uint8_t pinMotorMutePhase, uint8_t pinMotorMuteEnable);
    void FrequencyToMotor(float detectedFrequency, float targetFrequency);
    void Tick();

private:
    DCMotor _tuneMotor;
    DCMotor _pickMotor;
    DCMotor _muteMotor;

    float _targetFrequency;

    uint8_t _pinMotorPickLimit;
};