// === PNG/TFT includes ===
#include <PNGdec.h>
#include "images.h"                 // your C-array PNGs live here

#include <SPI.h>
#include <TFT_eSPI.h>               // https://github.com/Bodmer/TFT_eSPI
TFT_eSPI tft = TFT_eSPI();

// === Flash FS (persistent) ===
#include <FS.h>
#include <LittleFS.h>
#include <esp_system.h>

// =================== constants ===================
#define MAX_IMAGE_WIDTH 64
#define NUM_PROFILES 6
#define ICONS_PER_PROFILE 6

// buttons 
const uint8_t buttons[6] = { 12, 19, 35, 36, 39, 34 };
const uint8_t DEBOUNCE_MS = 40;
bool     lastState[6] = {1,1,1,1,1,1};
uint32_t lastTime[6]  = {0};

// encoders 
#define ENC0_A 22
#define ENC0_B 17
#define ENC0_PUSH 21

#define ENC1_A 16
#define ENC1_B 25
#define ENC1_PUSH 26

unsigned long _lastIncReadTime0 = micros();
unsigned long _lastDecReadTime0 = micros();
unsigned long _lastIncReadTime1 = micros();
unsigned long _lastDecReadTime1 = micros();
int _pauseLength   = 25000;
int _fastIncrement = 10;

volatile int counter0 = 0;
volatile int counter1 = 0;

// screens 
const uint8_t screenCS[6] = { 15, 5, 32, 33, 27, 14 };
const uint8_t NUM_SCREENS = sizeof(screenCS) / sizeof(screenCS[0]);

// =================== images from images.h ===================

const uint8_t* icons[NUM_PROFILES][ICONS_PER_PROFILE] = {
  { p0i0, p0i1, p0i2, p0i3, p0i4, p0i5 },
  { p1i0, p1i1, p1i2, p1i3, p1i4, p1i5 },
  { p2i0, p2i1, p2i2, p2i3, p2i4, p2i5 },
  { p3i0, p3i1, p3i2, p3i3, p3i4, p3i5 },
  { p4i0, p4i1, p4i2, p4i3, p4i4, p4i5 },
  { p5i0, p5i1, p5i2, p5i3, p5i4, p5i5 }
};

size_t iconSize[NUM_PROFILES][ICONS_PER_PROFILE] = {
  { sizeof(p0i0), sizeof(p0i1), sizeof(p0i2), sizeof(p0i3), sizeof(p0i4), sizeof(p0i5) },
  { sizeof(p1i0), sizeof(p1i1), sizeof(p1i2), sizeof(p1i3), sizeof(p1i4), sizeof(p1i5) },
  { sizeof(p2i0), sizeof(p2i1), sizeof(p2i2), sizeof(p2i3), sizeof(p2i4), sizeof(p2i5) },
  { sizeof(p3i0), sizeof(p3i1), sizeof(p3i2), sizeof(p3i3), sizeof(p3i4), sizeof(p3i5) },
  { sizeof(p4i0), sizeof(p4i1), sizeof(p4i2), sizeof(p4i3), sizeof(p4i4), sizeof(p4i5) },
  { sizeof(p5i0), sizeof(p5i1), sizeof(p5i2), sizeof(p5i3), sizeof(p5i4), sizeof(p5i5) }
};

// =================== globals ===================
PNG png;
int16_t xpos = 0, ypos = 0;

bool dset = false;
int  profile = 0;

static bool profileMode = false;         // persists across loop
static const size_t MAX_ICON_BYTES = 64 * 64 * 4; // temp RAM for draw-from-FS

// =================== helpers ===================
static String fsPathFor(uint8_t x, uint8_t y) {
  return String("/p") + x + "_i" + y + ".png";     // e.g. /p0_i0.png
}

// PNGdec callback
void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

// render from FS if present; fall back to FLASH arrays
static bool drawIconFromFS(uint8_t p, uint8_t i) {
  const String path = fsPathFor(p, i);
  if (!LittleFS.exists(path)) return false;

  fs::File f = LittleFS.open(path, "r");
  if (!f) return false;
  const size_t sz = f.size();
  if (sz == 0 || sz > MAX_ICON_BYTES) { f.close(); return false; }

  static uint8_t buf[MAX_ICON_BYTES];
  const size_t rd = f.read(buf, sz);
  f.close();
  if (rd != sz) return false;

  int16_t rc = png.openRAM(buf, sz, pngDraw);
  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    png.decode(NULL, 0);
    tft.endWrite();
    png.close();
    Serial.printf("[DRAW] FS %s (%u bytes)\n", path.c_str(), (unsigned)sz);
    return true;
  }
  return false;
}

