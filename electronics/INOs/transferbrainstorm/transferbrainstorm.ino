
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

const uint8_t buttons[6] = { 12, 19, 35, 36, 39, 34 };//{ 12, 19, 17, 16, 25, 26 };

const uint8_t DEBOUNCE_MS = 40;
bool lastState[6]   = {1,1,1,1,1,1};   // start HIGH (internal pull-ups)
uint32_t lastTime[6] = {0};            // last time we saw a state change

#define ENC0_A 22
#define ENC0_B 17 // need new pin for enc0_B
#define ENC0_PUSH 21

#define ENC1_A 16
#define ENC1_B 25
#define ENC1_PUSH 26

unsigned long _lastIncReadTime0 = micros(); 
unsigned long _lastDecReadTime0 = micros(); 
unsigned long _lastIncReadTime1 = micros(); 
unsigned long _lastDecReadTime1 = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 10;

volatile int counter0 = 0;
volatile int counter1 = 0;

// SDA = 23, SCL = 18, RES = 4, DC = 2

const uint8_t screenCS[6] = { 15, 5, 32, 33, 27, 14};   // chip‑select pins for the two screens
const uint8_t NUM_SCREENS = sizeof(screenCS) / sizeof(screenCS[0]);

boolean dset = false;
int profile = 0;

#define NUM_PROFILES 6
#define ICONS_PER_PROFILE 6

// pointer table + sizes
uint8_t* icons[NUM_PROFILES][ICONS_PER_PROFILE]   = {{ p0i0, p0i1, p0i2, p0i3, p0i4, p0i5 },
                                                                 { p1i0, p1i1, p1i2, p1i3, p1i4, p1i5 },
                                                                 { p2i0, p2i1, p2i2, p2i3, p2i4, p2i5 },
                                                                 { p3i0, p3i1, p3i2, p3i3, p3i4, p3i5 },
                                                                 { p4i0, p4i1, p4i2, p4i3, p4i4, p4i5 },
                                                                 { p5i0, p5i1, p5i2, p5i3, p5i4, p5i5 }
                                                                };
size_t         iconSize[NUM_PROFILES][ICONS_PER_PROFILE] = {{ sizeof(p0i0), sizeof(p0i1), sizeof(p0i2), sizeof(p0i3), sizeof(p0i4), sizeof(p0i5) },
                                                                  { sizeof(p1i0), sizeof(p1i1), sizeof(p1i2), sizeof(p1i3), sizeof(p1i4), sizeof(p1i5) },
                                                                  { sizeof(p2i0), sizeof(p2i1), sizeof(p2i2), sizeof(p2i3), sizeof(p2i4), sizeof(p2i5) },
                                                                  { sizeof(p3i0), sizeof(p3i1), sizeof(p3i2), sizeof(p3i3), sizeof(p3i4), sizeof(p3i5) },
                                                                  { sizeof(p4i0), sizeof(p4i1), sizeof(p4i2), sizeof(p4i3), sizeof(p4i4), sizeof(p4i5) },
                                                                  { sizeof(p5i0), sizeof(p5i1), sizeof(p5i2), sizeof(p5i3), sizeof(p5i4), sizeof(p5i5) }
                                                                 };

#include <esp_system.h>

static const size_t MAX_ICON_BYTES = 64*64*3;
static uint8_t iconsRam[NUM_PROFILES][ICONS_PER_PROFILE][MAX_ICON_BYTES];
static size_t iconRamSize[NUM_PROFILES][ICONS_PER_PROFILE];

void initIconRam()
{
  for(uint8_t p=0; p<NUM_PROFILES; ++p){
    for(uint8_t i=0; i<ICONS_PER_PROFILE; ++i){
      // copy initial flash data into RAM
      size_t sz = iconSize[p][i];
      if(sz > MAX_ICON_BYTES) sz = MAX_ICON_BYTES;
      memcpy(iconsRam[p][i], icons[p][i], sz);
      iconRamSize[p][i] = sz;
    }
  }
}

