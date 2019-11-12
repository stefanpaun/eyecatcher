
#define SIZE_SCREEN 26  //CHANGE TO 24 after testing
#define INT_MIN -32768



#define NUM_LEDS_SCREEN 676 //CHANGE TO 576 after testing, also update masks
#define NUM_LEDS_SYNAPSE 120 //CHANGE THIS ACCORDINGLY
#define NUM_STRIPS 1
#define CHANNEL_A 7 //Data pin for Screen A
#define CHANNEL_B 8//Data pin for Screen B

#define SYNAPSE_1 2 //Data pin for SYNAPSE_1
#define SYNAPSE_2 14 //Data pin for SYNAPSE_2

#define FRAMERATE_GROW 10   //so far 2 seems to run fine, 4 
#define FRAMERATE_VARIANCE 20
#define FRAMERATE_PULSES 1

#define STEP_SIZE 30
#define MIN_BRIGHTNESS 10  //Increases contrast, however also decreases overall brightness
#define SHIFT_BRI 0   // Should take values between 0 and 100ish (more than 100 is just overkill)

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "A80_Automaton.h"
#include "A51_Masks.h"
#include <math.h>

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

int low_threshold = 200;
int high_threshold = 300; 

// float prev_hue_A[SIZE_SCREEN][SIZE_SCREEN] = {0};
// float prev_sat_A[SIZE_SCREEN][SIZE_SCREEN] = {0};
// float prev_bri_A[SIZE_SCREEN][SIZE_SCREEN] = {0};


// Pin layouts on the teensy 4
//First: 1,0,24,25, 19,18,14,15,17,16,22,23,20,21,26,27
//Second: 10,12,11,13,6,9,32,8,7
//Third: 37, 36, 35, 34, 39, 38, 28, 31, 30

Adafruit_NeoPixel screen_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel screen_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);


//GIVE two digit identifier for synapse name
Adafruit_NeoPixel synapse_A = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel synapse_B = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE, SYNAPSE_2, NEO_GRB + NEO_KHZ800);


//Automaton values:  target, reward, penalty, floorInit, minInit, maxInit
//----------------------------------------------------------------------------------------Automatons for Side A
Automaton bg_automaton_A = Automaton(4, 1, 4, 0, 0, 1, true, mask_A, background_A);
Automaton fg_automaton_A = Automaton(2, 1, 2, 0, 0, 10, false, mask_A, background_A);
Automaton grow_automaton_A = Automaton(3, 1, 3, 0, 10, 20, false, mask_A, background_A);
Automaton sat_automaton_A = Automaton(2, 1, 1, 0, 1, 10, false, mask_A, background_A);
//----------------------------------------------------------------------------------------Automatons for Side B
Automaton bg_automaton_B = Automaton(4, 1, 4, 0, 0, 1, true, mask_B, background_B);
Automaton fg_automaton_B = Automaton(2, 1, 2, 0, 0, 10, false, mask_B, background_B);
Automaton grow_automaton_B = Automaton(3, 1, 3, 0, 10, 20, false, mask_B, background_B);
Automaton sat_automaton_B = Automaton(2, 1, 1, 0, 1, 10, false, mask_B, background_B);


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
int get_hue(nextCol * nextColors){
  float avg_hue = 0;
  int counter = 0;
  for(int i = 0; i < NUM_LEDS_SCREEN; i++){
    if((*nextColors).bri[i%SIZE_SCREEN][int(i/SIZE_SCREEN)] > 50){
    counter ++;
    avg_hue += (*nextColors).hue[i%SIZE_SCREEN][int(i/SIZE_SCREEN)];
    }
  }
  avg_hue = avg_hue / counter;
  return avg_hue; 
}

void instant_avg_bri(){
  float avg_bri = 0;
  for(int i = 0; i < NUM_LEDS_SCREEN; i++){
      avg_bri += next_colors_A.bri[i%SIZE_SCREEN][int(i/SIZE_SCREEN)];
  }
  avg_bri = avg_bri / NUM_LEDS_SCREEN;
  Serial.println(avg_bri);
}

void test_screen(){
  for (int i = 0; i < NUM_LEDS_SCREEN; i++){
    screen_A.setPixelColor(i, 0, 255, 0);
    screen_B.setPixelColor(i, 255, 0, 0);
    }
  screen_A.show();
  screen_B.show();
}

void send_pulse_on(int pixel, Adafruit_NeoPixel * synapse, int hue){
  int r, g, b;
  int sat = 255;
  int bri = 255;
  getRGB(hue, sat, bri, &r, &g, &b);
  (*synapse).setPixelColor(pixel, r, g, b);
  (*synapse).setPixelColor(pixel+1, r, g, b);
  (*synapse).setPixelColor(pixel+2, r, g, b);
  (*synapse).setPixelColor(pixel+3, r, g, b);
  (*synapse).setPixelColor(pixel+4, r, g, b);
  (*synapse).show();
  
}

