#include "Chime.h"

Chime::Chime(uint8_t coilPin, uint8_t pickPin, uint8_t stepperStepPin, uint8_t stepperDirectionPin)
{
    //this->_coilPin = coilPin;
    // this->_pickPin = pickPin;
    //this->_stepperStepPin = stepperStepPin;
    //this->_stepperDirectionPin = stepperDirectionPin;
}

// Mute string, change tuning, play note.
void Chime::PlayNote(uint8_t targetNoteNumber)
{
    _actionState = Note;
    _actionPhase = MuteStringPhase;
}

// Slide into targetNoteNumber from previous note.
void PlayGlissando(uint8_t targetNoteNumber, float noteVelocity)
{
    // TODO
}

// Slide into targetNoteNumber from specified starting note.
void PlayPortamento(uint8_t startNoteNumber, uint8_t targetNoteNumber, float noteVelocity)
{
    // TODO
}

// Process muting, picking, and tuning.
// Must be called often or lag will develop.
bool Chime::Update()
{

    ActionPhase previousActionPhase = _actionPhase;

    // Reset timers automatically.
    if (_actionPhase != previousActionPhase)
    {
        previousActionPhase = _actionPhase;
        ResetMuteTimer();
        ResetPickTimer();
    }

    // Process finite statemachine.
    if (_actionState == Mute)
    {
        MuteString(_pickSide);
    }
    else if (_actionState == Note)
    {
        if (_actionPhase == MuteStringPhase)
        {
            if (MuteString(_pickSide))
            {
                _actionPhase = MoveIntoPositionPhase;
            }
        }
        else if (_actionPhase == MoveIntoPositionPhase)
        {
            if (MoveIntoPosition(_targetNoteNumber))
            {
                InvertPickSide(_pickSide);
                _actionPhase = PickStringPhase;
            }
        }
        else if (_actionPhase == PickStringPhase)
        {
            if (PickString(_pickSide))
            {
                _actionPhase = TunePhase;
            }
        }
        else if (_actionPhase == TunePhase)
        {
            if (!Tune(_targetNoteNumber))
            {
                // set error!
            }
        }
    }
    else if (_actionState == Glissando)
    {
        // pick string
        // move stepper into known position at the specified velocity
        // tune frequency
    }
    else if (_actionState == Portamento)
    {
        // mute string
        // move stepper into known position (startNoteNumber)
        // pick string
        // move stepper into known position (targetNoteNumber) at the specified velocity
        // tune frequency
    }

    return true;
}

bool Chime::MuteString(PickSide pickSide)
{

    if (_pickSide == Left)
    {
        // move servo into mute position
    }
    else if (_pickSide == Right)
    {
        // move servo into mute position
    }

    if (millis() > (_muteTimer + 50))
    {
        return true;
    }

    return false;
}

bool Chime::PickString(PickSide &pickSide)
{

    if (pickSide == Left)
    {
        // move servo into mute position
    }
    else if (pickSide == Right)
    {
        // move servo into mute position
    }

    if (millis() > (_pickTimer + 50))
    {
        return true;
    }

    return false;
}

bool Chime::MoveIntoPosition(uint8_t noteNumber)
{
    // step into position of target note.

    // convert note into calibrated position

    //if (stepper.PositionToGo == 0)
    {
        return true;
    }

    return false;
}

bool Chime::Tune(uint8_t _targetNoteNumber)
{
    // process FFT

    // move stepper
    // PID

    // check if failure (not able to tune due to bad ADC (broke string))

    return true;
}

void Chime::InvertPickSide(PickSide &pickSide)
{
    if (pickSide == Left)
    {
        pickSide = Right;
    }
    else if (_pickSide == Right)
    {
        pickSide = Left;
    }
}

void Chime::ResetMuteTimer()
{
    _muteTimer = millis();
}

void Chime::ResetPickTimer()
{
    _pickTimer = millis();
}