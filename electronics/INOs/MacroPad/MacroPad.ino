#include <Keyboard.h>

#define BUTTON1 4
#define BUTTON2 5
#define BUTTON3 6
#define BUTTON4 7

#define MODE_BUTTON 2


void setup() {
  // put your setup code here, to run once:

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON4, INPUT_PULLUP);

  
  pinMode(MODE_BUTTON, INPUT_PULLUP);

  Serial.begin(9600);

  Keyboard.begin();

}

void openArduino()
{
  Serial.println("Arduino Opened");
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('a');
  Keyboard.releaseAll();
  delay(1000);
}

void openNotion()
{
  Serial.println("Notion Opened");
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('n');
  Keyboard.releaseAll();
  delay(1000);
}

void openFifa()
{}

void openDiscord()
{}

void loop() {
  // put your main code here, to run repeatedly:

  if(digitalRead(BUTTON1) == 0) openArduino();
  if(digitalRead(BUTTON2) == 0) openNotion();
  if(digitalRead(BUTTON3) == 0) openFifa();
  if(digitalRead(BUTTON4) == 0) openNotion();

  delay(200);

}