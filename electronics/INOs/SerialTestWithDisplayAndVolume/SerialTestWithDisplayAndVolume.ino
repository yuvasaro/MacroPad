#include <Keyboard.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN2_ADDRESS 0x3C
#define SCREEN1_ADDRESS 0x78

//Adafruit_SSD1306 display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON1 4
#define BUTTON2 5
#define BUTTON3 6
#define BUTTON4 7

#define MODE_BUTTON 1

// Define rotary encoder pins
#define ENC_A 2
#define ENC_B 3

#define MUTEBUTTON 0

unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 10;

volatile int counter = 0;

#include "images.hpp"

void setup() 
{

  Serial.begin(9600);

  pinMode(MODE_BUTTON, INPUT_PULLUP);
  pinMode(MUTEBUTTON, INPUT_PULLUP);
  
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON4, INPUT_PULLUP);

  // display1.begin(SSD1306_SWITCHCAPVCC, 0x7A);
  // display1.clearDisplay();

  // display1.drawBitmap(0, 0, im1d1_bmp, 128, 64, WHITE);
  // display1.display();

  display2.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display2.clearDisplay();

  display2.drawBitmap(0, 0, im1d2_bmp, 128, 64, WHITE);
  display2.display();

  // Set encoder pins and attach interrupts
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);

  pinMode(MUTEBUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), read_encoder, CHANGE);

}

int profile = 0;
bool dset = true;

void loop() 
{
  //Serial.println("Hello from Arduino!");
  if(digitalRead(MODE_BUTTON) == LOW) 
  {
    //Serial.println("Profile Switched");
    profile=(profile+1)%2;
    dset = false;
    Serial.println(profile+11);
    delay(250);
  }
  if(digitalRead(BUTTON1) == LOW) 
  {
    Serial.println("1");
    delay(250);
  }
  if(digitalRead(BUTTON2) == LOW) 
  {
    Serial.println("2");
    delay(250);
  }
  if(digitalRead(BUTTON3) == LOW) 
  {
    Serial.println("3");
    delay(250);
  }
  if(digitalRead(BUTTON4) == LOW) 
  {
    Serial.println("4");
    delay(250);
  }
  if(digitalRead(MUTEBUTTON) == LOW) 
  {
    Serial.println("73");
    delay(250);
  }
  switch(profile)
  {
      case 0:
        if(!dset)
        {
          // display1.clearDisplay();
          // display1.drawBitmap(0, 0, im1d1_bmp, 128, 64, WHITE);
          // display1.display();

          display2.clearDisplay();
          display2.drawBitmap(0, 0, im1d2_bmp, 128, 64, WHITE);
          display2.display();
          dset = true;
        }
        break;
      case 1:
        if(!dset)
        {
          // display1.clearDisplay();
          // display1.drawBitmap(0, 0, im2d1_bmp, 128, 64, WHITE);
          // display1.display();

          display2.clearDisplay();
          display2.drawBitmap(0, 0, im2d2_bmp, 128, 64, WHITE);
          display2.display();
          dset = true;
        }
        break;
      default:
        break;
  }

  static int lastCounter = 0;
  static bool buttonPressed = digitalRead(MUTEBUTTON);
  int buttonPressStart = millis();
  //if(buttonPressed) Serial.println(73);

  // If count has changed print the new value to serial
  if(counter < lastCounter){
    Serial.println(71); //wiring causes this to be volume UP
    lastCounter = counter;
  }
  else if (counter > lastCounter) {
    Serial.println(72); //wiring causes this to be volume DOWN
    lastCounter = counter;
  }
}

void read_encoder() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(ENC_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(ENC_B)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    int changevalue = 1;
    if((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    int changevalue = -1;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }

}