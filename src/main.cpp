//-------NOTE: if you do clean, don't forget to fix core_pins.h for milis()


#define POD_ID 3


#define AUTOMATON_INTERVAL 4000
#define FADE_INTERVAL 1000/30

#define BEAM_UPDATE_INTERVAL 1000/40
#define CIRCLE_UPDATE_INTERVAL 1000/60


#define BEAM_FREQUENCY_INTERVAL_1 900
#define BEAM_FREQUENCY_INTERVAL_2 800
#define CIRCLE_FREQUENCY_INTERVAL 2000
#define RATIO_GROWTH 1



             //ID of the teensy corresponding to the pod (1 - 5)
#if POD_ID == 1
#define SETUP_DELAY 5500 //5500
#elif POD_ID == 2
#define SETUP_DELAY 5000
#elif POD_ID == 3
#define SETUP_DELAY 4500
#elif POD_ID == 4
#define SETUP_DELAY 4000
#elif POD_ID == 5
#define SETUP_DELAY 3500
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







//------------Parameters for brightness shifting
#define SHIFT_BRI 70  
#define LOW_BRI 120
#define HIGH_BRI 255
int bri_level = 205;

#define CHANNEL_A 7             //Data pin for Screen_A
#define CHANNEL_B 8             //Data pin for Screen_B
#define SYNAPSE_1 2             //Data pin for SYNAPSE_1
#define SYNAPSE_2 14            //Data pin for SYNAPSE_2



bool isRegularAnimation;

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <EEPROM.h>
#include "A80_Automaton.h"
#include "A51_Masks.h"
#include "Color.h"
#include "Beam.h"
#include "Screen.h"
#include "Circle.h"





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
#include "CircleController.h"
#include "OpeningCeremony.h"

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
Screen screen_A; //= Screen(&fg_automaton_A, &bg_automaton_A, &grow_automaton_A, &sat_automaton_A, &matrix_A, &mask_A, &background_A);
Screen screen_B; //= Screen(&fg_automaton_B, &bg_automaton_B, &grow_automaton_B, &sat_automaton_B, &matrix_B, &mask_B, &background_B);


elapsedMillis sinceBeam1;
elapsedMillis sinceBeam2;
elapsedMillis sinceCircle;


bool screenFadeUpdateSwitcher = false;

boolean automatonReinitializeCeremony = true;
boolean implosionDone = false;
elapsedMillis implosionTimer;



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

void readAnimationType(){
  String animReadBuffer = "";
  unsigned int readStartTime = rtcMillis();
  isRegularAnimation = true;
  while(rtcMillis()<readStartTime+5000){
      while (Serial1.available()){
        char inChar = Serial1.read();
        if(inChar=='r' || inChar=='s') animReadBuffer = "";
        animReadBuffer += inChar;
      }
      Serial.println(animReadBuffer);
      if (animReadBuffer.equals("reg")){
        isRegularAnimation = true;
        break;
      } else if (animReadBuffer.equals("spec")){
        isRegularAnimation = false;
        break;
      }
  }
}

unsigned int randomSeedN = 0;
void updateRandomSeed(){
  EEPROM.get(0,randomSeedN);
  randomSeedN++;
  EEPROM.put(0,randomSeedN);
  randomSeed(randomSeedN);
}

elapsedMillis sinceAutomatonInit;
elapsedMillis sinceLastPulses;

void regular_setup(){
  screen_A.init_screen();
  screen_B.init_screen();
  sinceAutomatonInit = 0;
  sinceLastPulses = 0;
}

int numPulses = 0;

