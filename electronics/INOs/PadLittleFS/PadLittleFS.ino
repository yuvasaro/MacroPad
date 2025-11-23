// ============================================================================
//  MacroPad_LittleFS.ino
//  ESP32 firmware for MacroPad with 6 OLEDs + 6 buttons + 2 encoders
//  Receives PNG icons via serial (from Qt app) and stores them on LittleFS
//  Uses flat filenames: /pXiY.png  (e.g. /p0i3.png)
// ============================================================================

#include <FS.h>
#include <LittleFS.h>
#include <PNGdec.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <esp_system.h>

// ================== DISPLAY + PNG ==================
TFT_eSPI tft = TFT_eSPI();
PNG png;
#define MAX_IMAGE_WIDTH 64

// ================== CONSTANTS ==================
#define NUM_PROFILES 6
#define ICONS_PER_PROFILE 6
static const size_t MAX_ICON_BYTES = 64 * 64 * 4;

// Buttons
const uint8_t buttons[6] = { 12, 19, 35, 36, 39, 34 };
const uint8_t DEBOUNCE_MS = 40;
bool lastState[6] = {1,1,1,1,1,1};
uint32_t lastTime[6] = {0};

// Encoders
#define ENC0_A 22
#define ENC0_B 17
#define ENC0_PUSH 21

#define ENC1_A 16
#define ENC1_B 25
#define ENC1_PUSH 26

volatile int counter0 = 0, counter1 = 0;
unsigned long _lastIncReadTime0 = micros(), _lastDecReadTime0 = micros();
unsigned long _lastIncReadTime1 = micros(), _lastDecReadTime1 = micros();
int _pauseLength = 25000, _fastIncrement = 10;

// Screens
const uint8_t screenCS[6] = { 15, 5, 32, 33, 27, 14 };
const uint8_t NUM_SCREENS = sizeof(screenCS) / sizeof(screenCS[0]);

// Globals
bool dset = false;
int profile = 0;
bool profileMode = false;

// ================== HELPERS ==================
static String fsPathFor(uint8_t x, uint8_t y) {
  return String("/p") + x + "i" + y + ".png";
}

// Here are the callback functions that the decPNG library
// will use to open files, fetch data and close the file.

File pngfile;

void * pngOpen(const char *filename, int32_t *size) {
  Serial.printf("Attempting to open %s\n", filename);
  pngfile = LittleFS.open(filename, "r");
  *size = pngfile.size();
  return &pngfile;
}

void pngClose(void *handle) {
  File pngfile = *((File*)handle);
  if (pngfile) pngfile.close();
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
  if (!pngfile) return 0;
  page = page; // Avoid warning
  return pngfile.read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position) {
  if (!pngfile) return 0;
  page = page; // Avoid warning
  return pngfile.seek(position);
}

