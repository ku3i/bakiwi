/* SO(2)-Walker for Plops
 * 
 * written by Matthias Kubisch
 * Version for plops 0.01
 * Oct 30th, 2019
 * 
 *
 * TODO: 
 * + correct amplitude when playing a low freq. (amp is very low here)
 * + create board file ad cleanup code
 */


#include "jcl_tinyservo.h"
#include "jcl_switchpot.h"
#include "jcl_neural_osc.h"
//#include "jcl_plops.h"


/* LED + button */
#define LEDBTN 2

/* motors */
#define MOTOR_1 0
#define MOTOR_2 1

/* potis */
#define POTI_1 A2
#define POTI_2 A3

#define PREAMP 1.4f
#define MAX_ANGLE 90
#define WAIT_US 20000

/* timing */
unsigned long timestamp = 0;

bool pressed = false;
volatile bool paused = true;

jcl::Tinyservo motor_1;
jcl::Tinyservo motor_2;

jcl::SwitchPot poti_1(POTI_1, /*init1:*/.5f, /*init1:*/.5f); //amplitude
jcl::SwitchPot poti_2(POTI_2, /*init1:*/.5f, /*init2:*/.5f); //freq+phase

jcl::NeuralOscillator osc;

bool button_pressed() {
  static int integ = 0;
  pinMode(LEDBTN, INPUT_PULLUP);
  bool buttonstate = !digitalRead(LEDBTN);
  pinMode(LEDBTN, OUTPUT);

  integ += buttonstate ? 1 : -1;
  integ = constrain(integ, 0, 10);
  
  return integ>=5;
}

void setup() {
  /* interrupt for refreshing the servos */
  OCR0A = 0x7F;            
  TIMSK |= _BV(OCIE0A);    // enable compare interrupt (see below)
  
  pinMode(LEDBTN, OUTPUT);
  
  osc.set_frequency(poti_2.get_1st());
  osc.restart();
}

void loop() {

  /* pause, if button was pressed before and then released */
  bool state = button_pressed();
  if (pressed and !state)
  {
    if (paused) {
      paused = false;
      motor_1.attach(MOTOR_1);
      motor_2.attach(MOTOR_2);      
      poti_1.set(false);
      poti_2.set(false);
      osc.restart();
    } else {
      paused = true;
      
      motor_1.detach();
      motor_2.detach();
      poti_1.set(true);
      poti_2.set(true);
    }
  }
  pressed = state;

  poti_1.step();
  poti_2.step();

  /* propagate neural oscillator */
  osc.set_frequency(poti_2.get_1st());
  osc.step();
  

  /* read desired phase and create phase shifted motor signals */
  const float phase = 2*poti_2.get_2nd() - 1.f;

  /* create phase shifted motor signals */
  const float u1 = osc.x1;
  const float u2 = osc.get_phase_shifted_x1(phase);

  /* read desired amplitudes */
  const float lev = poti_1.get_1st();
  const float dif = poti_1.get_2nd();
  
  const float amp1 = PREAMP * lev * dif;
  const float amp2 = PREAMP * lev *(1.f - dif);

  /* calc motor outputs */
  const uint8_t out_1 = constrain(round( u1*amp1*MAX_ANGLE ) + MAX_ANGLE, 0, 2*MAX_ANGLE);
  const uint8_t out_2 = constrain(round( u2*amp2*MAX_ANGLE ) + MAX_ANGLE, 0, 2*MAX_ANGLE);

  /* write motors */
  if (!paused) {
    motor_1.write(out_1);
    motor_2.write(out_2);
  }

  /* set LED output */
  if (u1 > .0f) {
    pinMode(LEDBTN, INPUT_PULLUP);
  }
  else {
    pinMode(LEDBTN, OUTPUT); 
    digitalWrite(LEDBTN, LOW);
  }
  
  /* loop delay, wait until timer signals next 10ms slot is done */
  while(micros() - timestamp < WAIT_US);
  timestamp = micros();
  
}


/* gets called once every 2 milliseconds */ 
volatile uint8_t dt = 0;

SIGNAL(TIMER0_COMPA_vect)
{  
  ++dt;
  if (dt >= 10) { // apply the servos every 20 milliseconds
    dt = 0;
    if (!paused) {
      motor_1.apply();
      motor_2.apply();
    }
  }
}
