// T-Sat ESP32 (RX + Servo)
#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <ESP32Servo.h>

#define RF95_CS   5
#define RF95_INT   21
#define RF95_RST  4

#define SERVO_PIN   13   // choose a free PWM-capable pin (NOT SPI SCK)
#define BUTTON_PIN  2    // local safety/arm button on the satellite

RH_RF95 rf95(RF95_CS, RF95_INT);
Servo servo1;

void triggerDetach() {
  Serial.println("DETACH: moving servo...");
  servo1.write(180);     // detach position
  delay(2000);           // hold for 2s (adjust as needed)
  servo1.write(0);       // back to safe position (optional)
  Serial.println("Servo movement complete");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing T-Sat...");

  // Radio reset
  pinMode(RF95_RST, OUTPUT);
  digitalWrite(RF95_RST, HIGH);

  // Local safety button
  pinMode(BUTTON_PIN, INPUT);   // or INPUT_PULLUP if wired to GND

  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    while (1);
  }

  rf95.setFrequency(915.0);
  rf95.setTxPower(13, false);

  // Servo setup
  servo1.attach(SERVO_PIN);
  servo1.write(0);  // initial position

  Serial.println("T-Sat RX ready");
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      if (len >= sizeof(buf)) len = sizeof(buf) - 1;
      buf[len] = '\0';   // make it a C-string

      Serial.print("Received: ");
      Serial.println((char *)buf);

      // Check for "Detach"
      if (strcmp((char *)buf, "Detach") == 0) {
        // Optional safety: require local button to be pressed
        if (digitalRead(BUTTON_PIN) == HIGH) {
          Serial.println("Detach command + button HIGH → triggering servo");
          triggerDetach();

          // send confirmation back
          const char reply[] = "Detached";
          rf95.send((uint8_t *)reply, sizeof(reply));
          rf95.waitPacketSent();
        } else {
          Serial.println("Detach command ignored: BUTTON not pressed (safety).");
          const char reply[] = "Detach ignored - button LOW";
          rf95.send((uint8_t *)reply, sizeof(reply));
          rf95.waitPacketSent();
        }
      }
    } else {
      Serial.println("Receive failed");
    }
  }
}