static void drawIcon(uint8_t p, uint8_t i) {
  if (drawIconFromFS(p, i)) return;

  // arrays in PROGMEM → use openFLASH (not openRAM)
  int16_t rc = png.openFLASH((uint8_t*)icons[p][i], iconSize[p][i], pngDraw);
  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    png.decode(NULL, 0);
    tft.endWrite();
    png.close();
    Serial.printf("[DRAW] FLASH p%u i%u\n", p, i);
  } else {
    Serial.printf("[DRAW] FAILED p%u i%u\n", p, i);
  }
}

static void renderProfile(int p) {
  for (uint8_t i = 0; i < NUM_SCREENS; i++) {
    for (uint8_t j = 0; j < NUM_SCREENS; j++) {
      digitalWrite(screenCS[j], (i == j) ? LOW : HIGH);
    }
    drawIcon(p, i);
  }
}

// diagnostics
static void listFiles() {
  Serial.println("[FS] Listing:");
  fs::File root = LittleFS.open("/");
  fs::File f = root.openNextFile();
  while (f) {
    Serial.printf(" - %s (%u)\n", f.name(), (unsigned)f.size());
    f = root.openNextFile();
  }
}

static void formatFS() {
  Serial.println("[FS] Formatting…");
  LittleFS.format();
  LittleFS.begin(true);
  Serial.println("[FS] Format done.");
}

// =================== setup ===================
void setup() {
  Serial.begin(115200);
  delay(50);

  for (uint8_t i = 0; i < NUM_SCREENS; ++i) {
    pinMode(screenCS[i], OUTPUT);
    digitalWrite(screenCS[i], LOW);
    pinMode(buttons[i], INPUT_PULLUP);
  }

  pinMode(ENC0_PUSH, INPUT_PULLUP);
  pinMode(ENC1_PUSH, INPUT_PULLUP);
  pinMode(ENC0_A, INPUT_PULLUP);
  pinMode(ENC0_B, INPUT_PULLUP);
  pinMode(ENC1_A, INPUT_PULLUP);
  pinMode(ENC1_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC0_A), read_encoder0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC0_B), read_encoder0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC1_A), read_encoder1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC1_B), read_encoder1, CHANGE);

  tft.begin();
  tft.fillScreen(TFT_BLACK);

  if (!LittleFS.begin(true)) {
    Serial.println("[FS] mount fail");
  } else {
    Serial.println("[FS] mounted OK");
  }

  dset = false;
  Serial.println("[BOOT] Ready. cmds: 10..15=set profile, 47=image mode, 90=list, 91=format, 92=redraw");
}

// =================== image mode receive ===================
static void handleImageReceive() {
  // Wait until we have x,y
  while (Serial.available() < 2) {}
  uint8_t x = Serial.read();
  uint8_t y = Serial.read();

  if (x >= NUM_PROFILES || y >= ICONS_PER_PROFILE) {
    Serial.printf("[RX] bad slot x=%u y=%u; skipping till 0x30\n", x, y);
    // consume bytes until terminator 0x30
    while (true) { while (!Serial.available()) {} if (Serial.read() == 0x30) break; }
  } else {
    String path = fsPathFor(x, y);
    fs::File f = LittleFS.open(path, "w");
    if (!f) {
      Serial.printf("[RX] open fail: %s\n", path.c_str());
      // consume to 0x30
      while (true) { while (!Serial.available()) {} if (Serial.read() == 0x30) break; }
    } else {
      size_t count = 0;
      while (true) {
        while (!Serial.available()) {}
        uint8_t b = Serial.read();
        if (b == 0x30) break;  // end-of-image
        f.write(&b, 1);
        count++;
      }
      f.close();
      Serial.printf("[RX] wrote %s (%u bytes)\n", path.c_str(), (unsigned)count);
    }
  }

  // if host queued an exit (0x31), consume it, leave image mode and reboot
  if (Serial.available() && Serial.peek() == 0x31) {
    Serial.read();           // consume 0x31
    profileMode = false;
    dset = false;            // force redraw with new icons
    Serial.println("[RX] exit image mode; restarting…");
    ESP.restart();
  }
}

