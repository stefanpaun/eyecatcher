// #include <Adafruit_NeoPixel.h>
// #include "A80_Automaton.h"
// #include "A51_Masks.h"


// #define NUM_LEDS_SCREEN 676 
// #define NUM_STRIPS 1
// #define CHANNEL1 19

// #define FRAMERATE_GROW 0.01 //so far 2 seems to run fine, 4 
// #define FRAMERATE_VARIANCE 5

// uint32_t nextTime1 = 0;
// uint32_t nextTime2 = 0;


// //CRGB leds[NUM_STRIPS * NUM_LEDS_SCREEN];


// // Pin layouts on the teensy 4
// //First: 1,0,24,25,   19,18,14,15,17,16,22,23,20,21,26,27
// //Second: 10,12,11,13,6,9,32,8,7
// //Third: 37, 36, 35, 34, 39, 38, 28, 31, 30



// Adafruit_NeoPixel screen_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, 19, NEO_GRB + NEO_KHZ800);

// int h1, s1, v1;
// int h2, s2, v2;

// Automaton bg_automaton_A = Automaton(1, 1, 1, 0, 0, 1, true, mask_A, background_A);
// Automaton fg_automaton_A = Automaton(1, 1, 1, 0, 0, 1, false, mask_A, background_A);
// Automaton grow_automaton_A = Automaton(1, 1, 1, 0, 0, 1, false, mask_A, background_A);


// void setup() {
  
  
//   //trng_init();

  
// //  byte h,s,v;
// //  for(int i = 0; i < NUM_STRIPS; i++) {
// //      for(int j = 0; j < NUM_LEDS_SCREEN; j++){
// //        h = mask_small[j%SIZE_SCREEN][int(j/SIZE_SCREEN)].getHue();
// //        s = mask_small[j%SIZE_SCREEN][int(j/SIZE_SCREEN)].getSaturation();
// //        v = min(mask_small[j%SIZE_SCREEN][int(j/SIZE_SCREEN)].getBright(), 200);
// //        leds[(i*NUM_LEDS_SCREEN) + j] = CHSV(h,s,v);
// //      }
// //  }
        
//   screen_A.begin();
//   delay(50);
//   fg_automaton_A.init();
//   bg_automaton_A.init();
//   grow_automaton_A.init();
//   delay(50);
// }

// void loop() {
  
//   if (millis() >= nextTime1){
//      fg_automaton_A.iterate();
//      bg_automaton_A.iterate();
//      nextTime1 = millis() + (1000/FRAMERATE_VARIANCE);
     
//      if(fg_automaton_A.allZero()){
//       fg_automaton_A.init();
//       Serial.println("Restart");
//      }
//      if(bg_automaton_A.allZero()){
//       bg_automaton_A.init();
//       Serial.println("Restart");
      
//      }
  
//     for(int i = 0; i < NUM_STRIPS; i++) {
//         for(int j = 0; j < NUM_LEDS_SCREEN; j++) { 
//           if (!fg_automaton_A.can_color(j%SIZE_SCREEN, int(j/SIZE_SCREEN))){
//             screen_A.setPixelColor((i*NUM_LEDS_SCREEN) + j, fg_automaton_A.colorMap(fg_automaton_A.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]), 80, 0);
//           }
//         }
//      }
//   } 
  
//   if (millis() >= nextTime2){
//     nextTime2 = millis() + (1000/FRAMERATE_GROW);
//     grow_automaton_A.iterate();  
//   }

//   screen_A.show(); 
   
// }

// void gen_color_matrix_A(){
//   int hue, sat, bri;
//   for(int i = 0; i < NUM_STRIPS; i++) {
//         for(int j = 0; j < NUM_LEDS_SCREEN; j++) { 
//           if (fg_automaton_A.can_color(j%SIZE_SCREEN, int(j/SIZE_SCREEN))){
//             ////screen_A.setPixelColor((i*NUM_LEDS_SCREEN) + j, fg_automaton_A.colorMap(fg_automaton_A.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]), 0, 0); //Get HUE
//             hue = fg_automaton_A.colorMap(fg_automaton_A.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
//           } else {
//             screen_A.setPixelColor((i*NUM_LEDS_SCREEN) + j, bg_automaton_A.colorMap(fg_automaton_A.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]), 0, 0);
//             hue = bg_automaton_A.colorMap(bg_automaton_A.cells[j%SIZE_SCREEN][int(j/SIZE_SCREEN)]);
//           }
//           //sat = grow_automaton_A.colorMap(grow_automaton_A
//         }
//      }
// }
