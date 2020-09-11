#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

//#define REMOTE_PIN 7 // TO BE USED IN FINAL FORM

#define REMOTE_PIN 7

#include <NewRemoteReceiver.h>


int relaisPins[5] = {18, 19, 22, 21, 20};
bool onOffStateGoal = false;
int relaisOnAm = 0;
elapsedMillis sinceRelais = 0;
elapsedMillis sinceTimePrint = 0;
int previous_Dim = -1;

void setup() {
  setSyncProvider(getTeensy3Time);
  adjustTime(-3600);


  NewRemoteReceiver::init(REMOTE_PIN, 2, sendSignal);
  Serial1.begin(9600);

  Serial.println(getTeensy3Time());

  for (int i = 0; i < sizeof(relaisPins) / sizeof(relaisPins[0]); i++) {
    pinMode(relaisPins[i], OUTPUT);
  }
  Alarm.alarmRepeat(dowMonday, 7, 0, 0, turnOn);
  Alarm.alarmRepeat(dowMonday, 18, 0, 0, turnOff);

  Alarm.alarmRepeat(dowTuesday, 7, 0, 0, turnOn);
  Alarm.alarmRepeat(dowTuesday, 18, 0, 0, turnOff);

  Alarm.alarmRepeat(dowWednesday, 7, 0, 0, turnOn);
  Alarm.alarmRepeat(dowWednesday, 18, 0, 0, turnOff);

  Alarm.alarmRepeat(dowThursday, 7, 0, 0, turnOn);
  Alarm.alarmRepeat(dowThursday, 18, 0, 0, turnOff);

  Alarm.alarmRepeat(dowFriday, 7, 0, 0, turnOn);
  Alarm.alarmRepeat(dowFriday, 18, 0, 0, turnOff);

 
  
}

elapsedMillis timerStartMessage;
bool sentModeMess = false;
String modeMessage;

void loop() {
  Alarm.delay(0);
  if (onOffStateGoal && relaisOnAm < 5 && sinceRelais > 500) {
    digitalWrite(relaisPins[relaisOnAm], HIGH);
    relaisOnAm++;
    sinceRelais = 0;
  } else if (!onOffStateGoal && relaisOnAm > 0) {
    for (int i = 0; i < sizeof(relaisPins) / sizeof(relaisPins[0]); i++) {
      digitalWrite(relaisPins[i], LOW);
    }
    relaisOnAm = 0;
  }

  if (onOffStateGoal && timerStartMessage > 3500 && !sentModeMess) {
    sentModeMess = true;
    Serial1.print(modeMessage);
  }

}



time_t getTeensy3Time() {
  return Teensy3Clock.get();
}




void turnOn() {
  if (!onOffStateGoal) {
    onOffStateGoal = true;
    timerStartMessage = 0;
    sentModeMess = false;
    modeMessage = "reg";
    Serial.println("We're on");
  }
}

void turnOff() {
  onOffStateGoal = false;
  Serial.println("We're off");
}


void turnOnSpecial() {
  if (!onOffStateGoal) {
    
    onOffStateGoal = true;
    timerStartMessage = 0;
    sentModeMess = false;
    modeMessage = "spec";
    Serial.println("We're on Ceremony!!!!!");
  }
}

void turnOffSpecial() {
  onOffStateGoal = false;
  Serial.println("We're off");
}


void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void turnOnPod1(){
  digitalWrite(18, HIGH);
}

void turnOffPod1(){
  digitalWrite(18, LOW);
}

void sendSignal(NewRemoteCode receivedCode) {
  //Change address if you get new remote.
  //Serial.println("Addr: ");
  //Serial.println(receivedCode.address);
  if (receivedCode.address == 54414440 || receivedCode.address == 53901904 || receivedCode.address == 52079696) {
    if (receivedCode.groupBit) {
      switch (receivedCode.switchType) {
        case NewRemoteCode::off:
          turnOffSpecial();
          break;
        case NewRemoteCode::on:
          turnOnSpecial();
          //turnOnPod1();
          break;
      }
    }
    else {
      switch (receivedCode.switchType) {
        case NewRemoteCode::off:
          turnOff();
          break;
        case NewRemoteCode::on:
          turnOn();
          //turnOnPod1();
          break;
      }
      if (receivedCode.dimLevelPresent) {
        if (previous_Dim == -1) {
          previous_Dim = receivedCode.dimLevel;
        } else if (receivedCode.dimLevel > previous_Dim ) {
          previous_Dim = receivedCode.dimLevel;
          Serial.println("Increase brightness");
          Serial1.write("incb");
          Serial.println(", dim level: ");
          Serial.print(receivedCode.dimLevel);
        } else if (receivedCode.dimLevel < previous_Dim) {
          previous_Dim = receivedCode.dimLevel;
          Serial.println("Decrease brightness");
          Serial1.write("decb");
          Serial.println(", dim level: ");
          Serial.print(receivedCode.dimLevel);
        } else if (receivedCode.dimLevel == previous_Dim) {
          if (previous_Dim == 0) {
            Serial.println("Decrease brightness");
            Serial1.write("decb");
            Serial.println(", dim level: ");
            Serial.print(receivedCode.dimLevel);
          } else if (previous_Dim == 15) {
            Serial.println("Increase brightness");
            Serial1.write("incb");
            Serial.println(", dim level: ");
            Serial.print(receivedCode.dimLevel);
          }
        }
      }
    }

  }
}
