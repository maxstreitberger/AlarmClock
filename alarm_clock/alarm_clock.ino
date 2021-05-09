#include <DS3231.h>
#include <FastLED.h>
#include <CapacitiveSensor.h>
#include "pitches.h"

DS3231 clock;
RTCDateTime dt;
CapacitiveSensor capSensor = CapacitiveSensor(8, 7);

const int threshold = 1000;

unsigned long currentMillis;
unsigned long previousTime = 0;
const int intervalButton = 175;

const int dataPin  = 4; 
const int latchPin = 5;  
const int clockPin = 6;   

byte displayDigits[] = { 0, 0, 0, 0 };

// LED ring
const int NUM_LEDS = 24;
const int DATA_PIN = 2;
CRGB leds[NUM_LEDS];

long int current_hour = 0;
long int current_minute = 0;
long int current_second = 0;

long int wake_up_hour = 0;
long int wake_up_minute = 0;
long int wake_up_second = 0;

long int current_time_in_seconds = 0;
long int start_time_in_seconds = 0;
long int wake_up_time_in_seconds = 0;
long int time_to_midnight = 86400;

byte seven_seg_digits[11] = {
                          B00111111,
                          B00000110,
                          B01011011,
                          B01001111,
                          B01100110,
                          B01101101,
                          B01111101,
                          B00000111,
                          B01111111,
                          B01101111,
                          B00000000
};

//Save Button
int sButton = 13;
int start_mode = 1;
int reset_mode = 0;
unsigned long previousSBMillis;
int previousSBState = LOW;

//Up Button
int uButton = 11;
int previousUBState = LOW;
unsigned long previousUBMillis;

//Down Button
int dButton = 12;
int previousDBState = LOW;
unsigned long previousDBMillis;

//Switch Between Hour and Minute
int switchButton = 10;
int minutes_selected = 1;
int hours_selected = 0;
unsigned long previousSwitchBMillis;
int previousSwitchBState = LOW;

// Editing
bool isEditing = false;
int editing_led = A0;
int hours_led = A1;
int minutes_led = A2;
int armed_led = A3;

//Show time
bool isMidnight = false;
bool wake_up = false;
bool alarm_armed = false;

//Wake up melody
int melody[] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6};
int duration = 500;
int currentNote = 0;

void setup() {
  Serial.begin(9600);
  clock.begin();
  
  //clock.setDateTime(__DATE__, __TIME__);
  //clock.setDateTime(2021, 5, 5, 23, 58, 0);
  
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  pinMode(sButton, INPUT);
  pinMode(uButton, INPUT);
  pinMode(dButton, INPUT);
  pinMode(switchButton, INPUT);

  pinMode(editing_led, OUTPUT);
  pinMode(hours_led, OUTPUT);
  pinMode(minutes_led, OUTPUT);
  pinMode(armed_led, OUTPUT);
  
  digitalWrite(editing_led, LOW);
  digitalWrite(hours_led, LOW);
  digitalWrite(minutes_led, LOW);
  digitalWrite(armed_led, LOW);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); 
  FastLED.clear();
  FastLED.show();
  calculateSeconds();
}

void loop() {
  currentMillis = millis();
  if (!wake_up) {
    if (!isEditing) {
      digitalWrite(editing_led, LOW);
      digitalWrite(hours_led, LOW);
      digitalWrite(minutes_led, LOW);
      displayDigits[0] = current_hour / 10;
      displayDigits[1] = current_hour % 10;
      displayDigits[2] = current_minute / 10;
      displayDigits[3] = current_minute % 10;
    } else {
      if (minutes_selected) {
        digitalWrite(hours_led, LOW);
        digitalWrite(minutes_led, HIGH);
      } else {
        digitalWrite(hours_led, HIGH);
        digitalWrite(minutes_led, LOW);
      }
       
      digitalWrite(editing_led, HIGH);
    }
    
    long sensorValue = capSensor.capacitiveSensor(40);
  
    if ((sensorValue > threshold) || isEditing || start_mode) {
      DisplaySegments(); 
    }
  
    calculateSeconds();
    ProcessRing();

    if (isEditing) {
      switchBetweenHourAndMinute(); 
    } 
  } else {
    wakeUpMode();
  }

  checkSaveButton();
  checkUpButton();
  checkDownButton();

  if (alarm_armed) {
    digitalWrite(armed_led, HIGH);
  } else {
    digitalWrite(armed_led, LOW);
  }
}

void calculateSeconds() {
  dt = clock.getDateTime();
  current_hour = dt.hour;
  current_minute = dt.minute;
  current_second = dt.second;
  long int hour_minute = current_hour * 60;
  long int minute_second = current_minute * 60;
  current_time_in_seconds = (hour_minute * 60) + minute_second + dt.second;

  if (!isMidnight) {
    checkIfMidnight();
  } else {
    current_time_in_seconds += time_to_midnight;
  }
}

void checkIfMidnight() {
  if (current_time_in_seconds == 86399) {
    isMidnight = true;
    Serial.println("Midnight");
  }
}

