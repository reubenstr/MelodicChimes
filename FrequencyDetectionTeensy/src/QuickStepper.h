#include "Arduino.h"
#ifndef ADD_QS
#define ADD_QS

class QuickStepper
{

public:
    QuickStepper(uint8_t pinStep, uint8_t pinDirection);
    void SetCurrentPosition(signed long targetPosition);
    void SetTargetPosition(signed long targetPosition);
    void Stop();

    void Tick();

    int TotalSteps();

private:
    signed long _currentPosition;
    signed long _targetPosition;
    unsigned int _totalSteps = 0; // TEMP

    uint8_t _pinStep;
    uint8_t _pinDirection;

    unsigned int acceleration;

    const unsigned int delayBetweenSteps = 5000;
    unsigned long start = micros();

    bool inMotionFlag = false;

    /*    
    At full step:
    (1 / 0.004) / 200 * 60 = 60RPM
    (1 / 0.0033) / 200 * 60 = 90.9RPM
    (1 / 0.002) / 200 * 60 = 150RPM
    (1 / 0.0015) / 200 * 60 = 200RPM
    (1 / 0.001) / 200 * 60 = 300RPM
    (frequency) / pulses per revolution / 60 seconds per minute

    15 ms per freq detection / 2ms per step = 7.5 steps
    15 ms per freq detection / 1.5ms per step = 10 steps

    Seems best average case between notes is 22 steps.



    */
};

#endif