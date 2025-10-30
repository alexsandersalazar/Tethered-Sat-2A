#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <ESP32Servo.h>

#define RFM95_CS   5
#define RFM95_RST  14
#define RFM95_INT  26
#define RF95_FREQ  915.0 //change if diff
#define SERVO_PIN  18

RH_RF95 rf95(RFM95_CS, RFM95_INT);
Servo servo1;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  digitalWrite(RFM95_RST, LOW); delay(10);
  digitalWrite(RFM95_RST, HIGH); delay(10);

  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    while (1) { delay(1000); }
  }

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1) { delay(1000); }
  }

  rf95.setTxPower(13, false);

  servo1.attach(SERVO_PIN);
  servo1.write(0);

  Serial.println("LoRa RX ready");
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      int angle = (int)buf[0];
      angle = constrain(angle, 0, 180);
      servo1.write(angle);
      Serial.print("Got angle: ");
      Serial.print(angle);
      Serial.print("  RSSI: ");
      Serial.println(rf95.lastRssi());
    }
  }
}
