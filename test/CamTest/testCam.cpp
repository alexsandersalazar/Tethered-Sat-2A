#include <Arducam_Mega.h>
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define CAM1_PIN 00
#define CAM2_PIN 00         //WRITE PIN NUMBERS WHEN SELECTED
#define SD_PIN 00


#define IMAGE_MODE CAM_IMAGE_MODE_HD
#define PIXEL_FORMAT CAM_IMAGE_PIX_FMT_JPG
#define SPI_CLOCK_HZ_SD 20000000
#define READ_CHUNK        1024





Arducam_Mega cam1(CAM1_PIN);
Arducam_Mega cam2(CAM2_PIN);

void clear(){

    digitalWrite(CAM1_PIN, HIGH);
    digitalWrite(CAM2_PIN, HIGH);
    digitalWrite(SD_PIN, HIGH);

}

bool saveCurrentFrameToSD(Arducam_Mega& cam, const char* path) {
    clear();
  // Open file first (with all CS high so SD is idle until selected by library)
  File f = SD.open(path, FILE_WRITE);
  if (!f) {
    Serial.println("[ERR] SD.open failed");
    return false;
  }

  uint8_t buf[READ_CHUNK];
  uint32_t totalWritten = 0;

  // Read until camera reports no more data
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
  Serial.print("[OK] Wrote ");
  Serial.print(totalWritten);
  Serial.print(" bytes to ");
  Serial.println(path);
  return totalWritten > 0;
}


bool initCam(Arducam_Mega &cam){


   clear();

    if(!cam.begin()){
        Serial.println("Cam start failure");//error starting the camera
        return false;
    }

    delay(50);

    return true;
}






void setup(){

    Serial.begin(115200);
    while (!Serial) { ; }

    // SPI + CS pins
    SPI.begin(); // default SPI pins for your board


    pinMode(CAM1_PIN, OUTPUT);
    pinMode(CAM2_PIN, OUTPUT);
    pinMode(SD_PIN,   OUTPUT);
    clear();
    // SD card at requested SPI clock
    if (!SD.begin((uint8_t)SD_PIN)) {
        Serial.println(F("[ERR] SD.begin failed"));
        while (1) delay(10);
    }
    // Cameras
    if (!initCam(cam1) || !initCam(cam2)) {
        Serial.println(F("[ERR] One/both cameras failed to init"));
        while (1) delay(10);
    }

  Serial.println(F("[OK] Setup complete. Alternating captures..."));

  //SD EXISTS /log.csv

}

    bool cam1turn = true;
    int shot1 = 0;
    int shot2 = 0;
    char cam1Path[] = {"/cam1_"};       //NEED TO CHANGE BEFORE TESTING BEGINS!!!!!! THESE ARE TEMP
    char cam2Path[] = {"/cam2_"};    // filepath for cameras

    
void loop(){


    if(cam1turn){
        cam1.takePicture(IMAGE_MODE, PIXEL_FORMAT);

    char path[32];
    snprintf(path, sizeof(path), "%s%lu.jpg",cam1Path, (unsigned long)shot1++);
    if (!saveCurrentFrameToSD(cam1, path)){
        Serial.println("Error");//error
    }
    } else {
        cam2.takePicture(IMAGE_MODE, PIXEL_FORMAT);

        char path[32];
        snprintf(path, sizeof(path), "%s%lu.jpg",cam2Path, (unsigned long)shot2++);
        if (!saveCurrentFrameToSD(cam2, path)){
            Serial.println("Error");//error
        }
    }
    cam1turn = !cam1turn;

}