// ---- Beam effect ----


#define UP 0
#define DOWN 1

void addPixelColor(Adafruit_NeoPixel* strip, int pixel, Color c);

class Beam{
	public:
		bool active;
		Adafruit_NeoPixel* strip;
		bool direction;  // UP or DOWN
		Color color;
		float length;
		int duration;
		unsigned long startTime;
		float position;
		

		

Beam() {
	active = false;
}

void begin(Adafruit_NeoPixel * _strip, bool _direction, Color _color, float _length, int _duration)
{
	active = true;
	strip = _strip;
	direction = _direction;
	color = _color;
	length = _length;
	duration = _duration;
	startTime = rtcMillis();
}

void update() {
	if (!active) return;
	if (rtcMillis() > startTime + duration) {
		active = false;
		return;
	}

	float posFac = (float)(rtcMillis() - startTime) / duration;
	if (direction == DOWN) posFac = 1 - posFac;
	position = (strip->numPixels() + length) * posFac - length / 2;
}


void draw() {
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

};