#ifndef BAKIWI_SENSORS_TOUCH_H
#define BAKIWI_SENSORS_TOUCH_H

#include <CapacitiveSensor.h>

class Touchsensor {
    CapacitiveSensor s1;//,s2;
public:

    long val1 = 0;
        // val2 = 0;

    Touchsensor(uint8_t send, uint8_t recv1, uint8_t recv2)
    : s1(send, recv1)
    //, s2(send, recv2)
    {
        s1.set_CS_AutocaL_Millis(0xFFFFFFFF);
        //s2.set_CS_AutocaL_Millis(0xFFFFFFFF);
    };

    void step(void) {
        val1 = s1.capacitiveSensor(30);
        //val2 = s2.capacitiveSensor(30);
    }
};

#endif /* BAKIWI SENSORS_TOUCH_H */
