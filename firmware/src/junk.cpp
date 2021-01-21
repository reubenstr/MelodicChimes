 

// n steps between average freq readings to create a linear regression.
/*
void CalibrateSteps()
{


    200 steps/rev, 1/2 step, 2:1 pulley ratio 
    200 * 2 * 0.5

  

    Serial.println("Calibrate Steps between notes...");

    AccelStepper stepper(AccelStepper::DRIVER, PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION);
    stepper.setPinsInverted(true, false, false);
    stepper.setMaxSpeed(50000);
    stepper.setAcceleration(10000);

    // Tune starting frequency.
    Serial.println("Tune starting frequency.");
    float targetFrequency = C;
    float detectedFrequency = 0;
    float frequencyTolerance = 1;
    unsigned long start = millis() + 1000;

    while (1)
    {
        if (millis() - start > 2000)
        {
            start = millis();
            chime.Pick();
        }

        detectedFrequency = GetFrequency();

        if ((detectedFrequency > (targetFrequency - frequencyTolerance)) &&
            (detectedFrequency < (targetFrequency + frequencyTolerance)))
        {
            break;
        }

        chime.TuneFrequency(detectedFrequency, targetFrequency);
        chime.Tick();
    }

    // Progress through the sequence.
    Serial.println("Progress through the sequence.");
    const int numReadings = 30;
    const int numStepsBetweenMeasurements = 20;

    movingAvg avg(30);
    avg.begin();
    int freqCount = 0;
    bool nextReadingFlag = true;

    while (detectedFrequency < 450)
    {      
        if (nextReadingFlag)
        {
            nextReadingFlag = false;
            freqCount = 0;

            stepper.setCurrentPosition(0);
            stepper.moveTo(numStepsBetweenMeasurements);
            stepper.runToPosition();

            chime.Pick();
            delay(250); // give time for the string to settle
        }

        detectedFrequency = GetFrequency();

        if (detectedFrequency > 0)
        {
            avg.reading(detectedFrequency);

            if (freqCount++ > numReadings)
            {
                nextReadingFlag = true;
                Serial.printf("%3.2f\n", avg.getAvg());
                delay(500);
            }
        }

        chime.Tick();
    }
}

*/  


 
    /*
    // Manual motor test: tune motor
    analogWriteFrequency(PIN_MOTOR_TUNE_ENABLE, 500);
    //analogWriteFrequency(PIN_MOTOR_TUNE_PWM1, 1000);
    //analogWriteFrequency(PIN_MOTOR_TUNE_PWM2, 1000);
    pinMode(PIN_MOTOR_TUNE_PWM1, OUTPUT);
     pinMode(PIN_MOTOR_TUNE_PWM2, OUTPUT);

    // TESTING 3 pin configuration: EN (PWM) / IN(1 or 0) / IN(1  or 0)
    while (1)
    {

        analogWrite(PIN_MOTOR_TUNE_ENABLE, 80);
        digitalWrite(PIN_MOTOR_TUNE_PWM1, HIGH);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, LOW);
        DebugLEDs();
        delay(1000);

        analogWrite(PIN_MOTOR_TUNE_ENABLE, 200);
        digitalWrite(PIN_MOTOR_TUNE_PWM1, LOW);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, HIGH);
        DebugLEDs();
        delay(1000);
    }
    */

    /*
    while (1)
    {
   
        digitalWrite(PIN_MOTOR_TUNE_PWM1, HIGH);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, LOW);
        //digitalWrite(PIN_MOTOR_TUNE_PWM1, LOW);
        //digitalWrite(PIN_MOTOR_TUNE_PWM2, HIGH);

        DebugLEDs();
        delay(5);

        digitalWrite(PIN_MOTOR_TUNE_PWM1, HIGH);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, HIGH);
        DebugLEDs();
        delay(500);

        digitalWrite(PIN_MOTOR_TUNE_PWM1, LOW);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, HIGH);
        DebugLEDs();
        delay(50);

        digitalWrite(PIN_MOTOR_TUNE_PWM1, LOW);
        digitalWrite(PIN_MOTOR_TUNE_PWM2, LOW);
        DebugLEDs();
        delay(500);
        
    }
    */
//////////////////////////////////////////////////////
    /*   
    while (1)
    {
        CalibrateTiming();
    }*/

    //////////////////////////////////////////////////////
    // STEPS TO FREQ TEST
    /*
90° - 221hz to 280hz = delta 59hz
280 334 = 54
332 373 = 41
371 401 = 30
starting to skips steps at 1/2 step 1000us step delay

// 1/2 step with 100 steps
262 - 309 = 47 hz -> 0.94 steps per hz
309 - 365 = 56 hz -> 1.12 steps per hz
364 - 315 = 49 hz -> 0.98 steps per hz
314 - 353 = 39 hz -> 0.78 steps per hz

About one hz per full step.

*/

    /* 
    QuickStepper stepper(PIN_STEPPER_TUNE_STEP, PIN_STEPPER_TUNE_DIRECTION);

    stepper.SetCurrentPosition(0);
    stepper.SetTargetPosition(0);
    int count = 0;

    while (1)
    {
        float detectedFrequency = GetFrequency();
        if (detectedFrequency != 0)
        {
            Serial.printf("Frequency: %3.2f\n", detectedFrequency);

            if (count++ == 10)
            {
                stepper.SetTargetPosition(100);
                Serial.println("**** Stepper start");
            }
        }
        // Prevent over tension.
        if (detectedFrequency > 475)
        {
            stepper.Stop();
        }
        stepper.Tick();
    }
   */

    //////////////////////////////////////////////////////
    /* 
    // SPEED TEST between two frequencies
    bool flag = true;
    while (flag)
    {
        float detectedFrequency = GetFrequency();
        static float targetFrequency = 300;

        static unsigned long start = millis();
        static bool toggle = false;
        if (millis() - start > 4000)
        {
            start = millis();
            toggle = !toggle;

            if (toggle)
            {
                targetFrequency = 340;
            }
            else
            {
                targetFrequency = 440;
            }
        }

        chime.TuneFrequency(detectedFrequency, targetFrequency);
        chime.Tick();
    }
    */

    /////////////////////////////
    // STEPPER TEST
    /*
    while (1)
    {

        static unsigned long start = millis();
        static bool toggle = false;
        if (millis() - start > 2000)
        {
            start = millis();
            toggle = !toggle;

            stepper.SetCurrentPosition(0);

            if (toggle)
            {
                Serial.println("- position");
                stepper.SetTargetPosition(-81);
            }
            else
            {
                Serial.println("+ position");
                stepper.SetTargetPosition(81);
            }
        }

        digitalWrite(PIN_LED_1, digitalRead(PIN_STEPPER_TUNE_STEP));
        digitalWrite(PIN_LED_2, digitalRead(PIN_STEPPER_TUNE_DIRECTION));

        stepper.Tick();
    }
*/
    /////////////////////////////
 
 
 
 
 
 
 
 
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