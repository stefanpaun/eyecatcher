// ---- Beam effect ----

#ifndef Beam_h
#define Beam_h

#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include "Color.h"

#define UP 0
#define DOWN 1

void addPixelColor(Adafruit_NeoPixel* strip, int pixel, Color c);

class Beam{
	public:
		Beam();
		void begin(Adafruit_NeoPixel* _strip, bool _direction, Color _color, float _length, int _duration);

		bool active;
		Adafruit_NeoPixel* strip;
		bool direction;  // UP or DOWN
		Color color;
		float length;
		int duration;
		unsigned long startTime;
		float position;
		
		void update();
		void draw();
		

		
	private:
};

#endif
