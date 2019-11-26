//-------NOTE: if you do clean, don't forget to fix core_pins.h for milis()


#define POD_ID 2


             //ID of the teensy corresponding to the pod (1 - 5)
#if POD_ID == 1
#define SETUP_DELAY 3500
#elif POD_ID == 2
#define SETUP_DELAY 3000
#elif POD_ID == 3
#define SETUP_DELAY 2500
#elif POD_ID == 4
#define SETUP_DELAY 2000
#elif POD_ID == 5
#define SETUP_DELAY 1500
#endif


//--------------Parameters for LEDs
#define SIZE_SCREEN 24          //Dimension of LED matrix
#define NUM_LEDS_SCREEN 576     //Number of LEDs for 24x24 matrix

#if POD_ID == 1
  #define NUM_LEDS_SYNAPSE_A 91
  #define NUM_LEDS_SYNAPSE_B 160
#elif POD_ID == 2
  #define NUM_LEDS_SYNAPSE_A 0
  #define NUM_LEDS_SYNAPSE_B 0
#elif POD_ID == 3
  #define NUM_LEDS_SYNAPSE_A 112
  #define NUM_LEDS_SYNAPSE_B 99
#elif POD_ID == 4
  #define NUM_LEDS_SYNAPSE_A 0
  #define NUM_LEDS_SYNAPSE_B 0
#elif POD_ID == 5
  #define NUM_LEDS_SYNAPSE_A 156
  #define NUM_LEDS_SYNAPSE_B 115
#endif

#define CHANNEL_A 7             //Data pin for Screen_A
#define CHANNEL_B 8             //Data pin for Screen_B
#define SYNAPSE_1 2             //Data pin for SYNAPSE_1
#define SYNAPSE_2 14            //Data pin for SYNAPSE_2



//--------------Parameters for Automaton framerates
#define RATIO_GROWTH 1  //so far 2 seems to run fine, 4 
//#define FRAMERATE_VARIANCE 10
//#define FRAMERATE_PULSES 1

#define AUTOMATON_INTERVAL 600
#define FADE_INTERVAL 1000/60
#define BEAM_UPDATE_INTERVAL 1000/60
#define BEAM_FREQUENCY_INTERVAL 800


//------------Parameters for brightness shifting
#define SHIFT_BRI 80  
#define LOW_BRI 90
#define HIGH_BRI 255







int bri_level = 255;

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




#if POD_ID == 1
  Adafruit_NeoPixel matrix_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
  Adafruit_NeoPixel matrix_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);
#elif POD_ID == 2
  Adafruit_NeoPixel matrix_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
  Adafruit_NeoPixel matrix_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);
#elif POD_ID == 3
  Adafruit_NeoPixel matrix_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);
  Adafruit_NeoPixel matrix_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
#elif POD_ID == 4
  Adafruit_NeoPixel matrix_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);
  Adafruit_NeoPixel matrix_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
#elif POD_ID == 5
  Adafruit_NeoPixel matrix_A = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_B, NEO_GRB + NEO_KHZ800);
  Adafruit_NeoPixel matrix_B = Adafruit_NeoPixel(NUM_LEDS_SCREEN, CHANNEL_A, NEO_GRB + NEO_KHZ800);
#endif

//--------------------------------------------Screens

Adafruit_NeoPixel synapse_A = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE_A, SYNAPSE_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel synapse_B = Adafruit_NeoPixel(NUM_LEDS_SYNAPSE_B, SYNAPSE_2, NEO_GRB + NEO_KHZ800);



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


void updateScreensFade() {
  if (sinceFadeUpdate < FADE_INTERVAL) return;
  sinceFadeUpdate = 0;

  screen_A.updateFade();
  screen_B.updateFade();
}


byte incomingByte;
String readBuffer;  

void readMessages(){
  if(Serial1.available()){
    while (Serial1.available()){
      incomingByte = Serial1.read();
      readBuffer += char(incomingByte);
    }
    Serial.println(readBuffer);
    if (readBuffer.equals("incb")){
      bri_level = constrain(bri_level + 50, LOW_BRI, HIGH_BRI);
      matrix_A.setBrightness(bri_level);
      matrix_B.setBrightness(bri_level);
    } else if (readBuffer.equals("decb")){
      bri_level = constrain(bri_level - 50, LOW_BRI, HIGH_BRI);
      matrix_A.setBrightness(bri_level);
      matrix_B.setBrightness(bri_level);
      Serial.println("down");
    }


  }
  readBuffer = "";
}


void renderInterrupt() {
 

  updateBeams();
  updateScreensFade();
}




void setup() {
  delay(SETUP_DELAY);
  rtc_set(0);
  randomSeed(analogRead(17)+analogRead(16));


  Serial.begin(9600);
  Serial1.begin(9600);

  screen_A.init_screen();
  screen_B.init_screen();

  synapse_A.begin();
  synapse_B.begin();
  
  sinceBeam = 0;
  sinceBeamUpdate = 0;
  sinceFadeUpdate = 0;

  
}



void loop() {
  readMessages();
  if(sinceBeam > BEAM_FREQUENCY_INTERVAL){
    sinceBeam = 0;
    newBeam(&synapse_B, random(0,2), Color(random(0,360), random(80, 101), 100, HSB_MODE),random(8,40),random(2000,6000));
    newBeam(&synapse_A, random(0,2), Color(random(0,360), random(80, 101), 100, HSB_MODE),random(8,40),random(2000,6000));
  }

  screen_A.iterate_animation();
  screen_B.iterate_animation();
 
  renderInterrupt();
}
