



#include "arduino.h"

class Chime
{

public:
    // Constructor
    Chime(uint8_t coilPin, uint8_t pickPin, uint8_t stepperStepPin, uint8_t stepperDirectionPin);

    // Destructor.
    ~Chime(void);

    // Functions.
    void Pick(void);
    void Mute(void);
    void SetTargetFrequency(float targetFrequency);

    void GetState(void);

    enum State
    {
        Open,
        Vibrato,
        Mute
    };

private:
    // Variables.
   uint8_t _coilPin;
   uint8_t _pickPin;
   uint8_t _stepperStepPin;
   uint8_t _stepperDirectionPin;




};