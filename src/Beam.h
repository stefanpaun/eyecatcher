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

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
 
  for(int i = 0; i < strip->numPixels()*2; i++) {
   
   
    // fade brightness all LEDs one step
    for(int j=0; j<strip->numPixels(); j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
   
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <strip->numPixels()) && (i-j>=0) ) {
        strip->setPixelColor(i-j, red, green, blue);
      }
    }
   
    strip->show();
    delay(SpeedDelay);
  }
}

void fadeToBlack(int ledNo, byte fadeValue) {
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
   
    oldColor = strip->getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
   
    strip->setPixelColor(ledNo, r,g,b);
}


void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < strip->numPixels(); i++ ) {
    strip->setPixelColor(i, red, green, blue);
  }
  strip->show();
}
};