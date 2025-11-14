#include <Arducam_Mega.h>
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// --- CS pins per your wiring ---
#define CAM1_PIN 17   // ArduCam #1 CS
#define CAM2_PIN 4    // ArduCam #2 CS
#define SD_PIN   5    // SD card CS

// --- SPI wiring (shared) ---
// SCK  = GPIO 18
// MOSI = GPIO 23
// MISO = GPIO 19

#define IMAGE_MODE       CAM_IMAGE_MODE_HD
#define PIXEL_FORMAT     CAM_IMAGE_PIX_FMT_JPG
#define SPI_CLOCK_HZ_SD  20000000UL
#define READ_CHUNK       1024

Arducam_Mega cam1(CAM1_PIN);
Arducam_Mega cam2(CAM2_PIN);

static inline void deselectAll() {
  // Keep every device idle unless actively selected by its library
  digitalWrite(CAM1_PIN, HIGH);
  digitalWrite(CAM2_PIN, HIGH);
  digitalWrite(SD_PIN,   HIGH);
}

bool saveCurrentFrameToSD(Arducam_Mega& cam, const char* path) {
  deselectAll();

  File f = SD.open(path, FILE_WRITE);
  if (!f) {
    Serial.println("[ERR] SD.open failed");
    return false;
  }

  uint8_t  buf[READ_CHUNK];
  uint32_t totalWritten = 0;

  while (cam.getReceivedLength()) {
    uint32_t toRead = min<uint32_t>(READ_CHUNK, cam.getReceivedLength());
    uint32_t got    = cam.readBuff(buf, toRead);
    if (got == 0) break;

    size_t w = f.write(buf, got);
    if (w != got) {
      Serial.println("[ERR] SD write short");
      f.close();
      return false;
    }
    totalWritten += got;
  }

  f.flush();
  f.close();
  Serial.printf("[OK] Wrote %lu bytes to %s\n", (unsigned long)totalWritten, path);
  return totalWritten > 0;
}

bool initCam(Arducam_Mega &cam) {
  deselectAll();
  if (!cam.begin()) {
    Serial.println("[ERR] Cam start failure");
    return false;
  }
  delay(50);
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  // Shared SPI bus (VSPI defaults on ESP32 → SCK=18, MISO=19, MOSI=23)
  SPI.begin(18, 19, 23);

  pinMode(CAM1_PIN, OUTPUT);
  pinMode(CAM2_PIN, OUTPUT);
  pinMode(SD_PIN,   OUTPUT);
  deselectAll();

  

  // Initialize SD at 20 MHz on the same SPI bus
  if (!SD.begin((uint8_t)SD_PIN, SPI, SPI_CLOCK_HZ_SD)) {
    Serial.println(F("[ERR] SD.begin failed"));
    while (1) delay(10);
  }

  if (!initCam(cam1) || !initCam(cam2)) {
    Serial.println(F("[ERR] One/both cameras failed to init"));
    while (1) delay(10);
  }

  Serial.println(F("[OK] Setup complete. Alternating captures..."));
}

// state
bool cam1turn = true;
unsigned long shot1 = 0;
unsigned long shot2 = 0;
const char cam1PathPrefix[] = "/cam1_";
const char cam2PathPrefix[] = "/cam2_";

void loop() {
  if (cam1turn) {
    cam1.takePicture(IMAGE_MODE, PIXEL_FORMAT);
    char path[32];
    snprintf(path, sizeof(path), "%s%lu.jpg", cam1PathPrefix, shot1++);
    if (!saveCurrentFrameToSD(cam1, path)) Serial.println("[ERR] cam1 save failed");
  } else {
    cam2.takePicture(IMAGE_MODE, PIXEL_FORMAT);
    char path[32];
    snprintf(path, sizeof(path), "%s%lu.jpg", cam2PathPrefix, shot2++);
    if (!saveCurrentFrameToSD(cam2, path)) Serial.println("[ERR] cam2 save failed");
  }
  cam1turn = !cam1turn;
}
