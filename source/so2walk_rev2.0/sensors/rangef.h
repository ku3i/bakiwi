#include <VL53L0X.h>

/* time-of-flight sensor */

class Rangefinder {
    VL53L0X sensor;

public:

    uint16_t dx = 0;

    Rangefinder() : sensor() {}

    void init(void) {
        sensor.init();
        sensor.setTimeout(1);
        /* Start continuous back-to-back mode,
         * take readings as fast as possible. */
        sensor.startContinuous();
    }


    void step(void) {
        const auto t = sensor.readRangeContinuousMillimeters();
        if (65535 != t)         // if not timed-out
            dx = min(t,1200);

        /* note: out of range value is 8190
           but sensor can measure slightly above 1200
         */
    }

};

