// Using https://rextester.com/ to quickly compile and display output.
//g++  7.4.0

#include <iostream>
#include <math.h>
#include <iomanip> 

float NoteIdToFrequency(float noteId)
{
    return 440 * pow(2, (noteId - 69) / 12);
}

int main()
{     
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    
    for (int i = 58; i < 73; i++)
    {
        float uw = 0.00002215; // lbs/inch
        float l = 13.7; // inches
        float f = NoteIdToFrequency(i); // hertz
        float t = (uw * pow(2 * l *f, 2)) / 386.4; // lbs
        
        std::cout << "Note ID: " << i << ", Frequency: " << f << ", Tension: "  << t << " lbs.\n";
    }  
}

/*
Example outout using daddario branded string model #PL010 (0.010" diameter).

Note ID: 58, Frequency: 233.08, Tension: 2.34 lbs.
Note ID: 59, Frequency: 246.94, Tension: 2.62 lbs.
Note ID: 60, Frequency: 261.63, Tension: 2.95 lbs.
Note ID: 61, Frequency: 277.18, Tension: 3.31 lbs.
Note ID: 62, Frequency: 293.66, Tension: 3.71 lbs.
Note ID: 63, Frequency: 311.13, Tension: 4.17 lbs.
Note ID: 64, Frequency: 329.63, Tension: 4.68 lbs.
Note ID: 65, Frequency: 349.23, Tension: 5.25 lbs.
Note ID: 66, Frequency: 369.99, Tension: 5.89 lbs.
Note ID: 67, Frequency: 392.00, Tension: 6.61 lbs.
Note ID: 68, Frequency: 415.30, Tension: 7.42 lbs.
Note ID: 69, Frequency: 440.00, Tension: 8.33 lbs.
Note ID: 70, Frequency: 466.16, Tension: 9.35 lbs.
Note ID: 71, Frequency: 493.88, Tension: 10.50 lbs.
Note ID: 72, Frequency: 523.25, Tension: 11.78 lbs.

*/