
class Screen {

	typedef struct {
		RGB pixel[SIZE_SCREEN][SIZE_SCREEN] = {{{0,0,0}}};
	} ColorMatrix;

private:
	Automaton * _bg_automaton;
	Automaton * _fg_automaton;
	Automaton * _grow_automaton;
	Automaton * _sat_automaton;
	Adafruit_NeoPixel * _screen;
	bool newFrameReady;

	float fadeFactor = 0;
	unsigned long fadeStart = 0;

public:
	Screen(Automaton * fg_automaton, Automaton * bg_automaton, Automaton * grow_automaton, Automaton * sat_automaton, Adafruit_NeoPixel * screen) {
		_bg_automaton = bg_automaton;
		_fg_automaton = fg_automaton;
		_grow_automaton = grow_automaton;
		_sat_automaton = sat_automaton;
		_screen = screen;
		newFrameReady = false;
	}

	ColorMatrix* prev_colors = new ColorMatrix;
	ColorMatrix* currGoal_colors = new ColorMatrix;
	ColorMatrix* nextGoal_colors = new ColorMatrix;

	int shift_bri(int bri) {
		int val = bri;
		if (val > 150){
			val = constrain(val+SHIFT_BRI, LOW_BRI, HIGH_BRI);
		} else {
			val = 0;
		}
		
		return val;
	}


	void gen_color_matrix() {
		int hue = 0, sat = 0, bri = 0;
		for (int x = 0; x < SIZE_SCREEN; x++) {
			for (int y = 0; y < SIZE_SCREEN; y++) {
				if ((*_fg_automaton).can_color(x, y)) {
					hue = (*_fg_automaton).colorMap(x, y);
				}
				else {
					hue = (*_bg_automaton).colorMap(x, y);
				}
				bri = (*_grow_automaton).brightnessMap(x, y);
				sat = (*_sat_automaton).saturationMap(x, y);
				

				//hue = filter_hue(hue);
				bri = shift_bri(bri);

				getRGB(hue, sat, bri, &nextGoal_colors->pixel[x][y].r, &nextGoal_colors->pixel[x][y].g, &nextGoal_colors->pixel[x][y].b);
			}
		}
	}


	void interpolate_colors() {
		int r = 0, g = 0, b = 0;
		for (int x = 0; x < SIZE_SCREEN; x++) {
			for (int y = 0; y < SIZE_SCREEN; y++) {
				r = prev_colors->pixel[x][y].r * (1 - fadeFactor) + currGoal_colors->pixel[x][y].r * fadeFactor;
				g = prev_colors->pixel[x][y].g * (1 - fadeFactor) + currGoal_colors->pixel[x][y].g * fadeFactor;
				b = prev_colors->pixel[x][y].b * (1 - fadeFactor) + currGoal_colors->pixel[x][y].b * fadeFactor;
				_screen->setPixelColor(x*SIZE_SCREEN + y, r, g, b);
			}
		}
	}

	void updateFade() {
		if (fadeFactor >= 1) {
      Serial.println("end of fade");
			if (newFrameReady) {
        Serial.println("setup next fade");
				ColorMatrix* temp = prev_colors;
				prev_colors = currGoal_colors;
				currGoal_colors = nextGoal_colors;
				nextGoal_colors = temp;
				fadeStart = millis();
        newFrameReady = false;
			}
			else return;
		}

		fadeFactor = (float)(millis() - fadeStart) / AUTOMATON_INTERVAL;
		fadeFactor = min(fadeFactor, 1);

		interpolate_colors();
		_screen->show();
	}


	int ratio = 0;
	void iterate_animation() {
		if (newFrameReady) return;

		(*_fg_automaton).iterate();
		(*_bg_automaton).iterate();
		ratio++;
		if (ratio == 1){
			(*_grow_automaton).iterate_growth();
			(*_sat_automaton).iterate();
			ratio = 0;
		}

		gen_color_matrix();

		newFrameReady = true;
    Serial.println("newFrameReady");
	}

};
