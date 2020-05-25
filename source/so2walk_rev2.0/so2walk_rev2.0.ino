/// Bakiwi ///

/*---------------------------------+
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | software version 0.01           |
 | for board rev 2.0               |
 | October 9th, 2019               |
 +---------------------------------*/

/* DESCRIPTION.
 * This walking behavior is based on neural oscillators, so-called SO(2) networks.
 * Two recurrent neurons are coupled with the weights configured as a rotation matrix,
 * thus forming an approximated sin/cos waveform pair.
 * Based on the core oscillation, the motors are fed by this pattern generators' signals
 * which are mixed for adjusting phaseshift and amplified by readings from potentiometers.
 *
 * WARNING: Do NOT connect motors when using power from USB-port only.
 * This might overload the supply voltage of your computers USB port.
 */


/* TODOs:
 *  + buzzer: introduce volume by setting freq. 10x higher and use pwm mode for buzzer.
 *  + button: use proper hysteresis neuron (clip)
 */



#include "bakiwi_rev2.0.h"
#include "neural_osc.h"


bakiwi::Board board(42);
NeuralOscillator osc;
unsigned long timestamp = 0; /* timer variable */


void setup() {
  Serial.begin(1000000); /* 1MBaud */
  board.init();
  osc.set_frequency(board.freq);
  osc.restart();

  /* play totoro */
  board.buzzer.play_melody();
}

void loop()
{
  /* wait until timer signals the beginning of the next time slot */
  while(micros() - timestamp < (bakiwi::constants::WAIT_US));
  unsigned long dt = micros() - timestamp;
  timestamp = micros();

  /* begin execution of step functions */
  board.step(osc);

  /* propagate neural oscillator */
  osc.set_frequency(board.freq);
  osc.step();

  /* create phase shifted motor signals */
  const float u1 = osc.x1;
  const float u2 = osc.get_phase_shifted_x1(board.phase);

  board.set_motors(u1, u2);
  board.set_lights(u1, u2);


  Serial.print(dt/1000); Serial.print(" ");
  Serial.print(0.1f*board.motion.gx); Serial.print(" ");
  Serial.print(board.rangef.dx); Serial.print(" ");
  Serial.print(board.brgt); Serial.print(" ");
  Serial.print(board.touch); Serial.print(" ");

  Serial.println();

}