void checkSaveButton() {
  if (currentMillis - previousSBMillis > intervalButton) {
    int sBState = digitalRead(sButton);

    if (sBState != previousSBState) {
      if (start_mode == 1 && sBState == 0) {
        reset_mode = 1;
        start_mode = 0;
        
        Serial.println("Saved!");
        wake_up_hour += dt.hour;
        wake_up_minute += dt.minute;
  
        while (wake_up_minute >= 60) {
          wake_up_minute -= 60;
          wake_up_hour += 1;
        }
  
        while (wake_up_minute < 0) {
          wake_up_minute = 60 + wake_up_minute;
          wake_up_hour -= 1;
        }
        
        wake_up_second = dt.second;
        
        Serial.print("Hour: ");
        Serial.print(wake_up_hour);
        Serial.print(", Minute: ");
        Serial.println(wake_up_minute);
  
        long int hour_minute = wake_up_hour * 60;
        long int minute_second = wake_up_minute * 60;
        wake_up_time_in_seconds = (hour_minute * 60) + minute_second + dt.second;
        start_time_in_seconds = current_time_in_seconds;
        FastLED.clear();
        FastLED.show();
  
        isEditing = false;
        alarm_armed = true;
        wake_up_hour = 0;
        wake_up_minute = 0;
        
      } else if (reset_mode == 1 && sBState == 0) {
        reset_mode = 0;
        start_mode = 1;
        wake_up_time_in_seconds = 0;
        start_time_in_seconds = 0;
        FastLED.clear();
        FastLED.show();
        wake_up_hour = 0;
        wake_up_minute = 0;
        isMidnight = false;
        wake_up = false;
        alarm_armed = false;
        noTone(9);
        Serial.println(reset_mode);
      }
    } 
    previousSBState = sBState;
    previousSBMillis = currentMillis;
  }
}

void checkUpButton() {
  if (currentMillis - previousUBMillis > intervalButton) {
    int uBState = digitalRead(uButton);
    if (uBState == HIGH) {
      Serial.println("Up");
      isEditing = true;
  
      if (minutes_selected) {
        wake_up_minute += 1;
    
        if (wake_up_minute == 60) {
          wake_up_minute = 0;
          wake_up_hour += 1;
        }      
      } else { 
        if (wake_up_hour < 99) {
          wake_up_hour += 1; 
        }
      }
  
      displayDigits[0] = wake_up_hour / 10;
      displayDigits[1] = wake_up_hour % 10;
      displayDigits[2] = wake_up_minute / 10;
      displayDigits[3] = wake_up_minute % 10;
    }
    previousUBMillis = currentMillis;
  }
}

void checkDownButton() {
  if (currentMillis - previousDBMillis > intervalButton) {
    int dBState = digitalRead(dButton);
    if (dBState == HIGH) {
      Serial.println("Down");
      isEditing = true;
  
      if (minutes_selected) {
        wake_up_minute -= 1;
        if (wake_up_hour != 0) {
          if (wake_up_minute < 0) {
            wake_up_minute = 59;
            wake_up_hour -= 1; 
          } 
        } else {
          if (wake_up_minute < 0) {
            wake_up_minute = 0;
          }
        }
      } else {
        if (wake_up_hour != 0) {
          wake_up_hour -= 1; 
        }
      }
      
      displayDigits[0] = wake_up_hour / 10;
      displayDigits[1] = wake_up_hour % 10;
      displayDigits[2] = wake_up_minute / 10;
      displayDigits[3] = wake_up_minute % 10;
    }
    
    previousDBMillis = currentMillis;
  }
}

void switchBetweenHourAndMinute() {
  if (currentMillis - previousSwitchBMillis > intervalButton) {
    int switchButtonState = digitalRead(switchButton);
    
    if (switchButtonState != previousSwitchBState) {
      if (minutes_selected == 1 && switchButtonState == 0) {
        hours_selected = 1;
        minutes_selected = 0;
      } else if (hours_selected == 1 && switchButtonState == 0) {
        hours_selected = 0;
        minutes_selected = 1;
      }
    }  
   
    previousSwitchBState = switchButtonState;
    previousSwitchBMillis = currentMillis;
  }
}

void ProcessRing() {
  if (wake_up_time_in_seconds != 0) {
    int onLEDs = map(current_time_in_seconds, start_time_in_seconds, wake_up_time_in_seconds, 0, NUM_LEDS);
    //Serial.println(onLEDs);
    if (onLEDs != 0) {
      leds[onLEDs-1] = CHSV(160, 255, 75); 
      FastLED.show();  
    }
  }
  
  if ((wake_up_time_in_seconds == current_time_in_seconds) && (wake_up_time_in_seconds != 0)) {
    wake_up = true;
    wake_up_time_in_seconds = 0;
    start_time_in_seconds = 0;
  }
}

void wakeUpMode() {
  for (int i = 0; i < NUM_LEDS; i++) {
   leds[i] = CHSV(0, 255, 50); 
  }
  FastLED.show();

  currentNote += 1;
  if (currentNote == 8) {
    currentNote = 0;
  }

  tone(9, melody[currentNote], 1000);
  delay(1000);
}

void DisplaySegments() {
  int counter = 1;
  for (int x = 0; x < 4; x++) {
    digitalWrite(latchPin, 0);
    shiftOut(0);
    shiftOut(0);
    digitalWrite(latchPin, 1);

    digitalWrite(latchPin, 0);
    shiftOut(seven_seg_digits[displayDigits[x]]); // Second shift register
    shiftOut(counter); // First shift register
    digitalWrite(latchPin, 1);
    counter *= 2;
    delay(1);                              // 1 or 2 is ok
  }
  digitalWrite(latchPin, 0);
  shiftOut(0);
  shiftOut(0);
  digitalWrite(latchPin, 1);
}

void shiftOut(byte myDataOut) {
  int i = 0;
  int pinState;
  
  digitalWrite(dataPin, 0);
  digitalWrite(clockPin, 0);

  for (i = 7; i >= 0; i--)  {
    digitalWrite(clockPin, 0);

    if ( myDataOut & (1 << i) ) {
      pinState = 1;
    }
    else {
      pinState = 0;
    }

    digitalWrite(dataPin, pinState);
    digitalWrite(clockPin, 1);
    digitalWrite(dataPin, 0);
  }

  digitalWrite(clockPin, 0);
}
