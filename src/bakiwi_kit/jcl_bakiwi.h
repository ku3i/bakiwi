#ifndef JCL_PLOPS_H
#define JCL_PLOPS_H

/*---------------------------------+
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | October 30th, 2019              |
 +---------------------------------*/

#include <Arduino.h>
#include <math.h>
#include "jcl_tinyservo.h"
#include "jcl_switchpot.h"
#include "jcl_modules.h"


#ifdef round
#undef round
#endif

#ifdef constrain
#undef constrain
#endif


namespace jcl {

namespace io {

/* user interface */
const unsigned POTI_FRQ_PHS = A3; // poti for walking frequency + phase
const unsigned POTI_AMP_DIF = A2; // poti for motor amplitude and difference
const unsigned LEDBUTTON    =  2; // tactile button + LED pin

/* actuators */
const unsigned MOTOR_1    = 0;
const unsigned MOTOR_2    = 1;

} /* namespace io */


namespace constants {
    const unsigned long WAIT_US = 20*1000UL;
    const unsigned MAX_ANGLE = 90; // DEG
    const float    PREAMP = 1.4f;
}


    void apply(void) {
        if (!paused) {
            mot1.apply(tar1);
            mot2.apply(tar2);
        }
    }


class Button
{
    const uint8_t pin;
    bool is_pressed = false;
    bool state = false;
    int8_t integ = 0;

    const int8_t UP = 3; /* min. timesteps need with pin high/low to switch state */

    void read(void) {
        /* saturating integrator */
        integ = clip(integ + (digitalRead(pin) ? -1 : +1), 0, UP);

        /* hysteresis switch */
             if ( state and integ==0 ) state = false;
        else if (!state and integ==UP) state = true;
    }

public:
    Button(uint8_t pin) : pin(pin) {
        pinMode(pin, INPUT_PULLUP); /* needs pullup resistor */
    }

    bool step(void) {
        read();
        bool result = (is_pressed and !state); /* trigger on button release */
        is_pressed = state;
        return result;
    }

    bool get_state(void) const { return is_pressed; }
    int8_t get_integ(void) const { return integ;}
};


class Board {
public:

    bool paused = true;
    Button button;
    Tinyservo mot_1, mot_2;

    SwitchPot poti_1, poti_2;

    float freq  = .0f,
          amp1  = .0f,
          amp2  = .0f,
          phase = .0f;

    const uint8_t max_a = constants::MAX_ANGLE;

    Board(uint8_t id)
    : button(io::BUTTON)
    , mot_1(io::MOTOR_1), mot_2(io::MOTOR_2)
    , poti_2(io::POTI_1), poti_2(io::POTI_2)
    {
        read_potentiometers();
    }

    float motorscale(float a) { return round(clip(a * max_a + max_a, 0.f, 2.f * max_a)); }

    void set_motors(float u1, float u2) {
        mot_1.write(motorscale(u1 * amp1));
        mot_2.write(motorscale(u2 * amp2));
    }

    void step(void)
    {
        read_potentiometers();
        if (button.step()) {
            paused = !paused; // toggle pause on button release
            if (paused) {
                mot_1.detach();
                mot_2.detach();
            } else {
                mot_1.attach(pin1);
                mot_2.attach(pin2);
                //TODO restart osc
            }
        }

        /* write actuators */
        if (!paused) {
            mot1.write(tar1);
            mot2.write(tar2);
        }

    }

    bool is_paused(void) const { return paused; }

    float readpin(uint8_t pin) { return analogRead(pin) / 1023.f; }

    uint8_t to_pwm(float val) { return round(clip(val, 0.f, 1.f) * 255u); }

    void read_potentiometers(void) {
        amp1 = constants::PREAMP * readpin(io::POTI_AMP1);
        amp2 = constants::PREAMP * readpin(io::POTI_AMP2);
        freq = readpin(io::POTI_FREQ);
        phase = 2 * readpin(io::POTI_PHASE) - 1.f;
    }

    float get_temperature(void) const { return motion.tm; }



};


} /* namespace jcl */


#endif /* JCL_PLOPS_H */
