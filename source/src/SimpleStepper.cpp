
#include "SimpleStepper.h"


SimpleStepper::SimpleStepper(uint8_t stepperStepPin, uint8_t stepperDirectionPin)
{
    _stepperStepPin = stepperStepPin;
    _stepperDirectionPin = stepperDirectionPin;

    pinMode(_stepperStepPin, OUTPUT);
    pinMode(_stepperDirectionPin, OUTPUT);

    digitalWrite(_stepperStepPin, LOW);
    digitalWrite(_stepperDirectionPin, HIGH);    
}

void SimpleStepper::SetSpeed(int speed)
{
    if (speed < 0)
    {
        speed = 0;
    }

    _speed = speed;
}

void SimpleStepper::SetCurrentPosition(signed long currentPosition)
{
    _currentPosition = currentPosition;
    SetDirection();
}

void SimpleStepper::SetTargetPosition(signed long targetPosition)
{
    _targetPosition = targetPosition;
    SetDirection();
}

void SimpleStepper::SetDirection()
{
    
    bool _direction = _currentPosition < _targetPosition ? 1 : 0;
    
    if (_invertDirection)
    {
      //  _direction = !_direction;
    } 

    _positionChangePerStep = _direction ?  1 : -1;

    digitalWrite(_stepperDirectionPin, _direction); 

}

void SimpleStepper::SetDirectionInversion(bool invertFlag)
{
    _invertDirection = invertFlag;
}


bool SimpleStepper::Run(void)
{

delayMicroseconds(1000);

    if (_currentPosition != _targetPosition)
    {
        digitalWrite(_stepperStepPin, HIGH);
        delayMicroseconds(stepPulseLengthMicros);
        digitalWrite(_stepperStepPin, LOW);
        _currentPosition += _positionChangePerStep;
        return true;
    }

        return false;     
}

SimpleStepper::~SimpleStepper(void)
{
    // Clean up.
}