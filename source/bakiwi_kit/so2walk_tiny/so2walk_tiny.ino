/* SO(2)-Walker for Bakiwi Kit
 * 
 * written by Matthias Kubisch
 * Version for bakiwi kit 0.1
 * May 29th, 2020
 * 
 *
 * TODO: 
 * + correct amplitude when playing a low freq. (amp gets low here)
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

  const float freq_gain = (cs > 0) ? (1 - 5 * clamp(cs, 0.f, .2f))
                                   : (1 - 0.334 * cs);
                                 
  const float ampl_gain = 1 - clamp(cs, 0.f, 1.f);

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
  
  const float amp1 = lev * clamp(    dif, .0f, 1.f);
  const float amp2 = lev * clamp(2 - dif, .0f, 1.f);

  /* calc motor outputs */
  const uint8_t out_1 = clamp((unsigned) (round(u1 * amp1 * DEG90) + DEG90 + bakiwi.bias_1), 0u, 2 * DEG90);
  const uint8_t out_2 = clamp((unsigned) (round(u2 * amp2 * DEG90) + DEG90 + bakiwi.bias_2), 0u, 2 * DEG90);

  /* write motors */
  if (not paused)
    bakiwi.write_motors(out_1, out_2);

  
  /* set LED pwms */
  uint8_t led_1,led_2;
  if (cs < 0.25) {
    led_1 = 255 * clamp(u1 * amp1 * 4, 0.f, 1.f);
    led_2 = 255 * clamp(u2 * amp2 * 4, 0.f, 1.f);
  } else {
    led_1 = 255 * clamp(cs, 0.f, 1.f);
    led_2 = 255 * clamp(cs, 0.f, 1.f);
  }
  
  bakiwi.led_set_pwm(led_1, led_2);

  
  bakiwi.wait_for_next_cycle(); /* wait for next timeslot */
}
