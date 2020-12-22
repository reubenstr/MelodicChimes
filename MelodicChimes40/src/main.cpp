#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


// GUItool: begin automatically generated code
AudioInputAnalog         adc1;           //xy=470,269
AudioAnalyzeNoteFrequency notefreq1;      //xy=765,284
AudioConnection          patchCord1(adc1, notefreq1);
// GUItool: end automatically generated code

void setup() 
{
    AudioMemory(30);    
    notefreq1.begin(.15);
    pinMode(LED_BUILTIN, OUTPUT);

}

void loop() 
{   
    if (notefreq1.available()) {
        float note = notefreq1.read();
        float prob = notefreq1.probability();
        Serial.printf("Note: %3.2f | Probability: %.2f\n", note, prob);
    }
}