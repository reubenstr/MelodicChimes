
#include"Arduino.h"
#include "DCMotor.h"


class Chime
{

    public:

    Chime(uint8_t pinPhase, uint8_t pinEnable);
    void FrequencyToMotor(float detectedFrequency, float targetFrequency);
    void Tick();


    private:
    DCMotor _dcMotor;

    float _targetFrequency;

};