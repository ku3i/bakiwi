#ifndef JCL_SERVO_H
#define JCL_SERVO_H

/*---------------------------------+
 | Minimal Servo Lib               |
 | Jetpack Cognition Lab           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | June 5th 2020                   |
 +---------------------------------*/


#include <avr/interrupt.h>
#include "Arduino.h"
#include "jcl_modules.h"

/* This is a minimal two-servo class for attiny84/atmega328p (Bakiwi Kit)
 * It is NOT meant as a full library. Parts of the code is inspired by servo
 * libs with contributions of Jim Studt, David A. Mellis, and Paul Stoffregen. */

#if defined(__AVR_ATtiny84__)
 #define SERVO_PIN_A 6
 #define SERVO_PIN_B 5
#elif defined(__AVR_ATmega328P__)
  #define SERVO_PIN_A 9
  #define SERVO_PIN_B 10
#else
  static_assert(false, "no servo pins assinged");
#endif


namespace jcl {

/*-------------------------------------------+
 |  default pulse lengths [in us] for degree |
 |   0: 544                                  |
 | 180: 2400                                 |
 +-------------------------------------------*/

const uint16_t pulse_min = 544;
const uint16_t pulse_max = 2400;
const uint16_t a = clockCyclesPerMicrosecond() * pulse_min;
const uint16_t b = clockCyclesPerMicrosecond() * (pulse_max - pulse_min) / 180;


class JCLServoBase {
  static bool is_attached_A, is_attached_B;
};


bool JCLServoBase::is_attached_A = false;
bool JCLServoBase::is_attached_B = false;


template <uint8_t PIN>
class JCLServo : public JCLServoBase
{

  static void init_timer(void)
  {
    const uint8_t defSREG = SREG;

    cli();
    TCCR1A = _BV(WGM11);
    TCCR1B = _BV(WGM12)
           | _BV(WGM13)  /* Mode: Fast PWM, ICR1 is top */
           | _BV(CS11);  /* Clock Prescaler is /8       */

    OCR1A = 3000;
    OCR1B = 3000;
    ICR1  = clockCyclesPerMicrosecond()*(20000L/8);  /* ICR1 is 16bit, so its value is
                                                        e.g. 16*20000/8 = 40000, for 16MHz clock
                                                        20000us is servo refresh time
                                                        /8 is the prescaler. */

    TIMSK1 &=  ~(_BV(OCIE1A) | _BV(OCIE1B) | _BV(TOIE1) );

    SREG = defSREG;  // restore
  }


 public:

  JCLServo() {
    static_assert(PIN == SERVO_PIN_A or PIN == SERVO_PIN_B);
  }

  /* attaches servo to a pin, sets pin mode and timer output but
   * will not position the servo until a subsequent write() */
  void on(void)
  {
    digitalWrite(PIN, LOW);
    pinMode(PIN, OUTPUT);

    if (!is_attached_A && !is_attached_B) init_timer();

    if (PIN == SERVO_PIN_A) {
      is_attached_A = true;
      TCCR1A = (TCCR1A & ~_BV(COM1A0)) | _BV(COM1A1);
    }

    if (PIN == SERVO_PIN_B) {
      is_attached_B = true;
      TCCR1A = (TCCR1A & ~_BV(COM1B0)) | _BV(COM1B1);
    }
  }


  void off()
  {
    if (PIN == SERVO_PIN_A) {
      is_attached_A = false;
      TCCR1A = TCCR1A & ~_BV(COM1A0) & ~_BV(COM1A1);
      pinMode(PIN, INPUT);
    }

    if (PIN == SERVO_PIN_B) {
      is_attached_B = false;
      TCCR1A = TCCR1A & ~_BV(COM1B0) & ~_BV(COM1B1);
      pinMode(PIN, INPUT);
    }
  }

  /* sets the servos target angle from 0 to 180 deg */
  void set(uint8_t angle)
  {
    clamp(angle, uint8_t{0}, uint8_t{180});

    /* 8L is TCNT1 prescaler, change if the clock's prescaler changes */
    const uint16_t p = (a + b * angle) / 8;

    if (PIN == SERVO_PIN_A) OCR1A = p;
    if (PIN == SERVO_PIN_B) OCR1B = p;
  }


};

} /* namespace jcl */

#endif /* JCL_SERVO_H */