void regular_animation(){
  if(sinceLastPulses > 14000  && sinceAutomatonInit > 2000 ){
    if (numPulses == 20){
      sinceLastPulses = 0;
      numPulses = 0;
    } else {
      numPulses++;
    }

    if(sinceBeam1 > BEAM_FREQUENCY_INTERVAL_1){
      sinceBeam1 = 0;
      int rand1 = random(0, 2);
      Color color1;
      if (rand1 == 0){
        color1 = Color(random(screen_B.bottom_hue_threshold, screen_B.bottom_hue_threshold + screen_B.hue_difference), random(50, 75), random(55, 80), HSB_MODE);
      } else {
        color1 = Color(random(0,260), random(50, 75), random(55, 80), HSB_MODE);
      }
      newBeam(&synapse_A, rand1, color1, random(3, 30),random(1000,6000));
      
    }

    if (sinceBeam2 > BEAM_FREQUENCY_INTERVAL_2 ){
      sinceBeam2 = 0;
      int rand2 = random(0, 2);
      Color color2;
      if (rand2 == 0){
        color2 = Color(random(screen_A.bottom_hue_threshold, screen_A.bottom_hue_threshold + screen_A.hue_difference), random(50, 75), random(55, 80), HSB_MODE);
      } else {
        color2 = Color(random(0,260), random(50, 75), random(55, 80), HSB_MODE);
      }
      newBeam(&synapse_B, rand2, color2, random(3, 30),random(1000,6000));
    }

    


  }

  screen_A.iterate_animation();
  screen_B.iterate_animation();
 
  renderInterrupt(); 
}


void updateScreensFade() {
  if(screenFadeUpdateSwitcher){
    if(screen_A.updateFade(0)) screenFadeUpdateSwitcher = !screenFadeUpdateSwitcher;
  }else{
    if(screen_B.updateFade(1))  screenFadeUpdateSwitcher = !screenFadeUpdateSwitcher;
  }
}

void renderInterrupt() {
  updateBeams();
  updateScreensFade();
}


boolean didPulse = false;
boolean didIntro = false;
boolean startAutomaton = false;
elapsedMillis sinceIntro = 0;
int freq_offset = 500;
int speed_offset = 0;
void ceremony_animation(){

  	if (!didPulse && !didIntro) {
    Color c = Color(204, 57, 90, HSB_MODE);
		newBeam(&synapse_A, true, c, NUM_LEDS_SYNAPSE_A*1.5, 7 *1000);
		newBeam(&synapse_A, false, c, NUM_LEDS_SYNAPSE_A*1.5, 7* 1000);
		newBeam(&synapse_B, true, c, NUM_LEDS_SYNAPSE_B*1.5, 7 * 1000);
		newBeam(&synapse_B, false, c, NUM_LEDS_SYNAPSE_B*1.5, 7 * 1000);
		sinceCircle = 0;

    didPulse = true;
	}

  
	if (sinceCircle > 6000 && didPulse && !didIntro) {
    newCircle(&matrix_A, Color(random(0, 360), 100, 100, HSB_MODE), 1000, &mask_A);
		newCircle(&matrix_B, Color(random(0, 360), 100, 100, HSB_MODE), 1000, &mask_B);
    didIntro = true;
    sinceIntro = 0;
	}



  updateBeams();
	updateCircles();

  if (sinceIntro > 0 && didIntro){
    Color c = Color(204, 57, 90, HSB_MODE);
    if(sinceBeam1 > BEAM_FREQUENCY_INTERVAL_1-freq_offset){
      sinceBeam1 = 0;
      newBeam(&synapse_A, random(0, 2), c, random (2, 10), random(400+speed_offset, 900+speed_offset));
      newBeam(&synapse_B, random(0, 2), c, random (2, 10), random(400+speed_offset, 900+speed_offset));
      freq_offset = max(freq_offset - 50, 0); 
      speed_offset = constrain(speed_offset + 200, 0, 800);
    }
  }

  if (sinceIntro > 3500 && didIntro){
    regular_setup();
    isRegularAnimation = true;
  }
}




void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  delay(SETUP_DELAY);

  updateRandomSeed();

  rtc_set(0);
  
  readAnimationType();

  screen_A = Screen(&fg_automaton_A, &bg_automaton_A, &grow_automaton_A, &sat_automaton_A, &matrix_A, &mask_A, &background_A);
  screen_B = Screen(&fg_automaton_B, &bg_automaton_B, &grow_automaton_B, &sat_automaton_B, &matrix_B, &mask_B, &background_B);

  

  synapse_A.begin();
  synapse_B.begin();

  matrix_A.begin();
  matrix_B.begin();
  
 
  if(isRegularAnimation){
    regular_setup();
  } 


  sinceBeam1 = 0;
  sinceBeam2 = 0;
  sinceBeamUpdate = 0;
  sinceCircleUpdate = 0;
}


void loop() {
  readMessages();

  if (isRegularAnimation) regular_animation();
  else ceremony_animation();
}



