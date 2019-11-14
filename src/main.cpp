
#define SIZE_SCREEN 26  //CHANGE TO 24 after testing
#define INT_MIN -32768



#define NUM_LEDS_SCREEN 676 //CHANGE TO 576 after testing, also update masks
#define NUM_LEDS_SYNAPSE 120 //CHANGE THIS ACCORDINGLY
#define NUM_STRIPS 1
#define CHANNEL_A 7 //Data pin for Screen A
#define CHANNEL_B 8//Data pin for Screen B

#define SYNAPSE_1 2 //Data pin for SYNAPSE_1
#define SYNAPSE_2 14 //Data pin for SYNAPSE_2

#define FRAMERATE_GROW 6  //so far 2 seems to run fine, 4 
#define FRAMERATE_VARIANCE 10
#define FRAMERATE_PULSES 1

#define STEP_SIZE 10
#define MIN_BRIGHTNESS 10  //Increases contrast, however also decreases overall brightness
#define SHIFT_BRI 0   // Should take values between 0 and 100ish (more than 100 is just overkill)

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <Time.h>
#include <TimeAlarms.h>
#include "A80_Automaton.h"
#include "A51_Masks.h"
#include "Screen.h"
#include "Colore.h"



uint32_t nextTime3 = 0;


int low_threshold = 200;
int high_threshold = 300; 


#define LED_AM 120
#define BEAM_AM 1

Beam beams[BEAM_AM];

Segment seg[] = {
  Segment(0, 119)
};


byte segAm = sizeof(seg)/sizeof(Segment);





Adafruit_NeoPixel matrix_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel matrix_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);
//GIVE two digit identifier for synapse name
Adafruit_NeoPixel synapse_A = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel synapse_B = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_2, NEO_GRB + NEO_KHZ800);



void set_ledLib(int pixel, byte r, byte g, byte b){
  synapse_A.setPixelColor(pixel, r, g, b);
}

void show_ledLib(){
  synapse_A.show();
}

void reset_ledLib(){
  for(int i=0; i<LED_AM; i++){
    synapse_A.setPixelColor(i,0,0,0);
  }
}

Color get_ledLib(int pixel){
  uint32_t conn = synapse_A.getPixelColor(pixel);  // retrieve the color that has already been saved
  byte b = conn & 255;       // unpack the color
  byte g = conn >> 8 & 255;
  byte r = conn >> 16 & 255;
  Color pixelCol(r,g,b,RGB_MODE);
  return pixelCol;
}


Colore colore( LED_AM, seg, segAm, beams, BEAM_AM, &set_ledLib, &get_ledLib, &show_ledLib, &reset_ledLib );

//Automaton values:  target, reward, penalty, floorInit, minInit, maxInit
//----------------------------------------------------------------------------------------Automatons for Side A
Automaton fg_automaton_A = Automaton(2, 1, 2, 0, 0, 10, false, mask_A, background_A);
Automaton bg_automaton_A = Automaton(4, 1, 4, 0, 0, 1, true, mask_A, background_A);
Automaton grow_automaton_A = Automaton(3, 1, 3, 0, 10, 20, false, mask_A, background_A);
Automaton sat_automaton_A = Automaton(2, 1, 1, 0, 1, 10, false, mask_A, background_A);
//----------------------------------------------------------------------------------------Automatons for Side B
Automaton fg_automaton_B = Automaton(2, 1, 2, 0, 0, 10, false, mask_B, background_B);
Automaton bg_automaton_B = Automaton(4, 1, 4, 0, 0, 1, true, mask_B, background_B);
Automaton grow_automaton_B = Automaton(3, 1, 3, 0, 10, 20, false, mask_B, background_B);
Automaton sat_automaton_B = Automaton(2, 1, 1, 0, 1, 10, false, mask_B, background_B);


//-----------------------------------------------------------------------------------------
Screen screen_A = Screen(&fg_automaton_A, &bg_automaton_A, &grow_automaton_A, &sat_automaton_A, &matrix_A);
Screen screen_B = Screen(&fg_automaton_B, &bg_automaton_B, &grow_automaton_B, &sat_automaton_B, &matrix_B);


int filter_hue(int hue){
  int dif = high_threshold - low_threshold;
  if (hue > low_threshold && hue < high_threshold){
    hue = (hue + dif) % 360;
  }
  return hue;
}


int shift_bri(int bri){
  int val = bri;
  if (val < 150){
    val = 0;
  } else {
  val = min(255, bri + SHIFT_BRI);
  }
  return val;
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



void send_pulse(Adafruit_NeoPixel * synapse, int hue, int pixel){
  int r, g, b;
  int sat = 255;
  int bri = 255;
    getRGB(hue, sat, bri, &r, &g, &b);
    (*synapse).setPixelColor(pixel, r, g, b);
    (*synapse).setPixelColor(pixel+1, r, g, b);
    (*synapse).setPixelColor(pixel+2, r, g, b);
    (*synapse).setPixelColor(pixel+3, r, g, b);
    (*synapse).setPixelColor(pixel+4, r, g, b);

}

void send_pulse_full(Adafruit_NeoPixel * synapse, int hue){
  int r, g, b;
  int sat = 255;
  int bri = 255;
  for (int pixel = 0; pixel < NUM_LEDS_SYNAPSE; pixel++){
    getRGB(hue, sat, bri, &r, &g, &b);
    (*synapse).setPixelColor(pixel, r, g, b);
    (*synapse).setPixelColor(pixel+1, r, g, b);
    (*synapse).setPixelColor(pixel+2, r, g, b);
    (*synapse).setPixelColor(pixel+3, r, g, b);
    (*synapse).setPixelColor(pixel+4, r, g, b);
    (*synapse).show();
    (*synapse).clear();
    (*synapse).show();
  }

}

void init_A(){
  matrix_A.begin();
  fg_automaton_A.init();
  bg_automaton_A.init();
  grow_automaton_A.init_square();
  sat_automaton_A.init_square();
}

void init_B(){
  matrix_B.begin();
  fg_automaton_B.init();
  bg_automaton_B.init();
  grow_automaton_B.init_square();
  sat_automaton_B.init_square();
}



void setup() { 
  randomSeed(analogRead(17));
  init_A();
  init_B();
  synapse_A.begin();
  //seg[0].setPulsate(Color(100, 20, 30, RGB_MODE), 2);
  delay(1000);
  
}

int pixel = 0;
void loop() {
   
  // if (millis() >= nextTime3){
  //     //synapse_A.clear();
  //     nextTime3 = millis() + (1000/FRAMERATE_PULSES);
  //    pixel = (pixel + 1) % NUM_LEDS_SYNAPSE;
  //    //send_pulse(&synapse_A, 200, pixel);
  //   send_pulse_full(&synapse_A, 200);
  // }
 
  //iterate();
  //iterate_growth_automaton();
  screen_A.iterate_animation();
  screen_B.iterate_animation();
  
  colore.update();
  matrix_A.show();
  matrix_B.show();
  //synapse_A.show();
   
   

  // synapse_A.show();
}



// void test_screen(){
//   for (int i = 0; i < NUM_LEDS_SCREEN; i++){
//     screen_A.setPixelColor(i, 0, 255, 0);
//     screen_B.setPixelColor(i, 255, 0, 0);
//     }
//   screen_A.show();
//   screen_B.show();
// }


