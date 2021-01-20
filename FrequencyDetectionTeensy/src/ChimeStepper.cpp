
#include "Arduino.h"
#include "ChimeStepper.h"

ChimeStepper::ChimeStepper()
{
}

ChimeStepper::ChimeStepper(uint8_t pinStep, uint8_t pinDirection)
{

    _pinStep = pinStep;
    _pinDirection = pinDirection;

    digitalWrite(_pinStep, LOW);
    digitalWrite(_pinDirection, LOW);

    pinMode(_pinStep, OUTPUT);
    pinMode(_pinDirection, OUTPUT);

    _tuneTimer.begin(&ChimeStepper::TimerTick, 1000);
}

void ChimeStepper::SetCurrentPosition(signed long currentPosition)
{
    _currentPosition = currentPosition;
}

void ChimeStepper::SetTargetPosition(signed long targetPosition)
{

    if (!inMotionFlag)
    {
        // TODO: check if opposite direction, then reset accleration
        acclerationAcc = 0;
    }

    _targetPosition = targetPosition;
}

bool ChimeStepper::IsAtPosition()
{
    return _currentPosition == _targetPosition;
}

void ChimeStepper::SetSpeed(unsigned int speed)
{
    pullInOutSpeed = speed;
}

void ChimeStepper::Stop()
{
    _currentPosition = _targetPosition;
    inMotionFlag = false;
}

int ChimeStepper::TotalSteps()
{
    return _totalSteps;
}

void ChimeStepper::Tick()
{
    unsigned int pulsesPerSecond = pullInOutSpeed + acclerationAcc;
    if (pulsesPerSecond > maxSpeed)
        pulsesPerSecond = maxSpeed;

    unsigned int delayBetweenSteps = ConvertPulsesToMicroseconds(pulsesPerSecond);

    int stepsRemaining = abs(_targetPosition - _currentPosition);
    int stepsNeededToDeAcclerate = (pulsesPerSecond - pullInOutSpeed) / acclerationRate;

    if (_currentPosition < _targetPosition)
    {
        if (micros() - start > delayBetweenSteps)
        {
            Serial.printf("UP: %u -> %u | PPS: %u | ACC: %u | Del: %u\n", _currentPosition, _targetPosition, pulsesPerSecond, acclerationAcc, delayBetweenSteps);

            start = micros();

            if (stepsRemaining > stepsNeededToDeAcclerate)
            {

                acclerationAcc += acclerationRate;
                if (acclerationAcc > maxAcclerationAcc)
                {
                    acclerationAcc = maxAcclerationAcc;
                }
            }
            else
            {
                //acclerationAcc -= acclerationRate;
                if (acclerationAcc < 0)
                {
                    acclerationAcc = 0;
                }
            }

            inMotionFlag = true;
            direction = true;
            _currentPosition++;
            _totalSteps++;

            digitalWrite(_pinDirection, HIGH);
            digitalWrite(_pinStep, HIGH);
            delayMicroseconds(pulseLengthUs);
            digitalWrite(_pinStep, LOW);
        }
    }
    else if (_currentPosition > _targetPosition)
    {
        if (micros() - start > delayBetweenSteps - acceleration)
        {
            start = micros();

            Serial.printf("DOWN: %i -> %i | PPS: %i | ACC: %i | Del: %i\n", _currentPosition, _targetPosition, pulsesPerSecond, acclerationAcc, delayBetweenSteps);

            if (stepsRemaining > stepsNeededToDeAcclerate)
            {
                acclerationAcc += acclerationRate;
                if (acclerationAcc > maxAcclerationAcc)
                {
                    acclerationAcc = maxAcclerationAcc;
                }
            }
            else
            {
                //acclerationAcc -= acclerationRate;
                if (acclerationAcc < 0)
                {
                    acclerationAcc = 0;
                }
            }

            _currentPosition--;
            inMotionFlag = true;
            direction = false;

            digitalWrite(_pinDirection, LOW);
            digitalWrite(_pinStep, HIGH);
            delayMicroseconds(pulseLengthUs);
            digitalWrite(_pinStep, LOW);
        }
    }
    else
    {
        inMotionFlag = false;
    }
}

unsigned long ChimeStepper::ConvertPulsesToMicroseconds(unsigned int pulses)
{
    return (float)(1 / (float)pulses) * 1000.0 * 1000.0;
}

void ChimeStepper::TimerTick()
{
}
