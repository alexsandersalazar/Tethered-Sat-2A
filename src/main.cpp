// Ground ESP32 (TX)
#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>

#define RF95_CS   5
#define RF95_INT   21
#define RF95_RST  4
#define BUTTON_PIN 2   // Button on ground

RH_RF95 rf95(RF95_CS, RF95_INT);

int buttonValue = 0;
int lastButtonValue = 0;

// Message sent when the button is pressed
uint8_t detachMsg[] = "Detach";

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing ground...");

  pinMode(RF95_RST, OUTPUT);
  digitalWrite(RF95_RST, HIGH);

  pinMode(BUTTON_PIN, INPUT);  // or INPUT_PULLUP depending on wiring

  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    while (1);
  }

  rf95.setFrequency(915.0);

  Serial.println("Ground Station Ready");
}

void loop() {
  buttonValue = digitalRead(BUTTON_PIN);

  // Rising edge: button just pressed
  if (buttonValue == HIGH && lastButtonValue == LOW) {
    Serial.println("Sending 'Detach'...");
    rf95.send(detachMsg, sizeof(detachMsg));   // send "Detach"
    rf95.waitPacketSent();
    Serial.println("Message sent: 'Detach'");

    // optional debounce
    delay(200);
  }

  lastButtonValue = buttonValue;

  // (Optional) listen for replies from T-Sat
  if (rf95.waitAvailableTimeout(50)) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      if (len >= sizeof(buf)) len = sizeof(buf) - 1;
      buf[len] = '\0';

      Serial.print("Reply from T-Sat: ");
      Serial.println((char *)buf);
    } else {
      Serial.println("Receive failed");
    }
  }
}
