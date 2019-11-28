#define ALL 0
#define LINE 1
#define SQUARE 2
#define CIRCLE 3
#define MULTIPLE 4
#define IMPLOSION 5
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
	

	AutomatonValues sat_automatons = {
		1, 1, 1, 0, 0, 3
	};

	AutomatonValues hue_automatons[7] = {
		{3, 1, 3, 0, 0, 4},
		{3, 1, 1, 0, 0, 4},
		{2, 1, 2, 0, 0, 3},     
		{2, 1, 1, 0, 0, 6},
		{1, 2, 1, 0, 0, 2},
		{7, 7, 4, 0, 0, 4}, 
		{8, 1, 1, 0, 0, 3}
	};

	AutomatonValues bri_automatons[7] = {
		{7, 7, 2, 0, 0, 50},    
		{7, 7, 3, 0, 0, 50},	
		{5, 5, 2, 0, 0, 50},    
		{5, 5, 3, 0, 0, 50},
		{6, 6, 4, 0, 0, 50},
		{6, 1, 6, 0, 0, 50},
		{9, 1, 9, 0, 0, 50}			
	};

	Seed hue_seeds[1] = {
		{ALL, 1, true},
	};

	Seed bri_seeds[5] = {
		{IMPLOSION, 1, false},
		{SQUARE, random(2, 3), false},
		{CIRCLE, random(2, 4), false},
		{LINE, random(3, 10), false}
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


	elapsedMillis sinceFadeUpdate;
	


public:
	int bottom_hue_threshold;
	int hue_difference;
	Screen(Automaton * fg_automaton, Automaton * bg_automaton, Automaton * grow_automaton, Automaton * sat_automaton, Adafruit_NeoPixel * screen, Cellmask (* mask)[SIZE_SCREEN][SIZE_SCREEN], boolean (* bg_mask)[SIZE_SCREEN][SIZE_SCREEN]) {
		_bg_automaton = bg_automaton;
		_fg_automaton = fg_automaton;
		_grow_automaton = grow_automaton;
		_sat_automaton = sat_automaton;
		_screen = screen;
		_mask = mask;
		_bg_mask = bg_mask;
		newFrameReady = false;
		bottom_hue_threshold = random(0, 360);
		hue_difference = random(60, 260);
		sinceFadeUpdate = 0;
	}

	Screen(){};

	void define_automaton_ceremony(){
		initialize_automaton(_fg_automaton, &hue_automatons[6], false);
		initialize_automaton(_bg_automaton, &sat_automatons, true);
		initialize_automaton(_grow_automaton, &bri_automatons[2], true);
		initialize_automaton(_sat_automaton, &sat_automatons, true);
	}

	void define_automaton(){
		
		initialize_automaton(_fg_automaton, &hue_automatons[random(0, 8)], false);
		initialize_automaton(_bg_automaton, &sat_automatons, true);
		initialize_automaton(_grow_automaton, &bri_automatons[random(0, 8)], true);
		initialize_automaton(_sat_automaton, &sat_automatons, true);
	}

	void define_seeds(){
		initialize_seed(_fg_automaton, hue_seeds[0]);
		initialize_seed(_bg_automaton, hue_seeds[0]);
		initialize_seed(_grow_automaton, bri_seeds[random(0, 4)]);
		initialize_seed(_sat_automaton, bri_seeds[random(0, 4)]);
	}

	void define_seeds_ceremony(){
		initialize_seed(_fg_automaton, hue_seeds[0]);
		initialize_seed(_bg_automaton, hue_seeds[0]);
		initialize_seed(_grow_automaton, bri_seeds[1]);
		initialize_seed(_sat_automaton, hue_seeds[0]);
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
			case IMPLOSION:
				(*automaton).init_implosion(init_seed.rand);
				break;
			default:
				(*automaton).init();
			
		}
	}

	void init_screen() {	
		//_screen->begin();
		_screen->setBrightness(HIGH_BRI);
		define_automaton();
		define_seeds();
	}

	// void regular_screen_animation(){
	// 	define_automaton();
	// 	define_seeds();
	// }

	// void special_screen_animation(){
	// 	define_automaton_ceremony();
	// 	define_seeds_ceremony();
	// }

	ColorMatrix* prev_colors = new ColorMatrix;
	ColorMatrix* currGoal_colors = new ColorMatrix;
	ColorMatrix* nextGoal_colors = new ColorMatrix;

	int shift_bri(int bri) {
		int val = bri;
		if (val > LOW_BRI){
			val = constrain(val+SHIFT_BRI, LOW_BRI, HIGH_BRI);
		} else {
			val = 0;
		}
		
		return val;
	}

	int filter_hue(int hue){
		int shift_val = bottom_hue_threshold + (int) hue_difference/2;
		int val = (hue - shift_val) % 360 ;
		int bottom = (bottom_hue_threshold - shift_val) % 360;
		int top = (bottom_hue_threshold + hue_difference - shift_val) % 360;
		int result = map(val, 0, 360,  top, bottom);
		return result;
	}

	void gen_color_matrix() {
		int hue = 0, sat = 0, bri = 0;
		for (int x = 0; x < SIZE_SCREEN; x++) {
			for (int y = 0; y < SIZE_SCREEN; y++) {
				bri = (*_grow_automaton).brightnessMap(x, y);
				sat = (*_sat_automaton).saturationMap(x, y);	
				if ((*_fg_automaton).can_color(x, y)) {
					hue = (*_fg_automaton).colorMap(x, y);
				} else {
					hue = (*_bg_automaton).colorMap(x, y);
				}
				
				hue = filter_hue(hue);
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

	bool updateFade(int id) {
		if (sinceFadeUpdate < FADE_INTERVAL) return false;
		sinceFadeUpdate = 0;
		if (fadeFactor >= 1) {
      		
			if (newFrameReady) {
				Serial.print(id);
        		Serial.println("setup next fade");
				ColorMatrix* temp = prev_colors;
				prev_colors = currGoal_colors;
				currGoal_colors = nextGoal_colors;
				nextGoal_colors = temp;
				fadeStart = rtcMillis();
        		newFrameReady = false;
			}
			else{
				Serial.print(id);
				Serial.println("No next frame ready!!!");
				return true;
			}
		}

		fadeFactor = (float)(rtcMillis() - fadeStart) / AUTOMATON_INTERVAL;
		fadeFactor = min(fadeFactor, 1);

		interpolate_colors();
		_screen->show();
		return true;
	}


	void iterate_animation(){
		
		if (newFrameReady) return;

		if (allZero(_grow_automaton)){
			initialize_automaton(_grow_automaton, &bri_automatons[random(0, 8)], true);
			initialize_seed(_grow_automaton, bri_seeds[random(0, 5)]);
		}

		if (allZero(_fg_automaton)){
			initialize_automaton(_fg_automaton, &hue_automatons[random(0, 8)], false);
			initialize_seed(_fg_automaton, hue_seeds[0]);
		}
		
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
	}

	bool allZero(Automaton * automaton){
		for (int i = 0; i< SIZE_SCREEN; i++){
			for(int j = 0; j < SIZE_SCREEN; j++){
				if ((*automaton).cells[i][j] != 0){
					return false;
				}
			}
		}
		return true;
	}
};
