
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
    bool IsRunning();    
    void MotorStop();   

private:

     void MotorStart(); // If this goes public, needs flag and timeout prevention.

    uint8_t _pinPhase;
    uint8_t _pinEnable;

    bool _motorEnableFlag;

    bool _inverseFlag = false;
    unsigned long _runTime;
    unsigned long _startTime;
};