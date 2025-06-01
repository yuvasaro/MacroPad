
// This example renders a png file that is stored in a FLASH array
// using the PNGdec library (available via library manager).

// Note: The PNGDEC required lots of RAM to work (~40kbytes) so
// this sketch is will not run on smaller memory processors (e.g.
// ESP8266, STM32F103 etc.)

// Image files can be converted to arrays using the tool here:
// https://notisrac.github.io/FileToCArray/
// To use this tool:
//   1. Drag and drop file on "Browse..." button
//   2. Tick box "Treat as binary"
//   3. Click "Convert"
//   4. Click "Save as file" and move the header file to sketch folder
//   5. Open the sketch in IDE
//   6. Include the header file containing the array (panda.h in this example)

// Include the PNG decoder library
#include <PNGdec.h>
#include "images.h" // Image is stored here in an 8-bit array

PNG png; // PNG decoder instance

#define MAX_IMAGE_WIDTH 64 // Adjust for your images

int16_t xpos = 0;
int16_t ypos = 0;

// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include "SPI.h"
#include <TFT_eSPI.h>              // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

//====================================================================================
//                                    Setup
//====================================================================================

// SDA = 23, SCL = 18, RES = 4, DC = 2

const uint8_t screenCS[6] = { 15, 5, 32, 33, 27, 14};   // chip‑select pins for the two screens
const uint8_t NUM_SCREENS = sizeof(screenCS) / sizeof(screenCS[0]);

const uint8_t buttons[6] = { 12, 19, 35, 36, 39, 34 };//{ 12, 19, 17, 16, 25, 26 };

#define ENC0_A 22
#define ENC0_B 17 // need new pin for enc0_B
#define ENC0_PUSH 21

#define ENC1_A 16
#define ENC1_B 25
#define ENC1_PUSH 26

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n Using the PNGdec library");

  for (uint8_t i = 0; i < NUM_SCREENS; ++i) {
    pinMode(screenCS[i], OUTPUT);    // make it a push‑pull output
    digitalWrite(screenCS[i], LOW);  // keep the display un‑selected or in reset

    pinMode(buttons[i], INPUT_PULLUP);
  }

  // Initialise the TFT
  tft.begin();
  tft.fillScreen(TFT_BLACK);

  Serial.println("\r\nInitialisation done.");

}

//=========================================v==========================================
//                                      pngDraw
//====================================================================================
// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
// Callback function to draw pixels to the display
void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

//====================================================================================
//                                    Loop
//====================================================================================

boolean dset = false;
int profile = 0;

#define NUM_PROFILES 6
#define ICONS_PER_PROFILE 9

// pointer table + sizes
const uint8_t* const icons[NUM_PROFILES][ICONS_PER_PROFILE]   = {{ p0i0, p0i1, p0i2, p0i3, p0i4, p0i5, p0i6, p0i7, p0i8 },
                                                                 { p1i0, p1i1, p1i2, p1i3, p1i4, p1i5, p1i6, p1i7, p1i8 },
                                                                 { p2i0, p2i1, p2i2, p2i3, p2i4, p2i5, p2i6, p2i7, p2i8 },
                                                                 { p3i0, p3i1, p3i2, p3i3, p3i4, p3i5, p3i6, p3i7, p3i8 },
                                                                 { p4i0, p4i1, p4i2, p4i3, p4i4, p4i5, p4i6, p4i7, p4i8 },
                                                                 { p5i0, p5i1, p5i2, p5i3, p5i4, p5i5, p5i6, p5i7, p5i8 }
                                                                };