//====================================================================================
//                                    Setup
//====================================================================================
void setup()
{
  Serial.begin(115200);
  //Serial.println("\n\n Using the PNGdec library");

  for (uint8_t i = 0; i < NUM_SCREENS; ++i) {
    pinMode(screenCS[i], OUTPUT);    // make it a push‑pull output
    digitalWrite(screenCS[i], LOW);  // keep the display un‑selected or in reset

    pinMode(buttons[i], INPUT_PULLUP);
  }

  pinMode(ENC0_PUSH, INPUT_PULLUP);
  pinMode(ENC1_PUSH, INPUT_PULLUP);

  pinMode(ENC0_A, INPUT_PULLUP);
  pinMode(ENC0_B, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(ENC0_A), read_encoder0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC0_B), read_encoder0, CHANGE);

  pinMode(ENC1_A, INPUT_PULLUP);
  pinMode(ENC1_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC1_A), read_encoder1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC1_B), read_encoder1, CHANGE);



  // Initialise the TFT
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  initIconRam();

  //Serial.println("\r\nInitialisation done.");

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
void loop()
{

 bool profileMode = false;

  //Receiving current profile number from Qt App
  if(Serial.available())
  {
    int n_profile = Serial.parseInt();
    if(n_profile >= 10 && n_profile <= 15)
    {
      //Serial.printf("Received proflie number %d\n", n_profile);
      profile = n_profile-10;
      dset = false;
    }

    if(n_profile == 47)
    {
      profileMode = true;
    }

  }

  //Detecting Key Presses

  if(!profileMode)
  {
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

    // Encoder Detection

    //ENC0 - Left
    if(digitalRead(ENC0_PUSH) == LOW)
    {
      Serial.println(73);
      delay(200);
    }
    static int lastCounter0 = 0;
    if(counter0 < lastCounter0){
      Serial.println(71); 
      lastCounter0 = counter0;
    }
    else if (counter0 > lastCounter0) {
      Serial.println(72); 
      lastCounter0 = counter0;
    }

    //ENC1 - Right
    if(digitalRead(ENC1_PUSH) == LOW)
    {
      Serial.println(83);
      delay(200);
    }
    static int lastCounter1 = 0;
    if(counter1 < lastCounter1){
      Serial.println(81);
      lastCounter1 = counter1;
    }
    else if (counter1 > lastCounter1) {
      Serial.println(82);
      lastCounter1 = counter1;
    }

    

    //Changing images according to profile
    if(!dset)
    {
      //Serial.printf("Profile %d\n", profile);
      for(uint8_t i=0;i<NUM_SCREENS; i++)
      {
        for(uint8_t j=0;j<NUM_SCREENS; j++)
        {
          if(i==j) digitalWrite(screenCS[j], LOW);
          else digitalWrite(screenCS[j], HIGH);
        }
        //Serial.printf("Finished setting %dth CS\n", i);

        int16_t rc = png.openRAM((uint8_t *)icons[profile][i], iconSize[profile][i], pngDraw); //int16_t rc = png.openFLASH((uint8_t *)icons[profile][i], iconSize[profile][i], pngDraw);
        if (rc == PNG_SUCCESS) 
        {
          // Serial.println("Successfully opened png file");
          // Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
          tft.startWrite();
          uint32_t dt = millis();
          rc = png.decode(NULL, 0);
          //Serial.print(millis() - dt); Serial.println("ms");
          tft.endWrite();
          png.close(); // not needed for memory->memory decode
          //delay(50);
        }
      }
      dset = true;
    }
  }
  else  // profileMode == true
  {
    // 1) read x,y *once* as raw bytes
    while(Serial.available() < 2) { /* wait */; }
    uint8_t x = Serial.read();     // 0x00–0x05 for profiles
    uint8_t y = Serial.read();     // 0x00–0x05 for icon slot

    // clamp just in case
    if(x >= NUM_PROFILES || y >= ICONS_PER_PROFILE){
      profileMode = false;
      continue;
    }

    // 2) clear the RAM buffer for this slot
    memset(iconsRam[x][y], 0, MAX_ICON_BYTES);

    // 3) receive until end-of-image marker (0x30 decimal = 48)
    size_t offset = 0;
    while(true){
      if(!Serial.available()) continue;
      uint8_t b = Serial.read();
      if(b == 48){           // 0x30: end of this image
        break;
      }
      if(offset < MAX_ICON_BYTES){
        iconsRam[x][y][offset++] = b;
      }
    }
    iconRamSize[x][y] = offset;

    // 4) check for final profile-exit marker (0x31 decimal = 49)
    //    Qt app should send a 0x31 after the last image
    if(Serial.peek() == 49){
      Serial.read();        // consume the 0x31
      profileMode = false;
      // now reboot to apply new icons
      ESP.restart();
    }
  }
}

//====================================================================================
//                               Encoder Helper Methods
//====================================================================================
void read_encoder0() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(ENC0_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(ENC0_B)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    int changevalue = 1;
    if((micros() - _lastIncReadTime0) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime0 = micros();
    counter0 = counter0 + changevalue;              // Update counter
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    int changevalue = -1;
    if((micros() - _lastDecReadTime0) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime0 = micros();
    counter0 = counter0 + changevalue;              // Update counter
    encval = 0;
  }
}

void read_encoder1() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(ENC1_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(ENC1_B)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    int changevalue = 1;
    if((micros() - _lastIncReadTime1) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime1 = micros();
    counter1 = counter1 + changevalue;              // Update counter
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    int changevalue = -1;
    if((micros() - _lastDecReadTime1) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime1 = micros();
    counter1 = counter1 + changevalue;              // Update counter
    encval = 0;
  }
}

// if '47' observed, that means we're in image data receiving mode

// - now receive 2 integers, x and y of the image to be altered. then receive stream of bytes
// - clear pXiY, and begin appending the byte stream.
// - '48' observed again for end of image.
// - back to step 2
// - '49' observed again after last image to be updated - out of profile setting mode then
// - at the end 'reset' the esp32

