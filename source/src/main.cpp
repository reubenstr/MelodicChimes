#include <Arduino.h>
#include "SPI.h"
#include "arduinoFFT.h"
 #include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

#define PIN_COIL_1 A10


#define _cs 10
#define _dc 9
#define _rst 8


#define SAMPLES 128             //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno.
#define SAMPLING_FREQUENCY 1024 //Ts = Based on Nyquist, must be 2 times the highest expected frequency.
 
arduinoFFT FFT = arduinoFFT();
 
unsigned int samplingPeriod;
unsigned long microSeconds;
 
double vReal[SAMPLES]; //create vector of size SAMPLES to hold real values
double vImag[SAMPLES]; //create vector of size SAMPLES to hold imaginary values



Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);

int const numStrings = 4;

float stringFrequencyCurrent [4];
float stringFrequencyTarget [4];
 
void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200); //Baud rate for the Serial Monitor
    samplingPeriod = round(1000000*(1.0/SAMPLING_FREQUENCY)); //Period in microseconds 

digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
tft.begin();

    tft.setRotation(3);

 tft.fillScreen(ILI9340_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9340_WHITE, ILI9340_BLACK);  
  tft.setTextSize(3);
  tft.println("Hello World!");

}
 
void loop() 
{  

    for(int i=0; i<SAMPLES; i++)
    {
        microSeconds = micros();    //Returns the number of microseconds since the Arduino board began running the current script. 
     
        vReal[i] = analogRead(PIN_COIL_1); 
        vImag[i] = 0;

        /*remaining wait time between samples if necessary*/
        while(micros() < (microSeconds + samplingPeriod))
        {
          //do nothing
        }
    }
 
    // Perform FFT on samples, attain dominant frequency.
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    stringFrequencyCurrent[0] = (float)FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
    //Serial.println(peak);   
 
 tft.setCursor(0, 50);

for(int i = 0; i < numStrings; i++)
{
  if (stringFrequencyCurrent[i] == stringFrequencyTarget[i])
  {
    tft.setTextColor(ILI9340_WHITE, ILI9340_GREEN); 
  }
  else
  {
    tft.setTextColor(ILI9340_WHITE, ILI9340_RED); 
  }
  
  tft.println((String) stringFrequencyCurrent[i] + " " + stringFrequencyTarget[i]);

}
 

    //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(200);                       // wait for a second
  //digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
 // delay(100);    
}


