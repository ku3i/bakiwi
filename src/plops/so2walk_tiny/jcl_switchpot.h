#ifndef JCL_SWITCHPOT_H
#define JCL_SWITCHPOT_H

/*-----------------------+
 | Savepot               |
 | Jetpack Cognition Lab |
 | Matthias Kubisch      |
 | October 28th 2019     |
 +-----------------------*/

#include "Arduino.h"
//#include <EEPROM.h>


namespace jcl {

const float unlock_dx = 0.125;

float readpin(uint8_t pin) { return analogRead(pin) / 1023.f; }

void  savef(uint16_t addr, float f) {/*TODO*/} // EEPROM.write(prog_addr, program_set);
float loadf(uint16_t addr)          {/*TODO*/} // value = EEPROM.read(prog_addr);


class LockedADC {
    uint8_t pin;
    bool locked;
    float value;

public:

    LockedADC(uint8_t pin, float initial = 0.f, bool locked = true )
    : pin(pin), locked(locked), value(initial)
    {}

    void lock(void) { locked = true; }

    float get(void) const { return value; }

    void step(void) {
        const float nextval = readpin(pin);

        if (!locked) {
            value = nextval; // override immediately
        } //.. or check if we can unlock
        else if (fabs(value - nextval) <= unlock_dx) {
            locked = false;
            value = nextval;
        }
    }
};


class SwitchPot {
    //uint8_t eeprom_address;
    LockedADC val1, val2;
    bool flag = true;

public:
    SwitchPot(uint8_t pin/*, uint8_t address*/, float init1=0.5f, float init2=.5f)
    //: eeprom_address(address)
    : val1(pin, init1), val2(pin, init2)
    {
        //load();
    }

    void set(bool f) {
        flag = f;
        if (flag) val2.lock(); else val1.lock();
        //TODO save when locking
    }

    void step(void) {
        if (flag) val1.step(); else val2.step();
    }

    float get_1st(void) const { return val1.get(); }
    float get_2nd(void) const { return val2.get(); }
};


} /* namespace jcl */

#endif /* JCL_SWITCHPOT_H */
