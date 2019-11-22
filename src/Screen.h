#define ALL 0
#define LINE 1
#define SQUARE 2
#define CIRCLE 3
#define MULTIPLE 4
class Screen {

	typedef struct {
		RGB pixel[SIZE_SCREEN][SIZE_SCREEN] = {{{0,0,0}}};
	} ColorMatrix;

	typedef struct {
		int target;
		int penalty;
		int reward;
		int floorInit;
		int minInit;
		int maxInit;
	} AutomatonValues;

	typedef struct {
		int seed;
		int size;
		bool rand;
	} Seed;
	
	AutomatonValues possible_automatons[4] = {
		{2, 1, 2, 0, 0, 10},
		{4, 1, 4, 0, 0, 1},
		{3, 1, 3, 0, 10, 20},
		{1, 1, 1, 0, 0, 1}
	};

	Seed possible_seeds[2] = {
		{ALL, 1, true},
		{SQUARE, random(2, 6), true},
	};
	

private:
	Automaton * _bg_automaton;
	Automaton * _fg_automaton;
	Automaton * _grow_automaton;
	Automaton * _sat_automaton;
	Adafruit_NeoPixel * _screen;
	Cellmask  (*_mask)[SIZE_SCREEN][SIZE_SCREEN];
	boolean (*_bg_mask)[SIZE_SCREEN][SIZE_SCREEN];
	bool newFrameReady;
	int ratio = 0;
	float fadeFactor = 0;
	unsigned long fadeStart = 0;

public:

	Screen(Automaton * fg_automaton, Automaton * bg_automaton, Automaton * grow_automaton, Automaton * sat_automaton, Adafruit_NeoPixel * screen, Cellmask (* mask)[SIZE_SCREEN][SIZE_SCREEN], boolean (* bg_mask)[SIZE_SCREEN][SIZE_SCREEN]) {
		_bg_automaton = bg_automaton;
		_fg_automaton = fg_automaton;
		_grow_automaton = grow_automaton;
		_sat_automaton = sat_automaton;
		_screen = screen;
		_mask = mask;
		_bg_mask = bg_mask;
		newFrameReady = false;
		
	}

	void define_automaton(){
		initialize_automaton(_fg_automaton, &possible_automatons[0], false);
		initialize_automaton(_bg_automaton, &possible_automatons[1], true);
		initialize_automaton(_grow_automaton, &possible_automatons[2], true);
		initialize_automaton(_sat_automaton, &possible_automatons[3], true);
	}

	void define_seeds(){
		initialize_seed(_fg_automaton, possible_seeds[0]);
		initialize_seed(_bg_automaton, possible_seeds[0]);
		initialize_seed(_grow_automaton, possible_seeds[1]);
		initialize_seed(_sat_automaton, possible_seeds[1]);
	}

	void initialize_automaton(Automaton * automaton, AutomatonValues * val, bool bg){
		*automaton = Automaton((*val).target, (*val).penalty, (*val).reward, (*val).floorInit, (*val).minInit, (*val).maxInit, bg, *_mask, *_bg_mask);
	}

	void initialize_seed(Automaton * automaton, Seed init_seed){
		switch (init_seed.seed){
			case LINE:
				(*automaton).init_line(init_seed.size, init_seed.rand);
				break;
			case SQUARE:
				(*automaton).init_square(init_seed.size, init_seed.rand);
				break;
			case CIRCLE:
				(*automaton).init_circle(init_seed.size, init_seed.rand);
				break;
			case MULTIPLE:
				(*automaton).init_multiple(init_seed.size, init_seed.rand);
				break;
			default:
				(*automaton).init();
			
		}
	}

	void init_screen() {	
		_screen->begin();
		_screen->setBrightness(HIGH_BRI);
		define_automaton();
		define_seeds();
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
				
				// hue = (*_mask)[x][y].getHue();
				// bri = (*_mask)[x][y].getBright();
				// sat = (*_mask)[x][y].getSaturation();


				//hue = (hue + 100)%360;
				//hue = filter_hue(hue);
				//bri = shift_bri(bri);

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
				fadeStart = rtcMillis();
        newFrameReady = false;
			}
			else return;
		}

		fadeFactor = (float)(rtcMillis() - fadeStart) / AUTOMATON_INTERVAL;
		fadeFactor = min(fadeFactor, 1);

		interpolate_colors();
		_screen->show();
	}


	
	void iterate_animation() {
		if (newFrameReady) return;
		(*_fg_automaton).iterate();
		(*_bg_automaton).iterate();
		ratio++;
		if (ratio == RATIO_GROWTH){
			(*_grow_automaton).iterate_growth();
			(*_sat_automaton).iterate_growth();
			ratio = 0;
		}

		gen_color_matrix();

		newFrameReady = true;
    Serial.println("newFrameReady");
	}

};
