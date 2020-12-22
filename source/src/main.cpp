#include <Arduino.h>
#include "SPI.h"
#include "arduinoFFT.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
#include "Chime.h"

#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SerialFlash.h>

#include "TeensyStep.h"

#define PIN_COIL_1 A10
#define PIN_DRIVER_2_STEP 5
#define PIN_DRIVER_2_DIRECTION 4
#define SERVO_PICK_2 22


int const  numStrings = 4;

float stringFrequencyCurrent[4];
float stringFrequencyTarget[4];


Chime chime(PIN_COIL_1, SERVO_PICK_2, PIN_DRIVER_2_STEP, PIN_DRIVER_2_DIRECTION);


// GUItool: begin automatically generated code
AudioInputAnalog         adc1;           //xy=121,76
AudioAnalyzeNoteFrequency notefreq1;      //xy=336.00000762939453,73.00000286102295
AudioConnection          patchCord1(adc1, notefreq1);
// GUItool: end automatically generated code


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  Serial.println("Startup");                             
 
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(500);

  chime.PlayNote(0);

  AudioMemory(30);
  
  //notefreq1.begin(0.15);

  

 
}

void loop()
{

  chime.Update();
 


  static unsigned long oldMillis = 0;

  // read back fundamental frequency
  if (notefreq1.available())
  {
    float note = notefreq1.read();
    float prob = notefreq1.probability();
    Serial.printf("Note: %3.2f | Probability: %.2f\n", note, prob);
  }

 digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));

 

  
}
