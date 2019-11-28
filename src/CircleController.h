#define CIRCLE_AM 10


Circle circles[CIRCLE_AM];

elapsedMillis sinceCircleUpdate;



bool newCircle(Adafruit_NeoPixel* _strip, Color _c, int _duration, Cellmask (*_mask)[SIZE_SCREEN][SIZE_SCREEN]) {
	for (int i = 0; i < CIRCLE_AM; i++) {
		if (!circles[i].active) {
			circles[i].begin(_strip, SIZE_SCREEN / 2, SIZE_SCREEN / 2, _c, 2, 35, -3, _duration, _mask);
			return true;
		}
	}
	return false;
}

void updateCircles() {
	if ((int)sinceCircleUpdate < CIRCLE_UPDATE_INTERVAL) return;
	sinceCircleUpdate = 0;

	matrix_A.clear();
	matrix_B.clear();

	for (int i = 0; i < CIRCLE_AM; i++) {
		if (circles[i].active) {
			circles[i].update();
			circles[i].draw();
		}
	}

	// for (int i = 0; i < synapse_B.numPixels(); i++) {
	//   synapse_B.setPixelColor(i,50,0,10);
	//   synapse_A.setPixelColor(i,50,0,10);
	// }

	matrix_A.show();
	matrix_B.show();

	// Serial.print("Beam FrameTime: ");
	 //Serial.println(rtcMillis()-startTime);
}

struct RGB32 {
	byte b;
	byte g;
	byte r;
	byte a;
};

void addPixelColor(Adafruit_NeoPixel* strip, int pixel, Color c) {
	int colInt = strip->getPixelColor(pixel);
	RGB32* getCol = (RGB32*)&colInt;
	getCol->r = constrain(getCol->r + c.red(), 0, 255);
	getCol->g = constrain(getCol->g + c.green(), 0, 255);
	getCol->b = constrain(getCol->b + c.blue(), 0, 255);

	strip->setPixelColor(pixel, *(int*)getCol);
}
