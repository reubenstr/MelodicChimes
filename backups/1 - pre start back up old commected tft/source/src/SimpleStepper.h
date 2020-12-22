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
    bool Run(void);
    

private:

    uint8_t _stepperStepPin;
    uint8_t _stepperDirectionPin;

    const int stepPulseLengthMicros = 10;

    int _speed;
    signed long _currentPosition;
    signed long _targetPosition;

    unsigned long lastStepMicros;

};