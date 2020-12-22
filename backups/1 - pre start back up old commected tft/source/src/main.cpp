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

//#include "TeensyStep.h"

#define PIN_COIL_1 A10
#define PIN_DRIVER_2_STEP 5
#define PIN_DRIVER_2_DIRECTION 4
#define SERVO_PICK_2 22

#define _cs 10
#define _dc 9
#define _rst 8

#define SAMPLES 64              //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno.
#define SAMPLING_FREQUENCY 1024 //Ts = Based on Nyquist, must be 2 times the highest expected frequency.

arduinoFFT FFT = arduinoFFT();

unsigned int samplingPeriod;
unsigned long microSeconds;

double vReal[SAMPLES]; //create vector of size SAMPLES to hold real values
double vImag[SAMPLES]; //create vector of size SAMPLES to hold imaginary values

Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);

int const numStrings = 4;

float stringFrequencyCurrent[4];
float stringFrequencyTarget[4];

Chime chime(PIN_COIL_1, SERVO_PICK_2, PIN_DRIVER_2_STEP, PIN_DRIVER_2_DIRECTION);

// GUItool: begin automatically generated code
AudioInputAnalogStereo adcs1(A10, A3); //xy=114,69
AudioAnalyzeNoteFrequency notefreq1;   //xy=336.00000762939453,73.00000286102295
AudioAnalyzeNoteFrequency notefreq2;   //xy=428,326
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);

// GUItool: end automatically generated code

//Stepper motor_1(PIN_DRIVER_2_STEP, PIN_DRIVER_2_DIRECTION);
//StepControl controller;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Startup");                                    //Baud rate for the Serial Monitor;
  samplingPeriod = round(1000000 * (1.0 / SAMPLING_FREQUENCY)); //Period in microseconds

  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  tft.begin();

  tft.setRotation(3);

  tft.fillScreen(ILI9340_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tft.setTextSize(3);

  ///////////////
  tft.println(7);
  ///////////////

  chime.PlayNote(0);

  AudioMemory(90);
  notefreq1.begin(0.15);

  tft.println("pass");

  // setup the motors
  // motor_1
  //  .setMaxSpeed(500)       // steps/s
  //    .setAcceleration(20000); // steps/s^2
}

void loop()
{

  /*
 static int count = 0;
  tft.println(count++);

  constexpr int spr = 16*200;  // 3200 steps per revolution   
  for(int i = 0; i < 5; i++)
  {
    motor_1.setTargetRel(spr/4); // 1/4 revolution
    //controller.move(motor_1);  

    motor_1.setTargetRel(-spr/4);
    //controller.move(motor_1);  
  }
  */

 


  static unsigned long oldMillis = 0;

  // read back fundamental frequency
  if (notefreq1.available())
  {
    float note = notefreq1.read();
    float prob = notefreq1.probability();
    Serial.printf("Note: %3.2f | Probability: %.2f\n", note, prob);

/*
    if (note < 1000)
    {
      int i = millis() - oldMillis;
      tft.setCursor(0, 50);
      tft.printf("%3.2f\n", note);
      tft.printf("%.2f\n", prob);
      tft.printf("     %i      \n", i);

      Serial.printf("Note: %3.2f | Probability: %.2f | %i\n", note, prob, i);
      oldMillis = millis();
    }
    */
  }

  //chime.Update();

  //digitalWrite(LED_BUILTIN, HIGH);

  /*

  float returnVal = chime.SampleCoilForFrequency(PIN_COIL_1);
  if (returnVal !=0)
  {
    //stringFrequencyCurrent[0] = chime.SampleCoilForFrequency(PIN_COIL_1);
    stringFrequencyCurrent[0] = returnVal;   
  }
  */

  //digitalWrite(LED_BUILTIN, LOW);

  /**/

  if (millis() > (oldMillis + 500))
  {
    //oldMillis = millis();

    //tft.setCursor(0, 50);
    //tft.println(stringFrequencyCurrent[0]);

    /*
    for (int i = 0; i < numStrings; i++)
    {
      if (stringFrequencyCurrent[i] == stringFrequencyTarget[i])
      {
        tft.setTextColor(ILI9340_WHITE, ILI9340_GREEN);
      }
      else
      {
        tft.setTextColor(ILI9340_WHITE, ILI9340_RED);
      }
    
      tft.println((String) i + ": " + stringFrequencyCurrent[i] + " " + stringFrequencyTarget[i]);   
     }
      */
  }
}
