 /*  
    // Attempt a melody.
    static unsigned long startTarget = millis();
    static int noteSelect = 0;
    static unsigned int delay = 0;  

    static State state = sPick;

    static bool preTuneCompletedFlag;

    if (noteSelect < numNotes)
    {

        if (state == State::sPick)
        {
            if (millis() - startTarget >= delay)
            {
                startTarget = millis();

                Pick();
                delay = delays[noteSelect] / 2;
                targetFrequency = notes[noteSelect];
                Serial.printf("Target frequency: %3.2f | Note Index: %u\n", targetFrequency, noteSelect);
                noteSelect++;
                state = State::sMute;
            }
        }
        else if (state == State::sMute)
        {
            if (millis() - startTarget >= delay)
            {
                startTarget = millis();
                Mute();
                preTuneCompletedFlag = false;
                state = State::sPretune;
            }
        }
        else if (state == State::sPretune)
        {

            static unsigned long pretuneMillis = 0;
            static unsigned int preTuneTime = 0;

            if (millis() - startTarget >= delay)
            {
                startTarget = millis();
                state = State::sPick;
            }

            if (preTuneCompletedFlag == false)
            {
                preTuneCompletedFlag = true;
                pretuneMillis = millis();
                float deltaFreq = notes[noteSelect] - notes[noteSelect - 1];
                preTuneTime = abs(deltaFreq * 10);

                if (deltaFreq > 0)
                    SetMotorDirection(Direction::Up);
                if (deltaFreq <= 0)
                    SetMotorDirection(Direction::Down);

                Serial.printf("\t\t\tFreq Delta: %3.2f | Delay (ms): %u\n", deltaFreq, preTuneTime);
                char motState[12];
                sprintf(motState, "%s", MotorState() ? "On" : "Off");
                char dirStr[12];
                sprintf(dirStr, "%s", MotorState() == 0 ? "" : GetMotorDirection() ? "(Up)" : "(Down)");
                Serial.printf("\t\t\tNext Target %3.2f | Current: %3.2f | Motor state %s %s \n", notes[noteSelect], notes[noteSelect - 1], motState, dirStr);
        
                EnableMotor(true);
            }

            if (millis() - pretuneMillis > preTuneTime)
            {
                EnableMotor(false);
            }
        }
    }
    */

   /*
////////////////////////////////////////////////////////////////////////
void Pick(bool homeFlag = false)
{

    if (homeFlag)
    {
        pickSide = Left;
        servo3.write(100);
        return;
    }

    pickedFlag = true;

    if (pickSide == Left)
    {
        pickSide = Right;
        servo3.write(60);
    }
    else if (pickSide == Right)
    {
        pickSide = Left;
        servo3.write(100);
    }
}

void Mute()
{
    if (pickSide == Left)
    {
        servo3.write(80);
    }
    else if (pickSide == Right)
    {
        servo3.write(80);
    }
}
////////////////////////////////////////////////////////////////////////
   */


  /*   
    // Manual test plectum motor.
    pinMode(PIN_MOTOR_PICK_PHASE, OUTPUT);
    pinMode(PIN_MOTOR_PICK_ENABLE, OUTPUT);

    digitalWrite(PIN_MOTOR_PICK_ENABLE, HIGH);
    digitalWrite(PIN_MOTOR_PICK_PHASE, HIGH);

    while (1)
    {
    } 
    */

    /*  
    // Manual test mute motor.
    pinMode(PIN_MOTOR_MUTE_PHASE, OUTPUT);
    pinMode(PIN_MOTOR_MUTE_ENABLE, OUTPUT);

    digitalWrite(PIN_MOTOR_MUTE_ENABLE, HIGH);
        digitalWrite(PIN_MOTOR_MUTE_PHASE, HIGH);
        delay(200);

    while (1)
    {
        digitalWrite(PIN_MOTOR_MUTE_ENABLE, HIGH);
        digitalWrite(PIN_MOTOR_MUTE_PHASE, LOW);
        delay(300);
        digitalWrite(PIN_MOTOR_MUTE_ENABLE, LOW);
        delay(1000);
        digitalWrite(PIN_MOTOR_MUTE_ENABLE, HIGH);
        digitalWrite(PIN_MOTOR_MUTE_PHASE, HIGH);
        delay(300);
        digitalWrite(PIN_MOTOR_MUTE_ENABLE, LOW);
        delay(1000);
    }
    */