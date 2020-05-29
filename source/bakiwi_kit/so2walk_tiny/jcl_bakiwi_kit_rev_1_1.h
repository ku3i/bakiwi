#ifndef JCL_BAKIWI_KIT_REV_1_1_H
#define JCL_BAKIWI_KIT_REV_1_1_H


#include <PWMServo.h>
#include <EEPROM.h>

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

/* eeprom adresses */
const uint8_t MEM_OFFSET_1 = 22;
const uint8_t MEM_OFFSET_2 = 23;
const uint8_t MEM_CAP_GAIN = 21;

/* timing */
const unsigned WAIT_CYCLE_US = 20000;
const unsigned WAIT_CONFIG_TIMEOUT_MS = 100;



class BakiwiKit {

  /* timing */
  unsigned long timestamp = 0;
  int8_t button_integ = 0;

public:

  int8_t bias_1 = 0;
  int8_t bias_2 = 0;
  
  PWMServo motor_1 = {};
  PWMServo motor_2 = {};
  CapSense cap;

  LED<LED_1> led_1;
  LED<LED_2> led_2;


  BakiwiKit() : cap(CAPSEND,CAPRECV) {}

  void step(void) { 
    cap.step(); 
    //last todo: regularily update the eeprom value for wgain  
  }

  void init(void) {
    pinMode(BUTTON, INPUT_PULLUP);
    led_1.init();
    led_2.init();

    /* check for config mode */
    if (button_pressed_for_ms(WAIT_CONFIG_TIMEOUT_MS))
      configuration_routine();
    else {
      bias_1 = EEPROM.read(MEM_OFFSET_1);
      bias_2 = EEPROM.read(MEM_OFFSET_2);
    }

    /* read capacitive sense gain from memory and check for valid value 
       if no valid value is in memory, flash default value. */
    uint8_t cap_gain = EEPROM.read(MEM_CAP_GAIN);
    
    if (255 == cap_gain) { // no value written yet
      cap_gain = cap.get_weight();
      EEPROM.update(MEM_CAP_GAIN, cap_gain);
      led_1.on();
      delay(1000);
    } else {
      cap.set_weight(cap_gain);
    }
  }

  
  float readpin(uint8_t pin) { return analogRead(pin) / 1023.f; }

  float get_amp() { return readpin(POTI_AMP); }
  float get_bal() { return readpin(POTI_BAL); }
  float get_frq() { return readpin(POTI_FRQ); }
  float get_phs() { return readpin(POTI_PHS); }



  bool button_pressed(uint8_t N = 3)
  {  
    bool buttonstate = !digitalRead(BUTTON);
 
    button_integ += buttonstate ? 1 : -1;
    button_integ = clip(button_integ, 0, 2*N);
  
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
    motor_1.write(out_1);
    motor_2.write(out_2);
  }

  void motors_on(void) {
    motor_1.attach(MOTOR_1);
    motor_2.attach(MOTOR_2); 
  }

  void motors_off(void) {
    motor_1.detach();
    motor_2.detach();
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

    int8_t offset = 0;

    /* configure 1st motor */
    motor_1.attach(MOTOR_1);
    while(not button_pressed()) {
      led_1.toggle();
      delay(20); 
      offset = 32*readpin(POTI_PHS)-16;
      motor_1.write(90 + offset);
    }
    wait_for_button_released();
  
    motor_1.detach();
    led_1.off();
    EEPROM.write(MEM_OFFSET_1, offset);
    bias_1 = offset;

    /* configure 2nd motor */
    motor_2.attach(MOTOR_2);
    while(not button_pressed()) {
      led_2.toggle();
      delay(20); 
      offset = 32*readpin(POTI_PHS)-16;
      motor_2.write(90 + offset);
    }
    wait_for_button_released();
  
    motor_2.detach();
    led_2.off();
    EEPROM.write(MEM_OFFSET_2, offset);
    bias_2 = offset;
}

 
};


} /* namespace jcl */

#endif /* JCL_BAKIWI_KIT_REV_1_1_H */
