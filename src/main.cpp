#include "NewPing.h"
#include <Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define TRIGGER_PIN_1 7
#define ECHO_PIN_1 8

#define TRIGGER_PIN_2 6
#define ECHO_PIN_2 5

#define MAX_DISTANCE 50
#define THRESHOLD 5

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C 

#define LED_PIN 4  

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
NewPing sonar_1(TRIGGER_PIN_1, ECHO_PIN_1, MAX_DISTANCE);
NewPing sonar_2(TRIGGER_PIN_2, ECHO_PIN_2, MAX_DISTANCE);

Servo myServo; 

int current_position = 90;
unsigned long lastMovementTime = 0;      
const unsigned long returnToCenterDelay = 10000; 

unsigned long previousMillisServo = 0;
unsigned long previousMillisWink = 0;
const long intervalServo = 100;  
const long intervalWink = 1500;  

bool isWinking = false;
bool isMoving = false;

void setup() {
  Serial.begin(9600);

  myServo.attach(9); 
  myServo.write(90);

  pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN, LOW); 

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }

  display.display();
  delay(2000);

  // Clear the buffer
  display.clearDisplay();

  drawFace(false);
  delay(1000);  
}

void loop() {
  unsigned long currentMillis = millis();

 
  if (currentMillis - previousMillisServo >= intervalServo) {
    previousMillisServo = currentMillis;

    int distance1 = sonar_1.ping_cm(); 
    int distance2 = sonar_2.ping_cm(); 

    if (distance1 == 0 || distance1 >= MAX_DISTANCE) {
      distance1 = MAX_DISTANCE;
    }
    if (distance2 == 0 || distance2 >= MAX_DISTANCE) {
      distance2 = MAX_DISTANCE;
    }

    if (abs(distance1 - distance2) < THRESHOLD) {
      Serial.println("Object is in the center or too close to differentiate.");
      isMoving = false;
    } else if (distance1 < distance2) {
      Serial.println("Object is on the LEFT side.");

      current_position += 5; 
      if (current_position > 180) {
        current_position = 180;  
      }
      
      myServo.write(current_position);
      lastMovementTime = currentMillis;  
      isMoving = true; 
      
    } else {
      Serial.println("Object is on the RIGHT side.");

      current_position -= 5;  
      if (current_position < 0) {
        current_position = 0;  
      }

      myServo.write(current_position);
      lastMovementTime = currentMillis;  
      isMoving = true;
    }
  }

  if (currentMillis - lastMovementTime >= returnToCenterDelay) {
    if (current_position != 90) { 
      Serial.println("Returning to center position.");
      current_position = 90;
      myServo.write(current_position);
      isMoving = false;
    }
  }

  if (isMoving) {
    digitalWrite(LED_PIN, HIGH); 
  } else {
    digitalWrite(LED_PIN, LOW); 
  }

  if (currentMillis - previousMillisWink >= intervalWink) {
    previousMillisWink = currentMillis;
    isWinking = !isWinking; 
    drawFace(isWinking);
  }
}

void drawFace(bool isWinking) {
  display.clearDisplay();

  int eyeRadius = 8;
  int eyeXOffset = 30;
  int eyeYOffset = 20;
  int eyeLashLength = 5;

  int mouthWidth = 40;
  int mouthHeight = 10;
  int mouthX = (SCREEN_WIDTH - mouthWidth) / 2;
  int mouthY = 50;

  if (isWinking) {
    display.drawLine(eyeXOffset - eyeRadius, eyeYOffset, eyeXOffset + eyeRadius, eyeYOffset, SSD1306_WHITE);
  } else {
    display.fillCircle(eyeXOffset, eyeYOffset, eyeRadius, SSD1306_WHITE);
    display.fillCircle(eyeXOffset, eyeYOffset, eyeRadius - 4, SSD1306_BLACK); 

    display.drawLine(eyeXOffset - eyeRadius, eyeYOffset - eyeRadius, eyeXOffset - eyeRadius + eyeLashLength, eyeYOffset - eyeRadius - eyeLashLength, SSD1306_WHITE);
    display.drawLine(eyeXOffset, eyeYOffset - eyeRadius - 2, eyeXOffset, eyeYOffset - eyeRadius - eyeLashLength, SSD1306_WHITE);
    display.drawLine(eyeXOffset + eyeRadius, eyeYOffset - eyeRadius, eyeXOffset + eyeRadius - eyeLashLength, eyeYOffset - eyeRadius - eyeLashLength, SSD1306_WHITE);
  }

  display.fillCircle(SCREEN_WIDTH - eyeXOffset, eyeYOffset, eyeRadius, SSD1306_WHITE);
  display.fillCircle(SCREEN_WIDTH - eyeXOffset, eyeYOffset, eyeRadius - 4, SSD1306_BLACK);

  display.drawLine(SCREEN_WIDTH - eyeXOffset - eyeRadius, eyeYOffset - eyeRadius, SCREEN_WIDTH - eyeXOffset - eyeRadius + eyeLashLength, eyeYOffset - eyeRadius - eyeLashLength, SSD1306_WHITE);
  display.drawLine(SCREEN_WIDTH - eyeXOffset, eyeYOffset - eyeRadius - 2, SCREEN_WIDTH - eyeXOffset, eyeYOffset - eyeRadius - eyeLashLength, SSD1306_WHITE);
  display.drawLine(SCREEN_WIDTH - eyeXOffset + eyeRadius, eyeYOffset - eyeRadius, SCREEN_WIDTH - eyeXOffset + eyeRadius - eyeLashLength, eyeYOffset - eyeRadius - eyeLashLength, SSD1306_WHITE);

  display.drawLine(mouthX, mouthY, mouthX + mouthWidth / 2, mouthY + mouthHeight, SSD1306_WHITE);
  display.drawLine(mouthX + mouthWidth / 2, mouthY + mouthHeight, mouthX + mouthWidth, mouthY, SSD1306_WHITE);

  display.display();
}
