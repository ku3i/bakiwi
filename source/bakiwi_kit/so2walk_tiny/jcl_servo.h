#ifndef JCL_SERVO_H
#define JCL_SERVO_H

#include "jcl_modules.h"
#include <avr/interrupt.h>
#include "Arduino.h"

/* This is a minimal two-servo class for Attiny84 */
// TODO add contribution comment


#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) // Arduino
  #define SERVO_PIN_A 9
  #define SERVO_PIN_B 10
#elif defined(__AVR_ATtiny84__)
 #define SERVO_PIN_A 6
 #define SERVO_PIN_B 5
#else
  #define SERVO_PIN_A 9
  #define SERVO_PIN_B 10
#endif



// pulse length for 0 degrees in microseconds, 544uS default
// pulse length for 180 degrees in microseconds, 2400uS default
const unsigned min16 = 544 / 16;   // minimum pulse, 16uS units  (default is 34)
const unsigned max16 = 2400 / 16; // maximum pulse, 16uS units, 0-4ms range (default is 150)


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
    TCCR1A = _BV(WGM11); /* fast PWM, ICR1 is top */
    TCCR1B = _BV(WGM13)
           | _BV(WGM12)  /* fast PWM, ICR1 is top */
           | _BV(CS11);  /* div 8 clock prescaler */

    OCR1A = 3000;
    OCR1B = 3000;
    ICR1  = clockCyclesPerMicrosecond()*(20000L/8);  /* 20000 us (20ms) is a bit fast for the refresh, but
                                                        it keeps us from overflowing ICR1 at 20MHz clocks
                                                        That "/8" at the end is the prescaler. */

    #if defined(__AVR_ATmega8__)
      TIMSK &= ~(_BV(TICIE1) | _BV(OCIE1A) | _BV(OCIE1B) | _BV(TOIE1) );
    #else
      TIMSK1 &=  ~(_BV(OCIE1A) | _BV(OCIE1B) | _BV(TOIE1) );
    #endif

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
    digitalWrite(PIN, LOW); //check order?
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
    jcl::clamp(angle, uint8_t{0}, uint8_t{180});

    // TODO clarify this comment: 
    // bleh, have to use longs to prevent overflow, could be tricky if always a 16MHz clock, but not true
    // That 8L on the end is the TCNT1 prescaler, it will need to change if the clock's prescaler changes,
    // but then there will likely be an overflow problem, so it will have to be handled by a human.
    uint16_t p = (min16*16L*clockCyclesPerMicrosecond() + (max16-min16)*(16L*clockCyclesPerMicrosecond())*angle/180L)/8L;
    if (PIN == SERVO_PIN_A) OCR1A = p;
    if (PIN == SERVO_PIN_B) OCR1B = p;
  }

         
};


#endif /* JCL_SERVO_H */
