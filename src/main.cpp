
#define SIZE_SCREEN 26
#define INT_MIN -32768



#define NUM_LEDS_SCREEN 676 
#define NUM_STRIPS 1
#define CHANNEL_A 3 //Data pin for Screen A
#define CHANNEL_B 6 //Data pin for Screen B

#define FRAMERATE_GROW 10 //so far 2 seems to run fine, 4 
#define FRAMERATE_VARIANCE 20
#define FRAMERATE_GEN 100

#define STEP_SIZE 25

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "A80_Automaton.h"
#include "A51_Masks.h"

uint32_t nextTime1 = 0;
uint32_t nextTime2 = 0;
uint32_t nextTime3 = 0;



typedef struct ColorMatrix{
  float hue[SIZE_SCREEN][SIZE_SCREEN];
  float sat[SIZE_SCREEN][SIZE_SCREEN];
  float bri[SIZE_SCREEN][SIZE_SCREEN];
} nextCol, prevCol;

nextCol next_colors_A;
nextCol next_colors_B;
prevCol prev_colors_A = {{0}};
prevCol prev_colors_B = {{0}};

// float prev_hue_A[SIZE_SCREEN][SIZE_SCREEN] = {0};
// float prev_sat_A[SIZE_SCREEN][SIZE_SCREEN] = {0};
// float prev_bri_A[SIZE_SCREEN][SIZE_SCREEN] = {0};


// Pin layouts on the teensy 4
//First: 1,0,24,25, 19,18,14,15,17,16,22,23,20,21,26,27
//Second: 10,12,11,13,6,9,32,8,7
//Third: 37, 36, 35, 34, 39, 38, 28, 31, 30

Adafruit_NeoPixel screen_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel screen_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);


//Automaton values:  target, reward, penalty, floorInit, minInit, maxInit
//----------------------------------------------------------------------------------------Automatons for Side A
Automaton bg_automaton_A = Automaton(4, 1, 4, 0, 0, 1, true, mask_A, background_A);
Automaton fg_automaton_A = Automaton(2, 1, 2, 0, 0, 10, false, mask_A, background_A);
Automaton grow_automaton_A = Automaton(3, 1, 1, 0, 10, 20, false, mask_A, background_A);
Automaton sat_automaton_A = Automaton(2, 1, 1, 0, 1, 10, false, mask_A, background_A);
//----------------------------------------------------------------------------------------Automatons for Side B
Automaton bg_automaton_B = Automaton(9, 1, 4, 0, 0, 1, true, mask_B, background_B);
Automaton fg_automaton_B = Automaton(7, 3, 2, 0, 0, 4, false, mask_B, background_B);
Automaton grow_automaton_B = Automaton(3, 1, 1, 0, 5, 50, false, mask_B, background_B);
Automaton sat_automaton_B = Automaton(2, 1, 1, 0, 1, 10, false, mask_B, background_A);

void gen_color_matrix_A(){
  int hue = 0, sat = 0, bri = 0;
  for(int i = 0; i < NUM_STRIPS; i++) {
        for(int j = 0; j < NUM_LEDS_SCREEN; j++) { 
          if (fg_automaton_A.can_color(j%SIZE_SCREEN, int(j/SIZE_SCREEN))){
            hue = fg_automaton_A.colorMap(fg_automaton_A.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
          } else {
            hue = bg_automaton_A.colorMap(bg_automaton_A.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
          }
          bri = grow_automaton_A.brightnessMap(grow_automaton_A.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
          sat = sat_automaton_A.saturationMap(grow_automaton_A.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
         
          next_colors_A.hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = hue;
          next_colors_A.sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = sat;
          next_colors_A.bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = bri;
        }
     }
}

void gen_color_matrix_B(){
  int hue = 0, sat = 0, bri = 0;
  for(int i = 0; i < NUM_STRIPS; i++) {
        for(int j = 0; j < NUM_LEDS_SCREEN; j++) {     
          if (fg_automaton_B.can_color(j%SIZE_SCREEN, int(j/SIZE_SCREEN))){
            hue = fg_automaton_B.colorMap(fg_automaton_B.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
          } else {
            hue = bg_automaton_B.colorMap(bg_automaton_B.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
          }
          bri = grow_automaton_B.brightnessMap(grow_automaton_B.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
          sat = sat_automaton_B.saturationMap(grow_automaton_B.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
        
          next_colors_B.hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = hue;
          next_colors_B.sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = sat;
          next_colors_B.bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = bri;
          // Serial.print(j);
          // Serial.print(" ");
          // Serial.print(r);
          // Serial.print(" ");
          // Serial.print(g);
          // Serial.print(" ");
          // Serial.println(b);
        }
     }
}


float interpolate_function(float big_val, float small_val, float step){
  return (big_val - small_val) * (1/step) + small_val;
}


void interpolate_colors(nextCol * next, prevCol * prev, Adafruit_NeoPixel * screen){
  int r = 0, g = 0, b = 0;
  float current_hue, current_sat, current_bri;
  for (int step = 0; step < STEP_SIZE; step++){
    for(int i = 0; i < NUM_STRIPS; i++) {
          for(int j = 0; j < NUM_LEDS_SCREEN; j++) { 
            current_hue = interpolate_function((*next).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)], (*prev).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)],float(STEP_SIZE));
            current_sat = interpolate_function((*next).sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)], (*prev).sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)],float(STEP_SIZE)); 
            current_bri = interpolate_function((*next).bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)], (*prev).bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)],float(STEP_SIZE));

            (*prev).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_hue;
            (*prev).sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_sat;
            (*prev).bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_bri;  

            Serial.print(j);
            Serial.print(" ");
            Serial.print((*next).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
            Serial.print(" ");
            Serial.print((*prev).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
            Serial.print(" ");
            Serial.println(current_hue);
            
            getRGB(current_hue, current_sat, current_bri, &r, &g, &b);
            (*screen).setPixelColor((i*NUM_LEDS_SCREEN) + j, r, g, b);
        }
    }
  (*screen).show();
  }
  // matrix_copy(prev_hue_A, next_hue_A);
  // matrix_copy(prev_sat_A, next_sat_A);
  // matrix_copy(prev_bri_A, next_bri_A);
}




