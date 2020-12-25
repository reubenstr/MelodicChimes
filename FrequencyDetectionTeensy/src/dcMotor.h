
#include "Arduino.h"

class DCMotor
{

public:
    enum Direction
    {
        Up,
        Down
    };

    DCMotor(uint8_t pinPhase, uint8_t pinEnable);
    void SetMotorDirection(Direction direction);
    void MotorInversed(bool inverseFlag);
    void SetMotorRunTime(Direction direction, unsigned int runTime);
    void Tick();
    void MotorStart();
    void MotorStop();

private:
    uint8_t _pinPhase;
    uint8_t _pinEnable;

    bool _motorEnableFlag;

    bool _inverseFlag = false;
    unsigned long _runTime;
    unsigned long _startTime;
};