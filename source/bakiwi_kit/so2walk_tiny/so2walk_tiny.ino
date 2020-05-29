/* SO(2)-Walker for Bakiwi Kit
 * 
 * written by Matthias Kubisch
 * Version for bakiwi kit 0.1
 * May 29th, 2020
 * 
 *
 * TODO: 
 * + correct amplitude when playing a low freq. (amp is very low here)
 * + rewrite servo lib
 */

/* Note: Using the attiny layout by Davis A. Mellis
   https://github.com/damellis/attiny

   ATMEL ATTINY84 / ARDUINO
  
                             +-\/-+
                       VCC  1|    |14  GND
               (D 10)  PB0  2|    |13  AREF (D  0)
               (D  9)  PB1  3|    |12  PA1  (D  1) 
                       PB3  4|    |11  PA2  (D  2) 
    PWM  INT0  (D  8)  PB2  5|    |10  PA3  (D  3) 
    PWM        (D  7)  PA7  6|    |9   PA4  (D  4) 
    PWM        (D  6)  PA6  7|    |8   PA5  (D  5)        PWM
                             +----+
*/

#include "jcl_neural_osc.h"
#include "jcl_bakiwi_kit_rev_1_1.h"


/* misc */
#define PREAMP 1.0
#define MAX_ANGLE 90


bool pressed = false;
volatile bool paused = true;

using namespace jcl;

NeuralOscillator osc;
BakiwiKit        bakiwi;

void setup() {
  
  bakiwi.init(); /* initialize bakiwi board */

  osc.set_frequency(bakiwi.get_frq());
  osc.restart();

  /* start LED boot-up sequence */
  for (float i = 255; i > 2; i *= 0.95) {
    bakiwi.led_set_pwm((uint8_t)i, (uint8_t)i);
    delay(5);
  }
  
  bakiwi.leds_off();
}

void loop() {

  bakiwi.step();

  /* pause, if button was pressed before and then released */
  bool state = bakiwi.button_pressed();
  if (pressed and !state)
  {
    if (paused) {
      paused = false;
      bakiwi.motors_on();     
      osc.restart();
    } else {
      paused = true;
      bakiwi.motors_off();
    }
  }
  pressed = state;

  const float cs = bakiwi.cap.get();

  const float freq_gain = (cs > 0) ? (1 - 5 * clip(cs, 0, 0.2)) 
                                   : (1 - 0.334 * cs);
                                 
  const float ampl_gain = 1 - clip(cs);

  /* propagate neural oscillator */
  osc.set_frequency(freq_gain * bakiwi.get_frq());
  osc.step();

  /* read phase */
  const float phase = 2 * bakiwi.get_phs() - 1;

  /* create phase shifted motor signals */
  const float u1 = osc.x1;
  const float u2 = osc.get_phase_shifted_x1(phase);

  /* read amplitude and balance */
  const float lev = bakiwi.get_amp() * ampl_gain;
  const float dif = 2 * bakiwi.get_bal();
  
  const float amp1 = PREAMP * lev * clip(    dif);
  const float amp2 = PREAMP * lev * clip(2 - dif);

  /* calc motor outputs */
  const uint8_t out_1 = constrain(round(u1 * amp1 * MAX_ANGLE) + MAX_ANGLE + bakiwi.bias_1, 0, 2 * MAX_ANGLE);
  const uint8_t out_2 = constrain(round(u2 * amp2 * MAX_ANGLE) + MAX_ANGLE + bakiwi.bias_2, 0, 2 * MAX_ANGLE);

  /* write motors */
  if (!paused)
    bakiwi.write_motors(out_1, out_2);

  
  /* set LED pwms */
  uint8_t led_1,led_2;
  if (cs < 0.25) {
    led_1 = 255 * clip(u1 * amp1 * 4);
    led_2 = 255 * clip(u2 * amp2 * 4);
  } else {
    led_1 = 255 * clip(cs);
    led_2 = 255 * clip(cs);
  }
  
  bakiwi.led_set_pwm(led_1, led_2);

  
  bakiwi.wait_for_next_cycle(); /* wait for next timeslot */
}
