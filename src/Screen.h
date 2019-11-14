class Screen {

    typedef struct {
                float hue[SIZE_SCREEN][SIZE_SCREEN];
                float sat[SIZE_SCREEN][SIZE_SCREEN];
                float bri[SIZE_SCREEN][SIZE_SCREEN];
            } ColorMatrix;
  
    private:
        Automaton * _bg_automaton;
        Automaton * _fg_automaton;
        Automaton * _grow_automaton;
        Automaton * _sat_automaton;
        Adafruit_NeoPixel * _screen;
        bool makeNewFrame;
        int step;
        uint32_t nextTime1 = 0;
        uint32_t nextTime2 = 0;
        

  
    public: 
        Screen (Automaton * fg_automaton, Automaton * bg_automaton, Automaton * grow_automaton, Automaton * sat_automaton, Adafruit_NeoPixel * screen) {
            _bg_automaton = bg_automaton;
            _fg_automaton = fg_automaton;
            _grow_automaton = grow_automaton;
            _sat_automaton = sat_automaton;
            _screen = screen;
            step = 0;
            makeNewFrame = true;
        }
        
        ColorMatrix next_colors;
        ColorMatrix prev_colors = {{{0}}};


    void gen_color_matrix(){
        int hue = 0, sat = 0, bri = 0;
        for(int x = 0; x < SIZE_SCREEN; x++) {
            for(int y = 0; y < SIZE_SCREEN; y++) { 
                if ((*_fg_automaton).can_color(x, y)){
                    hue = (*_fg_automaton).colorMap(x,y);
                } else {
                    hue = (*_bg_automaton).colorMap(x,y);
                }
            
                bri = (*_grow_automaton).brightnessMap(x,y);
                sat = (*_sat_automaton).saturationMap(x,y);

                //hue = filter_hue(hue);
                //bri = shift_bri(bri);
                
                next_colors.hue[x][y] = hue;
                next_colors.sat[x][y] = sat;
                next_colors.bri[x][y] = bri;
            }
        }   
        makeNewFrame = false;
       
    }

    float interpolate_function(float big_val, float small_val, float step){
        return (big_val - small_val) * (1/step) + small_val;
    }

    void interpolate_colors(int step){
        int r = 0, g = 0, b = 0;
        float current_hue, current_sat, current_bri;
        for(int x = 0; x < SIZE_SCREEN; x++) {
            for(int y = 0; y < SIZE_SCREEN; y++) { 
                current_hue = interpolate_function(next_colors.hue[x][y], prev_colors.hue[x][y],float(STEP_SIZE));
                current_sat = interpolate_function(next_colors.sat[x][y], prev_colors.sat[x][y],float(STEP_SIZE)); 
                current_bri = interpolate_function(next_colors.bri[x][y], prev_colors.bri[x][y],float(STEP_SIZE));

                prev_colors.hue[x][y] = current_hue;
                prev_colors.sat[x][y] = current_sat;
                prev_colors.bri[x][y] = current_bri;  

                getRGB(current_hue, current_sat, current_bri, &r, &g, &b);
                (*_screen).setPixelColor((x*SIZE_SCREEN + y), r, g, b);
            }
        }
       
        
    }   

    void iterate_animation(){
        if (makeNewFrame){
            if (millis() >= nextTime1){
                (*_fg_automaton).iterate();
                (*_bg_automaton).iterate();
                nextTime1 = millis() + (1000/FRAMERATE_VARIANCE);
                Serial.println(nextTime1);
            }
            if (millis() >= nextTime2){
                (*_grow_automaton).iterate_growth();
                (*_sat_automaton).iterate();
                nextTime2 = millis() + (1000/FRAMERATE_GROW);
            }
            
            gen_color_matrix();
        }
        interpolate_colors(step);
        if (step == STEP_SIZE){
            makeNewFrame = true;
            step = step % STEP_SIZE;
        }
        step = (step + 1);
    }
};