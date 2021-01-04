//Mode: ENABLE_PHASE -> pin1 = Enable, pin2 = Phase.

#include "Arduino.h"

class DCMotor
{

public:
    enum DriverMode
    {
        IN1_IN2,
        ENABLE_PHASE
    };

    enum Direction
    {
        CW,
        CCW,
    };

    DCMotor(DriverMode driverMode, uint8_t pin1, uint8_t pin2);
    void MotorInversed(bool inverseFlag);
    void SetMotorRunTime(Direction direction, unsigned int runTime);
    void Tick();
    bool IsRunning();
    void MotorStop();

private:
    void MotorStart(); // If this goes public, needs flag and timeout prevention.

    DriverMode _driverMode;
    Direction _direction;

    uint8_t _pin1;
    uint8_t _pin2;
    uint8_t _pinEnable;
    uint8_t _pinPhase;

    bool _motorEnableFlag;

    bool _motorInversed = false;

    unsigned long _runTime;
    unsigned long _startTime;
};