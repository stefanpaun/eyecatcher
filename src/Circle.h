// circles.h

#include "Vector.h"

//void addPixelColor(Adafruit_NeoPixel* strip, int pixel, Color c);


class Circle {

public:
	boolean active;
	Adafruit_NeoPixel* strip;
    Cellmask (* mask)[SIZE_SCREEN][SIZE_SCREEN];
	Vector pos;
	float size, BSize, ESize;
	Color color;

	int duration;
	unsigned long startTime;
	float borderSize;

	
	Circle() { active = false; };
	void begin(Adafruit_NeoPixel * _strip, float x, float y, Color _color, float _borderSize, float _BSize, float _ESize, int _duration, Cellmask (* _mask)[SIZE_SCREEN][SIZE_SCREEN]) {
		active = true;
		strip = _strip;
		pos.x = x;
		pos.y = y;
		color = _color;
		borderSize = _borderSize;
		BSize = _BSize;
		ESize = _ESize;
		duration = _duration;
        mask = _mask;
		startTime = rtcMillis();
	}

	void update() {
		if (!active) return;
		if (rtcMillis() > startTime + duration) {
			active = false;
			return;
		}

		float sizeRatio = (float)(rtcMillis() - startTime) / duration;
		size = BSize + (ESize - BSize)*sizeRatio;
	}

	void draw() {
		if (!active) return;
		
		int x1 = pos.x - size / 2;
		int x2 = pos.x + size / 2;
		int y1 = pos.y - size / 2;
		int y2 = pos.y + size / 2;


		for (int x = x1; x <= x2; x++) {
			for (int y = y1; y <= y2; y++) {

				if (x < 0 || x >= SIZE_SCREEN || y < 0 || y >= SIZE_SCREEN) continue;
				int ledID = x* SIZE_SCREEN + y;

				float Dx = x - pos.x;
				float Dy = y - pos.y;
				float dist = sqrt(Dx * Dx + Dy * Dy);


				float b = 1-min(abs((size / 2.) - dist) / borderSize,1);

                //float b = dist<size/2 ? 1 : 0;

				Color c = Color(mask[x][y]->getHue(), 100, 100, HSB_MODE);
				c.multiply(b);
				addPixelColor(strip, ledID, c);
			}
		}
	}
	

};