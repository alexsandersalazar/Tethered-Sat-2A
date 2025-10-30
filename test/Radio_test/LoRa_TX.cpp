#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS   5
#define RFM95_RST  14
#define RFM95_INT  26
#define RF95_FREQ  915.0 //change if diff

RH_RF95 rf95(RFM95_CS, RFM95_INT);

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

  Serial.println("LoRa TX ready");
}

void loop() {
  static uint8_t angle = 0;
  uint8_t msg[1] = { angle };
  rf95.send(msg, sizeof(msg));
  rf95.waitPacketSent();

  Serial.print("Sent angle: ");
  Serial.println(angle);

  angle = (angle + 15) % 181;
  delay(500);
}
