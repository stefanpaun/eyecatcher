//-------NOTE: if you do clean, don't forget to fix core_pins.h for milis()


#define POD_ID 1                //ID of the teensy corresponding to the pod (1 - 5)




//--------------Parameters for LEDs
#define SIZE_SCREEN 24          //Dimension of LED matrix
#define NUM_LEDS_SCREEN 576     //Number of LEDs for 24x24 matrix
#define NUM_LEDS_SYNAPSE 120    //CHANGE THIS ACCORDINGLY----------------------!!!!!!!!!!!!!!!

#define CHANNEL_A 7             //Data pin for Screen_A
#define CHANNEL_B 8             //Data pin for Screen_B
#define SYNAPSE_1 2             //Data pin for SYNAPSE_1
#define SYNAPSE_2 14            //Data pin for SYNAPSE_2



//--------------Parameters for Automaton framerates
#define FRAMERATE_GROW 10  //so far 2 seems to run fine, 4 
#define FRAMERATE_VARIANCE 10
#define FRAMERATE_PULSES 1

#define AUTOMATON_INTERVAL 400
#define FADE_INTERVAL 1000/60 
#define BEAM_UPDATE_INTERVAL 1000/100
#define BEAM_FREQUENCY_INTERVAL 800


//------------Parameters for brightness shifting
#define SHIFT_BRI 100  
#define LOW_BRI 150
#define HIGH_BRI 255


#define LINE 1
#define SQUARE 2
#define CIRCLE 3
#define MULTIPLE 4

//------------Parameters for hue filtering
int low_threshold = 200;
int high_threshold = 300;



#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <Time.h>
#include <TimeAlarms.h>
#include "A80_Automaton.h"
#include "A51_Masks.h"
#include "Color.h"
#include "Beam.h"
#include "Screen.h"




Adafruit_NeoPixel matrix_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel matrix_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);
//GIVE two digit identifier for synapse name
Adafruit_NeoPixel synapse_A = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel synapse_B = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_2, NEO_GRB + NEO_KHZ800);



#include "BeamController.h"

//Automaton values:  target, reward, penalty, floorInit, minInit, maxInit
//----------------------------------------------------------------------------------------Automatons for Side A
Automaton fg_automaton_A; // = Automaton(2, 1, 2, 0, 0, 10, false, mask_A, background_A);
Automaton bg_automaton_A; // = Automaton(4, 1, 4, 0, 0, 1, true, mask_A, background_A);
Automaton grow_automaton_A; // = Automaton(3, 1, 3, 0, 10, 20, false, mask_A, background_A);
Automaton sat_automaton_A; // = Automaton(1, 1, 1, 0, 0, 1, false, mask_A, background_A);
//----------------------------------------------------------------------------------------Automatons for Side B
Automaton fg_automaton_B; // = Automaton(2, 1, 2, 0, 0, 10, false, mask_B, background_B);
Automaton bg_automaton_B; //= Automaton(4, 1, 4, 0, 0, 1, true, mask_B, background_B);
Automaton grow_automaton_B; //= Automaton(3, 1, 3, 0, 10, 20, false, mask_B, background_B);
Automaton sat_automaton_B; //= Automaton(1, 1, 1, 0, 0, 1, false, mask_B, background_B);


//-----------------------------------------------------------------------------------------
Screen screen_A = Screen(&fg_automaton_A, &bg_automaton_A, &grow_automaton_A, &sat_automaton_A, &matrix_A);
Screen screen_B = Screen(&fg_automaton_B, &bg_automaton_B, &grow_automaton_B, &sat_automaton_B, &matrix_B);


int filter_hue(int hue) {
  int dif = high_threshold - low_threshold;
  if (hue > low_threshold && hue < high_threshold) {
    hue = (hue + dif) % 360;
  }
  return hue;
}


void init_A() {
  matrix_A.begin();
  //matrix_A.setBrightness(255);
  fg_automaton_A.init();
  bg_automaton_A.init();
  grow_automaton_A.init_square(3, true);
  sat_automaton_A.init();
}

void init_B() {
  matrix_B.begin();
  //matrix_B.setBrightness(255);
  fg_automaton_B.init();
  bg_automaton_B.init();
  grow_automaton_B.init_square(3, true);
  sat_automaton_B.init();
}


elapsedMillis sinceFadeUpdate = 0;
void updateScreensFade() {
  if (sinceFadeUpdate < FADE_INTERVAL) return;
  sinceFadeUpdate = 0;

  screen_A.updateFade();
  screen_B.updateFade();
}

void renderInterrupt() {
  updateBeams();
  updateScreensFade();
}



typedef struct {
  int target;
  int penalty;
  int reward;
  int floorInit;
  int minInit;
  int maxInit;
} AutomatonValues;


void initialize_automaton_A(Automaton * automaton, AutomatonValues * val, bool bg){
  *automaton = Automaton((*val).target, (*val).penalty, (*val).reward, (*val).floorInit, (*val).minInit, (*val).maxInit, bg, mask_A, background_A);
}

void initialize_automaton_B(Automaton * automaton, AutomatonValues * val, bool bg){
  *automaton = Automaton((*val).target, (*val).penalty, (*val).reward, (*val).floorInit, (*val).minInit, (*val).maxInit, bg, mask_B, background_B);
}

void initialize_seed(Automaton * automaton, int seed){
  switch (seed){
    case LINE:
      (*automaton).init_line(4, true);
  }
}

elapsedMillis sinceBeam = 0;

void setup() {
  randomSeed(analogRead(17));
  fg_automaton_A = Automaton(2, 1, 2, 0, 0, 10, false, mask_A, background_A);
  bg_automaton_A = Automaton(4, 1, 4, 0, 0, 1, true, mask_A, background_A);
  grow_automaton_A = Automaton(3, 1, 3, 0, 10, 20, false, mask_A, background_A);
  sat_automaton_A = Automaton(1, 1, 1, 0, 0, 1, false, mask_A, background_A);
//----------------------------------------------------------------------------------------Automatons for Side B
  fg_automaton_B = Automaton(2, 1, 2, 0, 0, 10, false, mask_B, background_B);
  bg_automaton_B= Automaton(4, 1, 4, 0, 0, 1, true, mask_B, background_B);
  grow_automaton_B = Automaton(3, 1, 3, 0, 10, 20, false, mask_B, background_B);
  sat_automaton_B = Automaton(1, 1, 1, 0, 0, 1, false, mask_B, background_B);
  init_A();
  init_B();
  synapse_A.begin();
  synapse_B.begin();
  rtc_set(0);
  sinceBeam = 0;
  delay(1000);
  
}



void loop() {
  if(sinceBeam > BEAM_FREQUENCY_INTERVAL){
    sinceBeam = 0;
    newBeam(&synapse_B, random(0,2), Color(random(0,360), 100, 100, HSB_MODE),random(6,50),random(1500,3000));
    newBeam(&synapse_A, random(0,2), Color(random(0,360), 255, 255, HSB_MODE),random(6,50),random(1500,3000));
  }


  screen_A.iterate_animation();
  screen_B.iterate_animation();

  renderInterrupt();
}

