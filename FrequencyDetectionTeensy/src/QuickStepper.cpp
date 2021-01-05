
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

    if (!inMotionFlag)
    {
        // TODO: check if opposite direction, then reset accleration
        acceleration = 0;
    }

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
    //static unsigned long start = micros();

    if (_currentPosition < _targetPosition)
    {
        if (micros() - start > delayBetweenSteps - acceleration)
        {
            start = micros();

            inMotionFlag = true;

            _currentPosition++;
            _totalSteps++;

            acceleration += 250; // TEMP ACC TEST
            if (acceleration > 3000)
            {
                acceleration = 3000;
            }

            digitalWrite(_pinDirection, HIGH);

            digitalWrite(_pinStep, HIGH);
            delayMicroseconds(5);
            digitalWrite(_pinStep, LOW);
        }
    }
    else if (_currentPosition > _targetPosition)
    {
        if (micros() - start > delayBetweenSteps - acceleration)
        {
            start = micros();

            _currentPosition--;
            //_totalSteps++;

            inMotionFlag = true;

            digitalWrite(_pinDirection, LOW);

            digitalWrite(_pinStep, HIGH);
            delayMicroseconds(5);
            digitalWrite(_pinStep, LOW);
        }
    }
    else
    {
        inMotionFlag = false;
    }
}