// =================== loop ===================
void loop() {
  // ----- command handling (outside image mode) -----
  if (!profileMode && Serial.available()) {
    int cmd = Serial.parseInt(); // expecting numbers like "10\n", "47\n", etc.
    if (cmd >= 10 && cmd <= 15) {
      profile = cmd - 10;
      dset = false;
      Serial.printf("[CMD] profile=%d\n", profile);
    } else if (cmd == 47) {
      profileMode = true;
      Serial.println("[CMD] enter image mode");
    } else if (cmd == 90) {
      listFiles();
    } else if (cmd == 91) {
      formatFS();
    } else if (cmd == 92) {
      dset = false;
      Serial.println("[CMD] force redraw");
    }
  }

  if (!profileMode) {
    // -------- buttons (unchanged) --------
    for (uint8_t i = 0; i < NUM_SCREENS; ++i) {
      bool reading = digitalRead(buttons[i]);        // HIGH = released, LOW = pressed
      uint32_t now = millis();
      if (reading != lastState[i] && (now - lastTime[i] > DEBOUNCE_MS)) {
        lastTime[i]  = now;
        lastState[i] = reading;
        if (!reading) {                              // LOW means newly pressed
          Serial.println(i + 1);                    // send 1..6 once per press
        }
      }
    }

    // -------- encoders (unchanged) --------
    // ENC0 - Left
    if (digitalRead(ENC0_PUSH) == LOW) {
      Serial.println(73);
      delay(200);
    }
    static int lastCounter0 = 0;
    if (counter0 < lastCounter0) { Serial.println(71); lastCounter0 = counter0; }
    else if (counter0 > lastCounter0) { Serial.println(72); lastCounter0 = counter0; }

    // ENC1 - Right
    if (digitalRead(ENC1_PUSH) == LOW) {
      Serial.println(83);
      delay(200);
    }
    static int lastCounter1 = 0;
    if (counter1 < lastCounter1) { Serial.println(81); lastCounter1 = counter1; }
    else if (counter1 > lastCounter1) { Serial.println(82); lastCounter1 = counter1; }

    // -------- draw current profile once --------
    if (!dset) {
      renderProfile(profile);
      dset = true;
    }
  } else {
    // ----- image mode -----
    handleImageReceive();
  }
}

// =================== encoder ISRs (unchanged) ===================
void read_encoder0() {
  static uint8_t old_AB = 3;
  static int8_t encval = 0;
  static const int8_t enc_states[]  = {
    0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0
  };
  old_AB <<= 2;
  if (digitalRead(ENC0_A)) old_AB |= 0x02;
  if (digitalRead(ENC0_B)) old_AB |= 0x01;
  encval += enc_states[(old_AB & 0x0f)];
  if (encval > 3) {
    int v = 1; if ((micros() - _lastIncReadTime0) < _pauseLength) v *= _fastIncrement;
    _lastIncReadTime0 = micros(); counter0 += v; encval = 0;
  } else if (encval < -3) {
    int v = -1; if ((micros() - _lastDecReadTime0) < _pauseLength) v *= _fastIncrement;
    _lastDecReadTime0 = micros(); counter0 += v; encval = 0;
  }
}

void read_encoder1() {
  static uint8_t old_AB = 3;
  static int8_t encval = 0;
  static const int8_t enc_states[]  = {
    0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0
  };
  old_AB <<= 2;
  if (digitalRead(ENC1_A)) old_AB |= 0x02;
  if (digitalRead(ENC1_B)) old_AB |= 0x01;
  encval += enc_states[(old_AB & 0x0f)];
  if (encval > 3) {
    int v = 1; if ((micros() - _lastIncReadTime1) < _pauseLength) v *= _fastIncrement;
    _lastIncReadTime1 = micros(); counter1 += v; encval = 0;
  } else if (encval < -3) {
    int v = -1; if ((micros() - _lastDecReadTime1) < _pauseLength) v *= _fastIncrement;
    _lastDecReadTime1 = micros(); counter1 += v; encval = 0;
  }
}
