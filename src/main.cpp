//-------NOTE: if you do clean, don't forget to fix core_pins.h for milis()


#define POD_ID 5


#define AUTOMATON_INTERVAL 600
#define FADE_INTERVAL 1000/20
#define BEAM_UPDATE_INTERVAL 1000/40
#define BEAM_FREQUENCY_INTERVAL 1500
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
int bri_level = 255;

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


elapsedMillis sinceBeam;


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

void regular_setup(){
  screen_A.regular_screen_animation();
  screen_B.regular_screen_animation();
}


void regular_animation(){
  if (!isRegularAnimation) return;
  if(sinceBeam > BEAM_FREQUENCY_INTERVAL){
    sinceBeam = 0;
    int rand1 = random(0, 2);
    int rand2 = random(0, 2);
    Color color1, color2;
    if (rand1 == 0){
      color1 = Color(random(screen_A.bottom_hue_threshold, screen_A.bottom_hue_threshold + screen_A.hue_difference), random(75, 101), random(85, 100), HSB_MODE);
    } else {
      color1 = Color(random(0,360), random(75, 101), 100, HSB_MODE);
    }

    if (rand2 == 0){
      color2 = Color(random(screen_B.bottom_hue_threshold, screen_B.bottom_hue_threshold + screen_B.hue_difference), random(75, 101), random(85, 100), HSB_MODE);
    } else {
      color2 = Color(random(0,360), random(75, 101), 100, HSB_MODE);
    }
    newBeam(&synapse_A, rand1, color1, random(3, 30),random(1000,6000));
    newBeam(&synapse_B, rand2, color2, random(3, 30),random(1000,6000));
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

void ceremony_setup(){
  screen_A.special_screen_animation();
  screen_B.special_screen_animation();
  implosionTimer = 0;
}

void updateScreensFadeCeremony(int rate) {
  if(screenFadeUpdateSwitcher){
    if(screen_A.updateFadeCeremony(0, rate)) screenFadeUpdateSwitcher = !screenFadeUpdateSwitcher;
  }else{
    if(screen_B.updateFadeCeremony(1, rate))  screenFadeUpdateSwitcher = !screenFadeUpdateSwitcher;
  }
}


void renderInterruptCeremony(int rate) {
  updateBeams();
  updateScreensFadeCeremony(400);
}

int automatonRate = 700;
void ceremony_animation(){

  if (isRegularAnimation) return;
    
  if (implosionTimer > 2500){
    implosionDone = true;

  }

  if (!implosionDone){
  screen_A.iterate_animation();
  screen_B.iterate_animation();
 
  renderInterruptCeremony(700);
  } else {
    if(automatonReinitializeCeremony){
        screen_A.regular_screen_animation();
        screen_A.regular_screen_animation();
        automatonReinitializeCeremony = false;
      }
    screen_A.iterate_animation();
    screen_B.iterate_animation();
    automatonRate = min((automatonRate - 50), 0);
    renderInterruptCeremony(automatonRate);
    if (automatonRate == 0){
      isRegularAnimation = true;
    }  
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

  screen_A.init_screen();
  screen_B.init_screen();

  synapse_A.begin();
  synapse_B.begin();
  

  if(isRegularAnimation){
    regular_setup();
  } else {
    ceremony_setup();
  }


  sinceBeam = 0;
  sinceBeamUpdate = 0;
}


void loop() {
  readMessages();
  regular_animation();
  ceremony_animation();
}