const size_t         iconSize[NUM_PROFILES][ICONS_PER_PROFILE] = {{ sizeof(p0i0), sizeof(p0i1), sizeof(p0i2), sizeof(p0i3), sizeof(p0i4), sizeof(p0i5), sizeof(p0i6), sizeof(p0i7), sizeof(p0i8) },
                                                                  { sizeof(p1i0), sizeof(p1i1), sizeof(p1i2), sizeof(p1i3), sizeof(p1i4), sizeof(p1i5), sizeof(p1i6), sizeof(p1i7), sizeof(p1i8) },
                                                                  { sizeof(p2i0), sizeof(p2i1), sizeof(p2i2), sizeof(p2i3), sizeof(p2i4), sizeof(p2i5), sizeof(p2i6), sizeof(p2i7), sizeof(p2i8) },
                                                                  { sizeof(p3i0), sizeof(p3i1), sizeof(p3i2), sizeof(p3i3), sizeof(p3i4), sizeof(p3i5), sizeof(p3i6), sizeof(p3i7), sizeof(p3i8) },
                                                                  { sizeof(p4i0), sizeof(p4i1), sizeof(p4i2), sizeof(p4i3), sizeof(p4i4), sizeof(p4i5), sizeof(p4i6), sizeof(p4i7), sizeof(p4i8) },
                                                                  { sizeof(p5i0), sizeof(p5i1), sizeof(p5i2), sizeof(p5i3), sizeof(p5i4), sizeof(p5i5), sizeof(p5i6), sizeof(p5i7), sizeof(p5i8) }
                                                                 };

const uint8_t DEBOUNCE_MS = 40;
bool lastState[6]   = {1,1,1,1,1,1};   // start HIGH (internal pull-ups)
uint32_t lastTime[6] = {0};            // last time we saw a state change

void loop()
{
  if(Serial.available())
  {
    int n_profile = Serial.parseInt();
    if(n_profile >= 10 && n_profile <= 15)
    {
      Serial.printf("Received proflie number %d\n", n_profile);
      profile = n_profile-10;
      dset = false;
    }
  }

    // if(digitalRead(buttons[0])==LOW)
    // { 
    //   Serial.println((1));
    //   delay(150);
    // }
    // if(digitalRead(buttons[1])==LOW)
    // { 
    //   Serial.println((2));
    //   delay(150);
    // }
    // if(digitalRead(buttons[2])==LOW)
    // { 
    //   Serial.println((3));
    //   delay(150);
    // }
    // if(digitalRead(buttons[3])==LOW)
    // { 
    //   Serial.println((4));
    //   delay(150);
    // }
    // if(digitalRead(buttons[4])==LOW)
    // { 
    //   Serial.println((5));
    //   delay(150);
    // }
    // if(digitalRead(buttons[5])==LOW)
    // { 
    //   Serial.println((6));
    //   delay(150);
    // }

  for (uint8_t i = 0; i < NUM_SCREENS; ++i) {
    bool reading = digitalRead(buttons[i]);        // HIGH = released, LOW = pressed
    uint32_t now = millis();

    // Detect a *change* and make sure it has been stable for DEBOUNCE_MS
    if (reading != lastState[i] && (now - lastTime[i] > DEBOUNCE_MS)) {
      lastTime[i]  = now;
      lastState[i] = reading;

      if (!reading) {                            // LOW means newly pressed
        Serial.println(i + 1);                 // send 1-6 once per press
      }
    }
  }

  
  if(!dset)
  {
    Serial.printf("Profile %d\n", profile);
    for(uint8_t i=0;i<NUM_SCREENS; i++)
    {
      for(uint8_t j=0;j<NUM_SCREENS; j++)
      {
        if(i==j) digitalWrite(screenCS[j], LOW);
        else digitalWrite(screenCS[j], HIGH);
      }
      Serial.printf("Finished setting %dth CS\n", i);

      int16_t rc = png.openFLASH((uint8_t *)icons[profile][i], iconSize[profile][i], pngDraw);
      if (rc == PNG_SUCCESS) 
      {
        Serial.println("Successfully opened png file");
        Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
        tft.startWrite();
        uint32_t dt = millis();
        rc = png.decode(NULL, 0);
        Serial.print(millis() - dt); Serial.println("ms");
        tft.endWrite();
        png.close(); // not needed for memory->memory decode
        //delay(50);
      }
    }
    dset = true;
  }
}