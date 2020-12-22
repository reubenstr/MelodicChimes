
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
}

void SimpleStepper::SetTargetPosition(signed long targetPosition)
{
    _targetPosition = targetPosition;
}


bool SimpleStepper::Run(void)
{
    digitalWrite(_stepperStepPin, HIGH);
    delayMicroseconds(stepPulseLengthMicros);
    digitalWrite(_stepperStepPin, LOW);

    //delay(1);
    delayMicroseconds(250);

    return true;
}

SimpleStepper::~SimpleStepper(void)
{
    // Clean up.
}