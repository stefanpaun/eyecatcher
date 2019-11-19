
#define SIZE_SCREEN 24 //CHANGE TO 24 after testing




#define NUM_LEDS_SCREEN 576 //CHANGE TO 576 after testing, also update masks
#define NUM_LEDS_SYNAPSE 120 //CHANGE THIS ACCORDINGLY
#define NUM_STRIPS 1
#define CHANNEL_A 7 //Data pin for Screen A
#define CHANNEL_B 8//Data pin for Screen B

#define SYNAPSE_1 2 //Data pin for SYNAPSE_1
#define SYNAPSE_2 14 //Data pin for SYNAPSE_2

#define FRAMERATE_GROW 10  //so far 2 seems to run fine, 4 
#define FRAMERATE_VARIANCE 10
#define FRAMERATE_PULSES 1

#define STEP_SIZE 10
#define MIN_BRIGHTNESS 10  //Increases contrast, however also decreases overall brightness
#define SHIFT_BRI 0   // Should take values between 0 and 100ish (more than 100 is just overkill)

const int AUTOMATON_INTERVAL = 400;
const int FADE_INTERVAL = 1000/40 ;

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


uint32_t nextTime3 = 0;


int low_threshold = 200;
int high_threshold = 300;



Adafruit_NeoPixel matrix_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel matrix_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);
//GIVE two digit identifier for synapse name
Adafruit_NeoPixel synapse_A = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel synapse_B = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_2, NEO_GRB + NEO_KHZ800);



#include "BeamController.h"

//Automaton values:  target, reward, penalty, floorInit, minInit, maxInit
//----------------------------------------------------------------------------------------Automatons for Side A
Automaton fg_automaton_A = Automaton(2, 1, 2, 0, 0, 10, false, mask_A, background_A);
Automaton bg_automaton_A = Automaton(4, 1, 4, 0, 0, 1, true, mask_A, background_A);
Automaton grow_automaton_A = Automaton(3, 1, 3, 0, 10, 20, false, mask_A, background_A);
Automaton sat_automaton_A = Automaton(1, 1, 1, 0, 0, 1, false, mask_A, background_A);
//----------------------------------------------------------------------------------------Automatons for Side B
Automaton fg_automaton_B = Automaton(2, 1, 2, 0, 0, 10, false, mask_B, background_B);
Automaton bg_automaton_B = Automaton(4, 1, 4, 0, 0, 1, true, mask_B, background_B);
Automaton grow_automaton_B = Automaton(3, 1, 3, 0, 10, 20, false, mask_B, background_B);
Automaton sat_automaton_B = Automaton(1, 1, 1, 0, 0, 1, false, mask_B, background_B);


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



//TEST FUNCTION
// int get_hue(nextCol * nextColors){
//   float avg_hue = 0;
//   int counter = 0;
//   for(int i = 0; i < NUM_LEDS_SCREEN; i++){
//     if((*nextColors).bri[i%SIZE_SCREEN][int(i/SIZE_SCREEN)] > 50){
//     counter ++;
//     avg_hue += (*nextColors).hue[i%SIZE_SCREEN][int(i/SIZE_SCREEN)];
//     }
//   }
//   avg_hue = avg_hue / counter;
//   return avg_hue; 
// }

// void instant_avg_bri(){
//   float avg_bri = 0;
//   for(int i = 0; i < NUM_LEDS_SCREEN; i++){
//       avg_bri += next_colors_A.bri[i%SIZE_SCREEN][int(i/SIZE_SCREEN)];
//   }
//   avg_bri = avg_bri / NUM_LEDS_SCREEN;
//   Serial.println(avg_bri);
// }



void init_A() {
  matrix_A.begin();
  //matrix_A.setBrightness(255);
  fg_automaton_A.init();
  bg_automaton_A.init();
  grow_automaton_A.init_square();
  sat_automaton_A.init_square();
}

void init_B() {
  matrix_B.begin();
  //matrix_B.setBrightness(255);
  fg_automaton_B.init();
  bg_automaton_B.init();
  grow_automaton_B.init_square();
  sat_automaton_B.init_square();
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




void setup() {
  randomSeed(analogRead(17));
  init_A();
  init_B();
  synapse_A.begin();
  synapse_B.begin();
  delay(1000);
}

elapsedMillis sinceBeam = 0;

void loop() {
  if(sinceBeam>1000){
    sinceBeam = 0;
    newBeam(&synapse_B, random(0,2), Color(random(0,360), 100, 100, HSB_MODE),random(6,50),random(300,1000));
    newBeam(&synapse_A, random(0,2), Color(random(0,360), 100, 100, HSB_MODE),random(6,50),random(300,1000));
  }


  screen_A.iterate_animation();
  screen_B.iterate_animation();

  renderInterrupt();
}

