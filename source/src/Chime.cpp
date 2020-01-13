
#include "Chime.h"

Chime::Chime(uint8_t coilPin, uint8_t pickPin, uint8_t stepperStepPin, uint8_t stepperDirectionPin)
{

    this->_coilPin = coilPin;
    this->_pickPin = pickPin;
    this->_stepperStepPin = stepperStepPin;
    this->_stepperDirectionPin = stepperDirectionPin;

}
