//-------NOTE: if you do clean, don't forget to fix core_pins.h for milis()


#define POD_ID 1                //ID of the teensy corresponding to the pod (1 - 5)

#define SETUP_DELAY 1000


//--------------Parameters for LEDs
#define SIZE_SCREEN 24          //Dimension of LED matrix
#define NUM_LEDS_SCREEN 576     //Number of LEDs for 24x24 matrix
#define NUM_LEDS_SYNAPSE 120    //CHANGE THIS ACCORDINGLY----------------------!!!!!!!!!!!!!!!

#define CHANNEL_A 7             //Data pin for Screen_A
#define CHANNEL_B 8             //Data pin for Screen_B
#define SYNAPSE_1 2             //Data pin for SYNAPSE_1
#define SYNAPSE_2 14            //Data pin for SYNAPSE_2



//--------------Parameters for Automaton framerates
#define RATIO_GROWTH 1  //so far 2 seems to run fine, 4 
//#define FRAMERATE_VARIANCE 10
//#define FRAMERATE_PULSES 1

#define AUTOMATON_INTERVAL 400
#define FADE_INTERVAL 1000/60
#define BEAM_UPDATE_INTERVAL 1000/60
#define BEAM_FREQUENCY_INTERVAL 800


//------------Parameters for brightness shifting
#define SHIFT_BRI 100  
#define LOW_BRI 100
#define HIGH_BRI 255




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



//--------------------------------------------Screens
Adafruit_NeoPixel matrix_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel matrix_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel synapse_A = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel synapse_B = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_2, NEO_GRB + NEO_KHZ800);



#include "BeamController.h"

//Automaton values:  target, reward, penalty, floorInit, minInit, maxInit
//----------------------------------------------------------------------------------------Automatons for Side A
Automaton fg_automaton_A; 
Automaton bg_automaton_A; 
Automaton grow_automaton_A;
Automaton sat_automaton_A; 
//----------------------------------------------------------------------------------------Automatons for Side B
Automaton fg_automaton_B;
Automaton bg_automaton_B; 
Automaton grow_automaton_B; 
Automaton sat_automaton_B; 


//-----------------------------------------------------------------------------------------
Screen screen_A = Screen(&fg_automaton_A, &bg_automaton_A, &grow_automaton_A, &sat_automaton_A, &matrix_A, &mask_A, &background_A);
Screen screen_B = Screen(&fg_automaton_B, &bg_automaton_B, &grow_automaton_B, &sat_automaton_B, &matrix_B, &mask_B, &background_B);


elapsedMillis sinceBeam;
elapsedMillis sinceFadeUpdate;

int filter_hue(int hue) {
  int dif = high_threshold - low_threshold;
  if (hue > low_threshold && hue < high_threshold) {
    hue = (hue + dif) % 360;
  }
  return hue;
}





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
  rtc_set(0);
  randomSeed(analogRead(17));

  screen_A.init_screen();
  screen_B.init_screen();

  synapse_A.begin();
  synapse_B.begin();
  
  sinceBeam = 0;
  sinceBeamUpdate = 0;
  sinceFadeUpdate = 0;

  delay(SETUP_DELAY);
}



void loop() {
  if(sinceBeam > BEAM_FREQUENCY_INTERVAL){
    sinceBeam = 0;
    newBeam(&synapse_B, random(0,2), Color(random(0,360), random(80, 101), 100, HSB_MODE),random(8,40),random(2000,6000));
    newBeam(&synapse_A, random(0,2), Color(random(0,360), random(80, 101), 100, HSB_MODE),random(8,40),random(2000,6000));
  }

  screen_A.iterate_animation();
  screen_B.iterate_animation();

  renderInterrupt();
}
