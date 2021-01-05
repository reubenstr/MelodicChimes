
#include "QuickStepper.h"

QuickStepper::QuickStepper(uint8_t pinStep, uint8_t pinDirection)
{

    _pinStep = pinStep;
    _pinDirection = pinDirection;

    digitalWrite(_pinStep, LOW);
    digitalWrite(_pinDirection, LOW);

    pinMode(_pinStep, OUTPUT);
    pinMode(_pinDirection, OUTPUT);
}

void QuickStepper::SetCurrentPosition(signed long currentPosition)
{
    _currentPosition = currentPosition;
}

void QuickStepper::SetTargetPosition(signed long targetPosition)
{
    _targetPosition = targetPosition;
}

void QuickStepper::Stop()
{
    _currentPosition = _targetPosition;
}

int QuickStepper::TotalSteps()
{
    return _totalSteps;
}

void QuickStepper::Tick()
{
    static unsigned long start = micros();

    if (_currentPosition < _targetPosition)
    {
        if (micros() - start > delayBetweenSteps)
        {
            start = micros();

            _currentPosition++;
            _totalSteps++;

            digitalWrite(_pinDirection, HIGH);

            digitalWrite(_pinStep, HIGH);
            delayMicroseconds(5);
            digitalWrite(_pinStep, LOW);
        }
    }
    else if (_currentPosition > _targetPosition)
    {
        if (micros() - start > delayBetweenSteps)
        {
            start = micros();

            _currentPosition--;
            //_totalSteps++;

            digitalWrite(_pinDirection, LOW);

            digitalWrite(_pinStep, HIGH);
            delayMicroseconds(5);
            digitalWrite(_pinStep, LOW);
        }
    }
}