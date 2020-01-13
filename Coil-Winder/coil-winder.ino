// Quick sketch to activate stepper for coil winding.

#include <AccelStepper.h>

AccelStepper stepper(AccelStepper::DRIVER, 5, 6);

void setup()
{  
	pinMode(8, INPUT);           // set pin to input
	digitalWrite(8, HIGH);       // turn on pullup resistors

   stepper.setMaxSpeed(1600);	
   stepper.setSpeed(1600);	
}

void loop()
{  
	if (digitalRead(8) == 0)
	{
		stepper.runSpeed();
	}   
}
