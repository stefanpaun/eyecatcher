#include "Arduino.h"
#include "Beam.h"

Beam::Beam() {
	active = false;
}

void Beam::begin(Adafruit_NeoPixel * _strip, bool _direction, Color _color, float _length, int _duration)
{
	active = _direction;
	strip = _strip;
	direction = _direction;
	color = _color;
	length = _length;
	duration = _duration;
	startTime = millis();
}

void Beam::update() {
	if (!active) return;
	if (millis() > startTime + duration) {
		active = false;
		return;
	}

	float posFac = (float)(millis() - startTime) / duration;
	if (direction == DOWN) posFac = 1 - posFac;
	position = (strip->numPixels() + length) * posFac - length / 2;
}


void Beam::draw() {
	if (!active) return;

	int startLed = position - length / 2;
	int endLed = position + length / 2;

	for (int i = startLed; i <= endLed; i++) {
		if (i >= 0 && i < strip->numPixels()) {
			float dist = constrain(1 - abs(i - position) / length * 2, 0, 1);
			Color c = color;
			c.multiply(dist);
			addPixelColor(strip, i, c);
		}

	}
}