void send_pulse_off(int pixel, Adafruit_NeoPixel * synapse, int hue){
  int r = 0, g = 0, b = 0;
  (*synapse).setPixelColor(pixel, r, g, b);
  (*synapse).setPixelColor(pixel+1, r, g, b);
  (*synapse).setPixelColor(pixel+2, r, g, b);
  (*synapse).setPixelColor(pixel+3, r, g, b);
  (*synapse).setPixelColor(pixel+4, r, g, b);
  (*synapse).show();
}

void send_pulse(Adafruit_NeoPixel * synapse, int hue){
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
    r = 0; g = 0; b = 0;
    (*synapse).setPixelColor(pixel, r, g, b);
    (*synapse).setPixelColor(pixel+1, r, g, b);
    (*synapse).setPixelColor(pixel+2, r, g, b);
    (*synapse).setPixelColor(pixel+3, r, g, b);
    (*synapse).setPixelColor(pixel+4, r, g, b);
    (*synapse).show();
  }
}
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

          //hue = filter_hue(hue);
          bri = shift_bri(bri);
          
          next_colors_A.hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = hue;
          next_colors_A.sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = sat;
          next_colors_A.bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = bri;
        }
     }
  instant_avg_bri();
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
        

          //hue = filter_hue(hue);
          bri = shift_bri(bri);

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
           
            //Serial.println("Prev and next bri");
            //Serial.println(current_bri);
            
            //Serial.println(current_bri);
            (*prev).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_hue;
            (*prev).sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_sat;
            (*prev).bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_bri;  

            // Serial.print(j);
            // Serial.print(" ");
            // Serial.print((*next).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
            // Serial.print(" ");
            // Serial.print((*prev).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
            // Serial.print(" ");
            // Serial.println(current_hue);
            
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




// void interpolate_colors_A(){
//   int r = 0, g = 0, b = 0;
//   float current_hue, current_sat, current_bri;
//   for (int step = 0; step < STEP_SIZE; step++){
//     for(int i = 0; i < NUM_STRIPS; i++) {
//           for(int j = 0; j < NUM_LEDS_SCREEN; j++) { 
//             current_hue = interpolate_function(next_colors_A.hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)], prev_colors_A.hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)],float(STEP_SIZE));
//             current_sat = interpolate_function(next_colors_A.sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)], prev_colors_A.sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)],float(STEP_SIZE)); 
//             current_bri = interpolate_function(next_colors_A.bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)], prev_colors_A.bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)],float(STEP_SIZE));

//             prev_colors_A.hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_hue;
//             prev_colors_A.sat[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_sat;
//             prev_colors_A.bri[j%SIZE_SCREEN][int(j/SIZE_SCREEN)] = current_bri;  

//             // Serial.print(j);
//             // Serial.print(" ");
//             // Serial.print((*next).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
//             // Serial.print(" ");
//             // Serial.print((*prev).hue[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
//             // Serial.print(" ");
//             // Serial.println(current_hue);
            
//             getRGB(current_hue, current_sat, current_bri, &r, &g, &b);
//             screen_A.setPixelColor((i*NUM_LEDS_SCREEN) + j, r, g, b);
//         }
//     }
//   screen_A.show();
//   }
  // matrix_copy(prev_hue_A, next_hue_A);
  // matrix_copy(prev_sat_A, next_sat_A);
  // matrix_copy(prev_bri_A, next_bri_A);
//}

void init_A(){
  screen_A.begin();
  fg_automaton_A.init();
  bg_automaton_A.init();
  grow_automaton_A.init_square();
  sat_automaton_A.init_square();
}

void init_B(){
  screen_B.begin();
  fg_automaton_B.init();
  bg_automaton_B.init();
  grow_automaton_B.init_square();
  sat_automaton_B.init_square();
}


void setup() {
  Serial.begin(250000); //to be removed later
  delay(100);
  synapse_A.begin();
  delay(100);
  synapse_B.begin();
  delay(100);
  randomSeed(analogRead(17));
  init_A();
  delay(100);
  init_B();
  delay(100);
}


int i = 0;
void loop() {
  i = (i + 1) % NUM_LEDS_SYNAPSE;
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
    if(grow_automaton_A.allZero()){
      grow_automaton_A.init_square();
    }

    if(grow_automaton_B.allZero()){
      grow_automaton_B.init_square();
    }

    grow_automaton_A.iterate_growth();
    grow_automaton_B.iterate_growth();

 
    nextTime2 = millis() + (1000/FRAMERATE_GROW);  
  }

  if (millis() >= nextTime3){
    
    // send_pulse_on(i, &synapse_A, 120);
    // send_pulse_on(i, &synapse_B, 120);
    // delay(500);
    // send_pulse_off(i, &synapse_A, 120);
    // send_pulse_off(i, &synapse_B, 120);
    send_pulse(&synapse_A, get_hue(&next_colors_A));
    delay(10);
    send_pulse(&synapse_B, get_hue(&next_colors_B));
    nextTime3 = millis() + (1000/FRAMERATE_PULSES);  
  }

  //test_screen();
  interpolate_colors(& next_colors_A, & prev_colors_A, & screen_A);
  interpolate_colors(& next_colors_B, & prev_colors_B, & screen_B);
   
}

