#include <Arduino.h>



void setup(){
    Serial.begin(115200);          // initialize Serial at 115200 baud
    while (!Serial) { ; }          // optional; helps on some boards
    Serial.println("PRINTING ESP TEST");

}

void loop(){

}