#include "arduino.h"

class SimpleStepper
{

public:
    // Constructor   
    SimpleStepper(uint8_t stepperStepPin, uint8_t stepperDirectionPin);

    // Destructor.
    ~SimpleStepper(void);

    void SetSpeed(int speed);
    void SetCurrentPosition(signed long currentPosition);
    void SetTargetPosition(signed long targetPosition);
    void SetDirectionInversion(bool invertFlag);
    bool Run(void);
    

private:

    void SetDirection();

    uint8_t _stepperStepPin;
    uint8_t _stepperDirectionPin;

    const int stepPulseLengthMicros = 10;

    bool _direction;
    bool _invertDirection;
    signed int _positionChangePerStep;

    int _speed;
    signed long _currentPosition;
    signed long _targetPosition;

    unsigned long lastStepMicros;

};