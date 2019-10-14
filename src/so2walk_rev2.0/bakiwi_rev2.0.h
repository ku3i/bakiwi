#ifndef BAKIWI_REV2_H
#define BAKIWI_REV2_H

/*---------------------------------+
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | October 3th, 2019               |
 +---------------------------------*/

#include <Arduino.h>
#include <math.h>
#include <Servo.h>
#include <Wire.h>

#include "modules.h"
#include "sensors/rangef.h"
#include "sensors/motion.h"
#include "sensors/touch.h"
#include "sound/tones.h"
#include "sound/melodies.h"

#ifdef round
#undef round
#endif

#ifdef constrain
#undef constrain
#endif


volatile uint8_t led1pwm = 0;
volatile uint8_t led2pwm = 0;
volatile uint8_t pwmcnt = 0;
volatile bool buz_toggle = false; //TODO make buz_pwm and buz_vol
volatile bool buzzer_on = false;

namespace bakiwi {

namespace io { //TODO for consts: use same conventions for schem and code

/* user interface */
const unsigned POTI_FREQ  = A7; // poti for walking frequency
const unsigned POTI_PHASE = A2; // poti for phase shift between motors
const unsigned POTI_AMP1  = A6; // amplitude motor 1
const unsigned POTI_AMP2  = A3; // amplitude motor 2
const unsigned IRRECV     =  7; // to be removed
const unsigned BUTTON     =  4; // tactile button

/* actuators */
const unsigned MOTOR_1    =  9;
const unsigned MOTOR_2    = 10;
const unsigned MOTOR_3    =  5;
const unsigned MOTOR_4    =  6;
const unsigned LED_1      =  3;
const unsigned LED_2      = 11;
const unsigned BUZZER     =  8;

/* sensors*/
const unsigned TOUCH_SEND =  2;
const unsigned TOUCH_RCV1 = 13;
const unsigned TOUCH_RCV2 = 12;
const unsigned FLEX_SENS1 = A0;
const unsigned FLEX_SENS2 = A1;

} /* namespace io */


namespace constants {
    /* Note: the step size is largely dominated by the ToF sensor.
     * Without it, 10ms are reachable.
     */
    const unsigned long WAIT_US = 40*1000UL;
    const unsigned MAX_ANGLE = 90; // DEG
    const float    PREAMP = 1.4f;
}

void set_freq(float frq);


class Motors {

    const uint8_t pin1, pin2;
    bool const& paused;
    Servo   mot1, mot2;
    uint8_t tar1, tar2;

public:
    Motors(uint8_t pin1, uint8_t pin2, bool const& paused)
    : pin1(pin1), pin2(pin2), paused(paused), mot1(), mot2()
    {}

    void set_position(uint8_t t1, uint8_t t2) {
        tar1 = t1;
        tar2 = t2;
    }

    void attach() {
        mot1.attach(pin1);
        mot2.attach(pin2);
    }

    void detach() {
        mot1.detach();
        mot2.detach();
    }

    void step(void) {
        /* write motors */
        if (!paused) {
            mot1.write(tar1);
            mot2.write(tar2);
        }
    }
};


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

class Lights {
    const uint8_t pin1, pin2;
    uint8_t pwm1, pwm2;
public:
    Lights(uint8_t pin1, uint8_t pin2)
    : pin1(pin1), pin2(pin2)
    {
        pinMode(pin1, OUTPUT);
        pinMode(pin2, OUTPUT);
    }

    void set_pwm(uint8_t tar1, uint8_t tar2) {
        pwm1 = tar1;
        pwm2 = tar2;
    }

    void step(void) {
        //analogWrite(pin1, pwm1);
        //analogWrite(pin2, pwm2);
        led1pwm = pwm1/16;
        led2pwm = pwm2/16;
    }

};

class Sound {
    const uint8_t pin;
    bool on = false;
public:
    Sound(uint8_t pin) : pin(pin) { pinMode(pin, OUTPUT); }

    void play_note(unsigned no, unsigned dur_ms) {}
    void play_melody() {
        /* play totoro */
        switch_on();
        for (unsigned i = 0; i < 6; ++i) {
          set_freq(tonetable[88-totoro[i]]);
          delay(200);
        }
        switch_off();      
        set_freq(16000);
    }

    void step() {
        //test tones
        //unsigned sel = 87*board.freq;
        //set_freq(tonetable[sel]);
    }

