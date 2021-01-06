/*
 FFT TEST

 Reported frequency is 20hz higher than likely actual frequency
*/

#include "arduinoFFT.h"

#define SAMPLES 64             //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno.
#define SAMPLING_FREQUENCY 1024 //Ts = Based on Nyquist, must be 2 times the highest expected frequency.

arduinoFFT FFT = arduinoFFT();

unsigned int samplingPeriod;
unsigned long microSeconds;

double vReal[SAMPLES]; //create vector of size SAMPLES to hold real values
double vImag[SAMPLES]; //create vector of size SAMPLES to hold imaginary values

void setup()
{
  Serial.begin(115200); //Baud rate for the Serial Monitor
  samplingPeriod = round(1000000 * (1.0 / SAMPLING_FREQUENCY)); //Period in microseconds
}

void loop()
{
  static double acc;
  static int count;

  for (int i = 0; i < SAMPLES; i++)
  {
    microSeconds = micros();

    vReal[i] = analogRead(0);
    vImag[i] = 0;

    while (micros() < (microSeconds + samplingPeriod))
    {
      //do nothing
    }
  }

  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  // Serial.println(peak);
  acc += peak;

  static unsigned long startMillis = millis();
  count++;
  if (count > 9)
  {
    Serial.print("Ms: ");
    Serial.print(millis() - startMillis);
    Serial.print(" | Acc: ");
    Serial.println(acc / (count));
    startMillis = millis();
    count = 0;
    acc = 0;
  }
}
