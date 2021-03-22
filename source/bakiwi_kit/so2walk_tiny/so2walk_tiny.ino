/*---------------------------------+
 | Bakiwi Kit Firmware             |
 | Jetpack Cognition Lab           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | board ver. bakiwi kit rev 1.1   |
 | version 20.6.17                 |
 +---------------------------------*/

/*---WE THANK OUR SUPPORTERS----------------------------------------------+
 | Daniel Sitbon, Thomas Lobig, Sebastian Schmölling, Martin Marmulla,    |
 | Richard Lemke, Jens Priebe, Ines Müller, Stefan Kruse, Thomas Chung,   |
 | Andreas Gerken, Oswald Berthold, Pablo Zöllner, Andreas Pieper,        |
 | Hannes Ziegerhofer, Philip Brechler, Rebecca Mico, Wally WonTaek Chung |
 | Michael Merz, Sian Thomas, Gero, HealthCare Futurists, Bruno Böttcher  |
 | Andrea Mayr-Stalder, Irene Diez, Guido Schillaci, MaggiWuerze,         |
 | Heinrich Mellmann, Christian Thiele, Michael Mehling, Hendrik Schober, |
 | xHain hack+makespace, Mona Orgel, Paula, Thomas Prätzlich,             |
 | Karen Ellmer,                                                          |
 | to be continued                                                        |
 +------------------------------------------------------------------------*/

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
    bakiwi.leds_set_pwm((uint8_t)i, (uint8_t)i);
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

  /* read capacitive sensors (aka antennas) */
  const float cs = bakiwi.cap.get();

  /* reduce frequency when antennas touched */
  const float freq_gain = (cs > 0) ? (1 - 5 * clamp(cs, 0.f, .2f))
                                   : (1 - 0.334 * cs);

  /* reduce amplitude when antennas touched */
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

  /* set amplitudes for front (1) and rear (2) motor */
  const float amp1 = lev * clamp(    dif, .0f, 1.f);
  const float amp2 = lev * clamp(2 - dif, .0f, 1.f);

  /* prepare motor outputs */
  const uint8_t out_1 = clamp((unsigned) (round(u1 * amp1 * DEG90) + DEG90 + bakiwi.bias_1), 0u, 2 * DEG90);
  const uint8_t out_2 = clamp((unsigned) (round(u2 * amp2 * DEG90) + DEG90 + bakiwi.bias_2), 0u, 2 * DEG90);

  /* write motors */
  if (not paused)
    bakiwi.write_motors(out_1, out_2);

  /* prepare LED pwms */
  uint8_t led_1, led_2;
  if (cs < 0.25) { // if antennas not touched
    led_1 = 255 * clamp(u1 * amp1 * 4, 0.f, 1.f); // blink LEDs with oscillator state
    led_2 = 255 * clamp(u2 * amp2 * 4, 0.f, 1.f);
  } else {
    led_1 = 255 * clamp(cs, 0.f, 1.f); // light-up LEDs according to antenna touch
    led_2 = 255 * clamp(cs, 0.f, 1.f);
  }

  bakiwi.leds_set_pwm(led_1, led_2);

  bakiwi.wait_for_next_cycle();
}