    void switch_on(void) { on = true; }
    void switch_off(void) { on = false; }
};


class Board {
public:

    bool paused = true;
    Button button;
    Lights lights;
    Motors motors;
    Sound buzzer;
    Motionsensor motion;
    Rangefinder rangef;
    Touchsensor touch;

    float freq  = .0f,
          amp1  = .0f,
          amp2  = .0f,
          phase = .0f;

    const uint8_t max_a = constants::MAX_ANGLE;

    Board(uint8_t id)
    : button(io::BUTTON)
    , lights(io::LED_1, io::LED_2)
    , motors(io::MOTOR_1, io::MOTOR_2, paused)
    , buzzer(io::BUZZER)
    , motion(Wire)
    , rangef()
    , touch(io::TOUCH_SEND, io::TOUCH_RCV1, io::TOUCH_RCV2)
    {
        read_potentiometers();
    }

    /* must be called during setup() */
    void init(void) {
        Wire.begin();
        Wire.setClock(400000); /* set I2C clock speed to 400 kHz */
        motion.init();
        rangef.init();
    }

    float motorscale(float a) { return round(clip(a * max_a + max_a, 0.f, 2.f * max_a)); }

    void set_motors(float u1, float u2) {
        motors.set_position( motorscale(u1 * amp1), motorscale(u2 * amp2) );
    }

    void set_lights(float l1, float l2) {
        if (paused) // in pause mode, show amplitude
            lights.set_pwm( to_pwm(l1*(0.1+amp1))
                          , to_pwm(l2*(0.1+amp2)) );
        else
            lights.set_pwm( to_pwm(l1), to_pwm(l2) );
    }

    void step(void)
    {
        read_potentiometers();
        if (button.step()) {
            paused = !paused; // toggle pause on button release
            if (paused) {
                motors.detach();
            } else {
                motors.attach();
                //TODO restart_oscillation();
            }
        }

        /* write actuators */
        motors.step();
        lights.step();
        buzzer.step();

        /* read sensors */
        motion.step();
        rangef.step();
        //touch.step();
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

/* ISR for Timer 2 Compare Interrupt */
ISR(TIMER2_COMPA_vect)
{
  TCNT2 = 0; // reset register
  buz_toggle = !buz_toggle;
  digitalWrite(bakiwi::io::BUZZER, buz_toggle && buzzer_on);

  ++pwmcnt;
  pwmcnt %= 16; // <-- think about making this as a function of frequency
  digitalWrite( 3, pwmcnt < led1pwm);
  digitalWrite(11, pwmcnt < led2pwm);
}

/* prescaler table */
constexpr uint16_t presc[8] = {0,1,8,32,64,128,256,1024};

void set_timer_2(uint8_t pre, uint8_t ocr) {
    noInterrupts();       // disable all interrupts
    TCCR2A = (1<<WGM21);  // CTC mode
    TCCR2B = pre;         // set prescaler
    OCR2A = ocr;          // set timer compare register
    TIMSK2 = (1<<OCIE2A); // enable compare interrupt
    interrupts();         // enable all interrupts
}
 
/* determine the timer settings for desired frequency */ 
void set_freq(float frq)
{
  frq = clip(frq, 31, 20000);
  uint8_t i = 0;
  uint8_t p;
  unsigned ocr;

  do {
    ++i;
    ocr = round(F_CPU / frq / 2 / presc[i] - 1);
    p = i;
  }
  while (ocr > 255 && i < 8);

  set_timer_2(p, ocr);

  /*

  Lowest possible frequency is 31 Hz.


  example:
  F_CPU = 16000000Hz (16MHz)
  frq = 440Hz
  hence, we need to toggle buzzer pin at 2*frq = 880Hz

  ocr(1) = 16000000/880/1   - 1 = 18180 > 255 ? yes
  ocr(2) = 16000000/880/8   - 1 =  2271 > 255 ? yes
  ocr(3) = 16000000/880/32  - 1 =   567 > 255 ? yes
  ocr(4) = 16000000/880/64  - 1 =   283 > 255 ? yes
  ocr(5) = 16000000/880/128 - 1 =   141 > 255 ? no
  no --> p = 5 (b101), ocr = 141

  */
}


} /* namespace bakiwi */


#endif /* BAKIWI_REV2_H */
