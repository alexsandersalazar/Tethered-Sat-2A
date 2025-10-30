#include <Arduino.h>



void setup(){
    Serial.begin(115200);          // initialize Serial at 115200 baud

    delay(500); // wait half a second for Serial to come online
    Serial.println("PRINTING ESP TEST");

}

void loop(){

}