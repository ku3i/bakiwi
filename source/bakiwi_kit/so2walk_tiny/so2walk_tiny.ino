/* SO(2)-Walker for Bakiwi Kit
 * 
 * written by Matthias Kubisch
 * Version for bakiwi kit 0.01
 * Nov 9th, 2019
 * 
 *
 * TODO: 
 * + correct amplitude when playing a low freq. (amp is very low here)
 * + create board file ad cleanup code
 * + improve button response
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

#include <EEPROM.h>
#include <PWMServo.h>
#include "jcl_capsense.h"
#include "jcl_neural_osc.h"


/* LEDs + button */
#define LED_1 8
#define LED_2 A7
#define BUTTON 10

/* motors */
#define MOTOR_1 A6
#define MOTOR_2 A5

/* potis */
#define POTI_FREQ A0
#define POTI_AMP1 A1
#define POTI_AMP2 A2
#define POTI_PHSE A3

/* capsense */
#define CAPSEND 9
#define CAPRECV 4

/* misc */
#define PREAMP 1.0f // 1.4f
#define MAX_ANGLE 90

/* timing */
#define WAIT_CYCLE_US 20000
#define WAIT_CONFIG_TIMEOUT_MS 100 

/* eeprom adresses */
#define MEM_OFFSET_1 22
#define MEM_OFFSET_2 23

/* timing */
unsigned long timestamp = 0;

bool pressed = false;
volatile bool paused = true;

PWMServo motor_1;
PWMServo motor_2;


int8_t offset_1 = 0;
int8_t offset_2 = 0;

using namespace jcl;

CapSense cap = CapSense(CAPSEND,CAPRECV);
NeuralOscillator osc;


float readpin(uint8_t pin) { return analogRead(pin) / 1023.f; }


bool button_pressed(uint8_t N = 3) {
  static int integ = 0;
  bool buttonstate = !digitalRead(BUTTON);
 
  integ += buttonstate ? 1 : -1;
  integ = constrain(integ, 0, 2*N);
  
  return integ>=N;
}

void configuration_routine(void)
{
  while(button_pressed())
  {
    digitalWrite(LED_1, !digitalRead(LED_1));
    digitalWrite(LED_2, !digitalRead(LED_2));
    delay(64); 
  }
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);

  /* configure 1st motor */
  int8_t offset = 0;
  motor_1.attach(MOTOR_1);
  while(!button_pressed()) {
    digitalWrite(LED_1, !digitalRead(LED_1));
    delay(20); 
    offset = 32*readpin(POTI_PHSE)-16;
    motor_1.write(90 + offset);
  }
  motor_1.detach();
  digitalWrite(LED_1, LOW);
  offset_1 = offset;  
  EEPROM.write(MEM_OFFSET_1, offset_1);

  while(button_pressed()) {} // wait for button release

  /* configure 2nd motor */
  motor_2.attach(MOTOR_2);
  while(!button_pressed()) {
    digitalWrite(LED_2, !digitalRead(LED_2));
    delay(20); 
    offset = 32*readpin(POTI_PHSE)-16;
    motor_2.write(90 + offset);
  }
  motor_2.detach();
  digitalWrite(LED_2, LOW);
  offset_2 = offset;
  EEPROM.write(MEM_OFFSET_2, offset_2);
  
}

void setup() {
  
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);

  bool config_mode = false;
  while(millis() < WAIT_CONFIG_TIMEOUT_MS) { config_mode = button_pressed(8); delay(10); }

  if (config_mode)
    configuration_routine();
  else {
    offset_1 = EEPROM.read(MEM_OFFSET_1);
    offset_2 = EEPROM.read(MEM_OFFSET_2);
  }
  
  osc.set_frequency(readpin(POTI_FREQ));
  osc.restart();

  /* LED start greeting sequence */
  for (uint8_t i = 0; i < 128; ++i) {
    analogWrite(LED_1, i);
    analogWrite(LED_2, i);
    delay(16);
  }
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, HIGH);
  delay(500);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
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
      osc.restart();
    } else {
      paused = true;
      motor_1.detach();
      motor_2.detach();
    }
  }
  pressed = state;

  const float cs = cap.step();

  const float freq_gain = (cs>0) ? (1.0f - 5*clip(cs, 0.f, .2f)) 
                                 : (1.0f - 0.334f*cs);
                                 
  const float ampl_gain = 1.0f - clip(cs, 0.f, 1.f);

  /* propagate neural oscillator */
  osc.set_frequency(freq_gain*readpin(POTI_FREQ));
  osc.step();

  /* read desired phase and create phase shifted motor signals */
  const float phase = 2*readpin(POTI_PHSE) - 1.f;

  /* create phase shifted motor signals */
  const float u1 = osc.x1;
  const float u2 = osc.get_phase_shifted_x1(phase);

  /* read desired amplitudes */
  const float lev = readpin(POTI_AMP1)*ampl_gain;
  const float dif = 2*readpin(POTI_AMP2);
  
  const float amp1 = PREAMP * lev * clip(dif, 0.f, 1.f);
  const float amp2 = PREAMP * lev * clip(2.f - dif, 0.f, 1.f);

  /* calc motor outputs */
  const uint8_t out_1 = constrain(round( u1*amp1*MAX_ANGLE ) + MAX_ANGLE + offset_1, 0, 2*MAX_ANGLE);
  const uint8_t out_2 = constrain(round( u2*amp2*MAX_ANGLE ) + MAX_ANGLE + offset_2, 0, 2*MAX_ANGLE);

  /* write motors */
  if (!paused) {
    motor_1.write(out_1);
    motor_2.write(out_2);
  }


  uint8_t pwm_1,pwm_2;
  
  /* set LED pwms */
  if (cs < 0.25) {
    pwm_1 = 255 * clip(u1 * amp1 * 4, .0f, 1.f);
    pwm_2 = 255 * clip(u2 * amp2 * 4, .0f, 1.f);
  } else {
    pwm_1 = 255 * clip(cs, .0f, 1.f);
    pwm_2 = 255 * clip(cs, .0f, 1.f);
  }

  analogWrite(LED_1, pwm_1);
  analogWrite(LED_2, pwm_2);

  /* loop delay, wait until timer signals next time slot is done */
  while(micros() - timestamp < WAIT_CYCLE_US);
  timestamp = micros();
  
}
