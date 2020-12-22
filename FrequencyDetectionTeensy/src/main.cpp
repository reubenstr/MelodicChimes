#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

//AudioInputAnalog adc1(A0);
//AudioAnalyzeNoteFrequency notefreq1;
//AudioConnection patchCord1(adc1, notefreq1);


// defined AUDIO_GUITARTUNER_BLOCKS value of 24 changed to 6 for
// faster analysis at the sacrafice of less lower frequency detection
// which is not required for this application.

AudioInputAnalogStereo adcs1;
AudioAnalyzeNoteFrequency notefreq2;
AudioAnalyzeNoteFrequency notefreq1;
AudioConnection patchCord1(adcs1, 0, notefreq1, 0);
AudioConnection patchCord2(adcs1, 1, notefreq2, 0);
// GUItool: end automatically generated code

void setup()
{
  delay(1000); // Give platformio time to switch back to the terminal.

  Serial.begin(115200);
  Serial.println("Teensy starting up.");

  AudioMemory(120); // 30
  notefreq1.begin(.10); //0.15
  notefreq2.begin(.10);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{

  // Flash onboard LED to show activity
  static unsigned long start = millis();
  if (millis() - start > 50)
  {
    start = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  /* */
  static int timeElapsed1 = millis();
  if (notefreq1.available())
  {
    float note = notefreq1.read();
    float prob = notefreq1.probability();
    Serial.printf("Note 1 (%U): %3.2fhz | Probability: %.2f\n",  millis() - timeElapsed1, note, prob);
    timeElapsed1 = millis();
  }

  static int timeElapsed2 = millis();
  if (notefreq2.available())
  {
    float note2 = notefreq2.read();
    float prob2 = notefreq2.probability();
    Serial.printf("Note 2 (%U): %3.2fhz | Probability: %.2f\n",  millis() - timeElapsed2, note2, prob2);
    timeElapsed2 = millis();
  }
}