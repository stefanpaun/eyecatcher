#define BEAM_AM 10

Beam beams[BEAM_AM];

elapsedMillis sinceBeamUpdate = 0;



bool newBeam(Adafruit_NeoPixel* _strip, bool _direction, Color _color, float _length, int _duration) {
  for (int i = 0; i < BEAM_AM; i++) {
    if (!beams[i].active) {
      beams[i].begin(_strip, _direction, _color, _length, _duration);
      sinceBeamUpdate = 0;
      return true;
    }
  }
  return false;
}

void updateBeams() {
  if ((int)sinceBeamUpdate < BEAM_UPDATE_INTERVAL) return;
  sinceBeamUpdate = 0;

  synapse_A.clear();
  synapse_B.clear();

  for (int i = 0; i < BEAM_AM; i++) {
    if (beams[i].active) {
      beams[i].update();
      beams[i].draw();
    }
  }

  // for (int i = 0; i < synapse_B.numPixels(); i++) {
  //   synapse_B.setPixelColor(i,50,0,10);
  //   synapse_A.setPixelColor(i,50,0,10);
  // }

  synapse_A.show();
  synapse_B.show();
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
