/*
  neoPixelMethods.h

  Helper methods for the neopixel library.

*/

#include <Adafruit_NeoPixel.h>

// Matrix colors (NeoPixels)
const int NeoOff = 0x00000000;
const int NeoRed = 0x00FF0000;
const int NeoGreen = 0x0000FF00;
const int NeoBlue = 0x000000FF;

// Pack color data into 32 bit unsigned int (copied from Neopixel library).
uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (uint32_t)((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Input a value 0 to 255 to get a color value (of a pseudo-rainbow).
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Fade color by specified amount.
uint32_t Fade(uint32_t color, int amt)
{
	signed int r, g, b;

	r = (color & 0x00FF0000) >> 16;
	g = (color & 0x0000FF00) >> 8;
	b = color & 0x000000FF;

	r -= amt;
	g -= amt;
	b -= amt;

	if (r < 0)
		r = 0;
	if (g < 0)
		g = 0;
	if (b < 0)
		b = 0;

	return Color(r, g, b);
}