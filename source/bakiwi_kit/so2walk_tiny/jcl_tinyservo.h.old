/*-----------------------+
 | Tinyservo             |
 | Jetpack Cognition Lab |
 | Matthias Kubisch      |
 | October 28th 2019     |
 +-----------------------*/

/* use apply() once every 20 milliseconds
   e.g. with an interupt of timer0
 */


#include "Arduino.h"

namespace jcl {

    const uint8_t default_angle = 90;

    const uint16_t def_min_ang = 0;
    const uint16_t def_max_ang = 180;

    const uint16_t def_min_us = 544;
    const uint16_t def_max_us = 2400;

class Tinyservo {
    bool is_attached;
    uint8_t pin, angle;
    uint16_t dt_us;

public:
    Tinyservo(void)
    : is_attached(false)
    , pin(255)
    , angle(default_angle)
    , dt_us(0)
    {}

    void attach(uint8_t p) {
        pin = p;
        angle = default_angle;
        is_attached = true;
        pinMode(pin, OUTPUT);
    }

    void detach(void) {
        is_attached = false;
        pinMode(pin, INPUT);
    }

    void write(uint8_t target) {
        angle = target;
        if (not is_attached) return;
        dt_us = map(target, def_min_ang, def_max_ang, def_min_us, def_max_us);
    }

    void apply(void) const {
        digitalWrite(pin, HIGH);
        delayMicroseconds(dt_us);
        digitalWrite(pin, LOW);
    }

};

} /* namespace jcl */
