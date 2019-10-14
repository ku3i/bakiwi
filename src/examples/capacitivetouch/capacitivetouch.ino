#include <CapacitiveSensor.h>

CapacitiveSensor cs = CapacitiveSensor(2,12);


void setup() {
  cs.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
  Serial.begin(1000000);
  //cs.set_CS_Timeout_Millis(10);
}

void loop() {
    long start = micros();
    long total1 =  cs.capacitiveSensor(30);

    Serial.print(micros() - start);        // check on performance in milliseconds
    Serial.print("\t");                    // tab character for debug window spacing
    Serial.println(total1);                // print sensor output 3
    //delay(20);          
}
