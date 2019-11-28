#define BEAM_AM 10

Beam beams[BEAM_AM];

elapsedMillis sinceBeamUpdate;



bool newBeam(Adafruit_NeoPixel* _strip, bool _direction, Color _color, float _length, int _duration) {
  for (int i = 0; i < BEAM_AM; i++) {
    if (!beams[i].active) {
      beams[i].begin(_strip, _direction, _color, _length, _duration);
      return true;
    }
  }
  return false;
}

void updateBeams() {
  if ((int)sinceBeamUpdate < BEAM_UPDATE_INTERVAL) return;
  unsigned int startTime = rtcMillis();
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
  

 // Serial.print("Beam FrameTime: ");
  //Serial.println(rtcMillis()-startTime);
}


