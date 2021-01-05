// DC Motor Controller for H-bridge drivers
//
// Supports drivers with pin schemas:
//          PWM1/PWM2 
//          ENABLE/PHASE 

#include "Arduino.h"
#include "DCMotor.h"

DCMotor::DCMotor(DriverMode driverMode, uint8_t pin1, uint8_t pin2)
{
    _driverMode = driverMode;
    _pin1 = pin1;
    _pin2 = pin2;
    _pinEnable = pin1;
    _pinPhase = pin2;

    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);

    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
}

void DCMotor::MotorInversed(bool motorInversed)
{
    _motorInversed = motorInversed;
}

void DCMotor::SetMotorRunTime(Direction direction, unsigned int runTime)
{
    _runTime = runTime;
    _startTime = millis();
    _motorEnableFlag = true;
    _direction = direction;
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
    if (_driverMode == DriverMode::IN1_IN2)
    {
        if (_direction == Direction::CW)
        {
            if (_motorInversed)
            {
                
                digitalWrite(_pin1, LOW);
                digitalWrite(_pin2, HIGH);
            }
            else
            {
                digitalWrite(_pin1, HIGH);
                digitalWrite(_pin2, LOW);
            }
        }
        else if (_direction == Direction::CCW)
        {
            if (_motorInversed)
            {
                digitalWrite(_pin1, HIGH);
                digitalWrite(_pin2, LOW);
            }
            else
            {
                digitalWrite(_pin1, LOW);
                digitalWrite(_pin2, HIGH);
            }
        }
    }
    else if (_driverMode == DriverMode::ENABLE_PHASE)
    {
        if (_direction == Direction::CW)
        {
            digitalWrite(_pinEnable, HIGH);
            if (_motorInversed)
            {
                digitalWrite(_pinPhase, LOW);
            }
            else
            {
                digitalWrite(_pinPhase, HIGH);
            }
        }
        else if (_direction == Direction::CCW)
        {
            digitalWrite(_pinEnable, HIGH);
            if (_motorInversed)
            {
                digitalWrite(_pinPhase, HIGH);
            }
            else
            {
                digitalWrite(_pinPhase, LOW);
            }
        }
    }
}

void DCMotor::MotorStop()
{
    _motorEnableFlag = false;
    if (_driverMode == DriverMode::IN1_IN2)
    {
        digitalWrite(_pin1, HIGH);
        digitalWrite(_pin2, HIGH);
    }
    else if (_driverMode == DriverMode::ENABLE_PHASE)
    {
        digitalWrite(_pinEnable, LOW);
    }
}

bool DCMotor::IsRunning()
{
    return _motorEnableFlag;
}