void interpolate_colors_A(){
  int r = 0, g = 0, b = 0;
  float current_hue, current_sat, current_bri;
  for (int step = 0; step < STEP_SIZE; step++){
    for(int i = 0; i < NUM_STRIPS; i++) {
          for(int j = 0; j < NUM_LEDS_SCREEN; j++) { 
            current_hue = interpolate_function(next_colors_A.hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)], prev_colors_A.hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)],float(STEP_SIZE));
            current_sat = interpolate_function(next_colors_A.sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)], prev_colors_A.sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)],float(STEP_SIZE)); 
            current_bri = interpolate_function(next_colors_A.bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)], prev_colors_A.bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)],float(STEP_SIZE));

            prev_colors_A.hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_hue;
            prev_colors_A.sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_sat;
            prev_colors_A.bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_bri;  

            // Serial.print(j);
            // Serial.print(" ");
            // Serial.print((*next).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
            // Serial.print(" ");
            // Serial.print((*prev).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
            // Serial.print(" ");
            // Serial.println(current_hue);
            
            getRGB(current_hue, current_sat, current_bri, &r, &g, &b);
            screen_A.setPixelColor((i*NUM_LEDS_SCREEN) + j, r, g, b);
        }
    }
  screen_A.show();
  }
  // matrix_copy(prev_hue_A, next_hue_A);
  // matrix_copy(prev_sat_A, next_sat_A);
  // matrix_copy(prev_bri_A, next_bri_A);
}

void init_A(){
  screen_A.begin();
  fg_automaton_A.init();
  bg_automaton_A.init();
  grow_automaton_A.init_growth();
  sat_automaton_A.init_growth();
}

void init_B(){
  screen_B.begin();
  fg_automaton_B.init();
  bg_automaton_B.init();
  grow_automaton_B.init_growth();
  sat_automaton_B.init_growth();
}


void setup() {
  Serial.begin(250000); //to be removed later
  randomSeed(analogRead(0));
  init_A();
  delay(100);
  init_B();
  delay(100);
}

void loop() {
  if (millis() >= nextTime1){
    fg_automaton_A.iterate();
    bg_automaton_A.iterate();
    sat_automaton_A.iterate_growth();
    fg_automaton_B.iterate();
    bg_automaton_B.iterate();
    sat_automaton_B.iterate_growth();
    gen_color_matrix_A();
    gen_color_matrix_B();
    if(fg_automaton_A.allZero()){
      fg_automaton_A.init();
      bg_automaton_A.init();
    }
    if(bg_automaton_A.allZero()){
      bg_automaton_A.init();
      bg_automaton_B.init();
    }
    nextTime1 = millis() + (1000/FRAMERATE_VARIANCE);
  } 
  
  if (millis() >= nextTime2){
    grow_automaton_A.iterate_growth();
    grow_automaton_B.iterate_growth();
    nextTime2 = millis() + (1000/FRAMERATE_GROW);  
  }

  interpolate_colors(& next_colors_A, & prev_colors_A, & screen_A);
  interpolate_colors(& next_colors_B, & prev_colors_B, & screen_B);
  //interpolate_colors_A();
}

