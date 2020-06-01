#ifndef JCL_BAKIWI_KIT_REV_1_1_H
#define JCL_BAKIWI_KIT_REV_1_1_H


#include <avr/eeprom.h>

#include "jcl_servo.h"
#include "jcl_modules.h"
#include "jcl_capsense.h"
#include "jcl_led.h"

namespace jcl {

/* board's pin layout */
const uint8_t LED_1    =  8;
const uint8_t LED_2    = A7;
const uint8_t BUTTON   = 10;

const uint8_t POTI_FRQ = A0;
const uint8_t POTI_AMP = A1;
const uint8_t POTI_BAL = A2;
const uint8_t POTI_PHS = A3;

const uint8_t MOTOR_1  = A6;
const uint8_t MOTOR_2  = A5;

const uint8_t CAPSEND  =  9;
const uint8_t CAPRECV  =  4;

/* memory layout */
const uint8_t MEM_CAP_SIZE = 20; // starts from address 0
const uint8_t MEM_CAP_SLOT = 21;

const uint8_t MEM_OFFSET_1 = 22;
const uint8_t MEM_OFFSET_2 = 23;

const uint8_t MEM_STARTS   = 24; //+25, number of starts is uint16_t



/* timing */
const unsigned WAIT_CYCLE_US = 20000;
const unsigned WAIT_CONFIG_TIMEOUT_MS = 100;

/* misc */
const unsigned DEG90 = 90;


constexpr unsigned cycles_per_minute = 60000 /*1min in ms*/ / (WAIT_CYCLE_US/1000) /*ms*/;

#define CONFIG_MOTOR(N)                       \
{                                             \
  int8_t offset = 0;                          \
  motor_##N.on();                             \
  while(not button_pressed()) {               \
    led_##N.toggle();                         \
    delay(20);                                \
    offset = 32*readpin(POTI_PHS)-16;         \
    motor_##N.set(90 + offset);               \
  }                                           \
  wait_for_button_released();                 \
  motor_##N.off();                            \
  led_##N.off();                              \
  eeprom_update_byte(MEM_OFFSET_##N, offset); \
  bias_##N = offset;                          \
}                                             \


class BakiwiKit {

  /* timing */
  unsigned long timestamp = 0;
  int button_integ = 0;

  unsigned cycles = 0;
  uint8_t slot_id = 0;

public:

  int8_t bias_1 = 0;
  int8_t bias_2 = 0;
  
  JCLServo<MOTOR_1> motor_1 = {};
  JCLServo<MOTOR_2> motor_2 = {};
  CapSense cap;

  LED<LED_1> led_1;
  LED<LED_2> led_2;


  BakiwiKit() : cap(CAPSEND,CAPRECV) {
    static_assert(cycles_per_minute == 3000);
  }

  void step(void) { 
    cap.step(); 

    /* regularily update the eeprom value for wgain */
    if (cycles++ >= cycles_per_minute) {
      //led_2.on();
      //delay(500);
      eeprom_update_byte(slot_id, cap.get_weight());
      cycles=0;
    }
  }

  void init(void) {
    pinMode(BUTTON, INPUT_PULLUP);
    led_1.init();
    led_2.init();


    eeprom_busy_wait();
    /* check for config mode */
    if (button_pressed_for_ms(WAIT_CONFIG_TIMEOUT_MS))
      configuration_routine();
    else {
      bias_1 = eeprom_read_byte(MEM_OFFSET_1);
      bias_2 = eeprom_read_byte(MEM_OFFSET_2);
    }

    uint16_t numstarts = eeprom_read_word(MEM_STARTS);
    eeprom_write_word(MEM_STARTS, ++numstarts);

    /* read capacitive sense gain from memory and check for valid value 
       if no valid value is in memory, flash default value. */
    slot_id = eeprom_read_byte(MEM_CAP_SLOT); // read last slot_id from memory
    const uint8_t cap_gain = eeprom_read_byte(slot_id);

    if (++slot_id >= MEM_CAP_SIZE) slot_id = 0; // rotate slot_id
    eeprom_write_byte(MEM_CAP_SLOT, slot_id);       // and save to memory
    
    if (255 == cap_gain) { // no gain value written yet
      eeprom_write_byte(slot_id, cap.get_weight());
      led_1.on();
      delay(1000);
    } else {
      cap.set_weight(cap_gain);
    }
  }

  
  float readpin(uint8_t pin) const { return analogRead(pin) / 1023.f; }

  float get_amp() const { return readpin(POTI_AMP); }
  float get_bal() const { return readpin(POTI_BAL); }
  float get_frq() const { return readpin(POTI_FRQ); }
  float get_phs() const { return readpin(POTI_PHS); }



  bool button_pressed(uint8_t N = 3)
  {  
    const bool buttonstate = !digitalRead(BUTTON);
 
    button_integ += buttonstate ? 1 : -1;
    button_integ = clamp(button_integ, 0, 2*N);
  
    return button_integ>=N;
  }

  void led_set_pwm(uint8_t pwm_1, uint8_t pwm_2)
  {
    led_1.pwm(pwm_1);
    led_2.pwm(pwm_2);
  }

  void leds_off(void) {
    led_1.off(); 
    led_2.off();
  }

  void leds_toggle(void) {
    led_1.toggle();
    led_2.toggle();
  }

  void write_motors(uint8_t out_1, uint8_t out_2) {
    motor_1.set(out_1);
    motor_2.set(out_2);
  }

  void motors_on(void) {
    motor_1.on();
    motor_2.on();
  }

  void motors_off(void) {
    motor_1.off();
    motor_2.off();
  }

  /* use for constant loop delay, 
     wait until timer signals next time slot is done */
     
  void wait_for_next_cycle(void) {
    while(micros() - timestamp < WAIT_CYCLE_US);
    timestamp = micros();
  }

  bool button_pressed_for_ms(unsigned int timeout_ms) {
    bool pressed = false;
    while(millis() < timeout_ms) {
      if (button_pressed(8)) pressed = true;
      delay(10); 
    }
    return pressed;
  }

  void wait_for_button_released(void) { 
    while(button_pressed()) { delay(20); }
  }

  void configuration_routine(void)
  {
    while(button_pressed()) {
      leds_toggle();
      delay(64); 
    }
    leds_off();

    CONFIG_MOTOR(1);
    CONFIG_MOTOR(2);
}


};


} /* namespace jcl */

#endif /* JCL_BAKIWI_KIT_REV_1_1_H */
