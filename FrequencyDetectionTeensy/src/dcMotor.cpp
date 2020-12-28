// DC Motor Controller for H-bridge drivers such as the Pololu DRV8835.
//
// Expects the driver to use a phase and enable input mode.

#include "Arduino.h"
#include "DCMotor.h"

DCMotor::DCMotor(uint8_t pinPhase, uint8_t pinEnable)
{
    _pinPhase = pinPhase;
    _pinEnable = pinEnable;

    digitalWrite(_pinPhase, LOW);
    digitalWrite(_pinEnable, LOW);

    pinMode(_pinPhase, OUTPUT);
    pinMode(_pinEnable, OUTPUT);
}

void DCMotor::MotorInversed(bool inverseFlag)
{
    _inverseFlag = inverseFlag;
}

void DCMotor::SetMotorDirection(Direction direction)
{
    if (direction == Direction::Up)
    {
        if (_inverseFlag)
            digitalWrite(_pinPhase, false);
        else
            digitalWrite(_pinPhase, true);
    }

    if (direction == Direction::Down)
    {
        if (_inverseFlag)
            digitalWrite(_pinPhase, true);
        else
            digitalWrite(_pinPhase, false);
    }
}

void DCMotor::SetMotorRunTime(Direction direction, unsigned int runTime)
{
    _runTime = runTime;
    _startTime = millis();    
    _motorEnableFlag = true;
    SetMotorDirection(direction);
}

void DCMotor::Tick()
{
    if (millis() - _startTime > _runTime)
    {
        _motorEnableFlag = false;
    }

    if (_motorEnableFlag)
    {
        MotorStart();
    }
    else
    {
        MotorStop();
    }
}

void DCMotor::MotorStart()
{    
    digitalWrite(_pinEnable, HIGH);
}

void DCMotor::MotorStop()
{
     _motorEnableFlag = false;
    digitalWrite(_pinEnable, LOW);
}

bool DCMotor::IsRunning()
{
    return   _motorEnableFlag;
}