void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(0, pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

static void drawIconFromFS(uint8_t p, uint8_t i) {
  String path = String("/p") + p + "i" + i + ".png";
  if (!LittleFS.exists(path)) {
    Serial.printf("[DRAW] Missing %s\n", path.c_str());
    return;
  }

  // Stream from FS using callbacks
  int16_t rc = png.open(path.c_str(), pngOpen, pngClose, pngRead, pngSeek, pngDraw);
  if (rc == PNG_SUCCESS) {
    if (png.getWidth() > MAX_IMAGE_WIDTH) {
      Serial.printf("[DRAW] %s too wide (%dpx)\n", path.c_str(), png.getWidth());
      png.close();
      return;
    }
    tft.startWrite();
    uint32_t t0 = millis();
    png.decode(NULL, 0);
    tft.endWrite();
    png.close();
    Serial.printf("[DRAW] %s OK in %lu ms\n", path.c_str(), (unsigned long)(millis() - t0));
  } else {
    Serial.printf("[DRAW] open() failed for %s\n", path.c_str());
  }
}

static void renderProfile(int p) {
  for (uint8_t i = 0; i < NUM_SCREENS; i++) {
    for (uint8_t j = 0; j < NUM_SCREENS; j++) {
      digitalWrite(screenCS[j], (i == j) ? LOW : HIGH);
    }
    drawIconFromFS(p, i);
  }
}

static void listFiles() {
  Serial.println("[FS] Listing:");
  fs::File root = LittleFS.open("/");
  fs::File f = root.openNextFile();
  while (f) {
    Serial.printf(" - %s (%u bytes)\n", f.name(), (unsigned)f.size());
    f = root.openNextFile();
  }
}

static void formatFS() {
  Serial.println("[FS] Formatting FS...");
  LittleFS.format();
  LittleFS.begin(true);
  Serial.println("[FS] Format complete.");
}

// ================== IMAGE RECEIVE ==================
static void handleImageReceive() {
  // Expect: x(1), y(1), len(4), <len bytes>
  while (Serial.available() < 6) {}
  uint8_t x = Serial.read();
  uint8_t y = Serial.read();
  uint32_t len = 0;
  len |= uint32_t(Serial.read());
  len |= uint32_t(Serial.read()) << 8;
  len |= uint32_t(Serial.read()) << 16;
  len |= uint32_t(Serial.read()) << 24;

  if (x >= NUM_PROFILES || y >= ICONS_PER_PROFILE || len > MAX_ICON_BYTES) {
    Serial.printf("[RX] Invalid params x=%u y=%u len=%u\n", x, y, len);
    return;
  }

  String path = fsPathFor(x, y);
  fs::File f = LittleFS.open(path, "w");
  if (!f) { Serial.printf("[RX] Open fail %s\n", path.c_str()); return; }

  uint8_t buf[512];
  size_t remaining = len;
  while (remaining > 0) {
    size_t chunk = min<size_t>(remaining, sizeof(buf));
    size_t got = Serial.readBytes((char*)buf, chunk);
    if (got == 0) continue;
    f.write(buf, got);
    remaining -= got;
  }
  f.close();
  Serial.printf("[RX] Wrote %s (%u bytes)\n", path.c_str(), (unsigned)len);

  if (Serial.peek() == 49) {   // 0x31 = end of transfer
    Serial.read();
    Serial.println("[RX] Transfer complete, restarting...");
    ESP.restart();
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(50);

  for (uint8_t i = 0; i < NUM_SCREENS; i++) {
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
    Serial.println("[FS] Mount failed.");
  } else {
    Serial.println("[FS] Mounted OK.");
    listFiles();
  }

  Serial.println("[BOOT] Ready. 1-9, 71-73, 81-83=cmds 10..15=set profile, 47=image mode, 90=list, 91=format, 92=redraw");
}

// ================== LOOP ==================
void loop() {
  if (!profileMode && Serial.available()) {
    int cmd = Serial.peek();
    if (cmd == 47) { Serial.read(); profileMode = true; Serial.println("[CMD] Enter image mode"); }
    else if (cmd == 49) { Serial.read(); ESP.restart(); }
    else {
      int n_profile = Serial.parseInt();
      if (n_profile >= 10 && n_profile <= 15) {
        profile = n_profile - 10;
        dset = false;
        Serial.printf("[CMD] profile=%d\n", profile);
      } else if (n_profile == 90) listFiles();
      else if (n_profile == 91) formatFS();
      else if (n_profile == 92) { dset = false; Serial.println("[CMD] Force redraw"); }
    }
  }

  if (!profileMode) {
    // --- buttons ---
    for (uint8_t i = 0; i < NUM_SCREENS; ++i) {
      bool reading = digitalRead(buttons[i]);
      uint32_t now = millis();
      if (reading != lastState[i] && (now - lastTime[i] > DEBOUNCE_MS)) {
        lastTime[i] = now;
        lastState[i] = reading;
        if (!reading) Serial.printf("#%d\n", i + 1);
      }
    }

    // --- encoders ---
    if (digitalRead(ENC0_PUSH) == LOW) { Serial.println("#73"); delay(200); }
    static int lastCounter0 = 0;
    if (counter0 < lastCounter0) { Serial.println("#71"); lastCounter0 = counter0; }
    else if (counter0 > lastCounter0) { Serial.println("#72"); lastCounter0 = counter0; }

    if (digitalRead(ENC1_PUSH) == LOW) { Serial.println("#83"); delay(200); }
    static int lastCounter1 = 0;
    if (counter1 < lastCounter1) { Serial.println("#81"); lastCounter1 = counter1; }
    else if (counter1 > lastCounter1) { Serial.println("#82"); lastCounter1 = counter1; }

    // --- draw profile once ---
    if (!dset) { renderProfile(profile); dset = true; }
  } 
  else {
    handleImageReceive();
  }
}

// ================== ENCODER ISR ==================
void read_encoder0() {
  static uint8_t old_AB = 3; static int8_t encval = 0;
  static const int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  old_AB <<= 2;
  if (digitalRead(ENC0_A)) old_AB |= 0x02;
  if (digitalRead(ENC0_B)) old_AB |= 0x01;
  encval += enc_states[(old_AB & 0x0f)];
  if (encval > 3) { int v = 1; if ((micros() - _lastIncReadTime0) < _pauseLength) v *= _fastIncrement;
    _lastIncReadTime0 = micros(); counter0 += v; encval = 0; }
  else if (encval < -3) { int v = -1; if ((micros() - _lastDecReadTime0) < _pauseLength) v *= _fastIncrement;
    _lastDecReadTime0 = micros(); counter0 += v; encval = 0; }
}

void read_encoder1() {
  static uint8_t old_AB = 3; static int8_t encval = 0;
  static const int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  old_AB <<= 2;
  if (digitalRead(ENC1_A)) old_AB |= 0x02;
  if (digitalRead(ENC1_B)) old_AB |= 0x01;
  encval += enc_states[(old_AB & 0x0f)];
  if (encval > 3) { int v = 1; if ((micros() - _lastIncReadTime1) < _pauseLength) v *= _fastIncrement;
    _lastIncReadTime1 = micros(); counter1 += v; encval = 0; }
  else if (encval < -3) { int v = -1; if ((micros() - _lastDecReadTime1) < _pauseLength) v *= _fastIncrement;
    _lastDecReadTime1 = micros(); counter1 += v; encval = 0; }
}
