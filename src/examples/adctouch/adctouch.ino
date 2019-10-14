#include <ADCTouch.h>

/* example code for running touch sensors with ADCTouch lib.*/

int ref0, ref1;     //reference values to remove offset

void setup()
{
    /* No pins to setup. 
       Pins can still be used regularly,
       although it will affect readings. */

    Serial.begin(115200);
    ref0 = ADCTouch.read(A0, 10);    //create reference values to
    ref1 = ADCTouch.read(A1, 10);    //account for the capacitance of the pad
}

void loop()
{
    int value0 = ADCTouch.read(A0);   //no second parameter
    int value1 = ADCTouch.read(A1);   //   --> 100 samples

    value0 -= ref0;                   //remove offset
    value1 -= ref1;

    Serial.print(value0);
    Serial.print("\t");
    Serial.print(value1);             //send actual readings
    Serial.println();
    delay(5);
}
