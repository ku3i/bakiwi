#include <VL53L0X.h>

/* time-of-flight sensor */

class Rangefinder {
    VL53L0X sensor;

public:

    uint16_t dx = 0;

    Rangefinder() : sensor() {}

    void init(void) {
        sensor.init();
        sensor.setTimeout(50);
        /* Start continuous back-to-back mode,
         * take readings as fast as possible. */
        sensor.startContinuous();
    }


    void step(void) {
        const auto t = sensor.readRangeContinuousMillimeters();
        dx = min(t,2000);
        //if (sensor.timeoutOccurred()) { }
    }

};

