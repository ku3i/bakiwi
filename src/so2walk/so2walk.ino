/* SO(2)-Walker
 * 
 * written by Matthias Kubisch
 * Version 0.9
 * May 20th, 2019
 * 
 * This walking behavior is based on neural oscillators, so-called SO(2) networks.
 * Two recurrent neurons are coupled with the weights configured as a rotation matrix, 
 * thus forming an approximated sin/cos waveform pair.
 * Based on the core oscillation, the motors are fed by this pattern generators' signals 
 * which are mixed for adjusting phaseshift and amplified by readings from potentiometers.
 * 
 * The theoretical basis can be found in this paper:
 * 'SO(2)-Networks as Neural Oscillators' by Frank Pasemann, Manfred Hild and Keyan Zahedi
 */


#include <math.h>
#include <Servo.h>

/* LEDs */
#define LED_1 11
#define LED_2 5

/* button */
#define TACTBUTTON 12

/* potis */
#define POTI_1 A3
#define POTI_2 A2
#define POTI_3 A5
#define POTI_4 A7

/* motors */
#define MOTOR_1 9 
#define MOTOR_2 10
#define MOTOR_3 3
#define MOTOR_4 6

#define PREAMP 1.4f
#define MAX_ANGLE 90
#define WAIT_MS 20

/* weights */
float w11,w12,w21,w22;

float x1 = .0001f; /* a little grain to start the oscillation */
float x2 = .0f;

bool pressed = false;
bool paused = true;

Servo motor_1;
Servo motor_2;

float readpin(uint8_t pin) {
  return analogRead(pin) / 1023.f;
}

bool button_pressed() {
  static int integ = 0;
  bool buttonstate = !digitalRead(TACTBUTTON);
  digitalWrite(LED_BUILTIN, integ>=5);

  integ += buttonstate ? 1 : -1;
  integ = constrain(integ, 0, 10);
  
  return integ>=5;
}

void update_weights()
{
  /*              
   (x1,x2)^T  <-- tanh[ (1+eps) * r(dp) * (x1,x2)^T ]

   r(dp) = [ cos(dp), sin(dp) 
            -sin(dp), cos(dp) ]

  */
  
  const float val = readpin(POTI_1);
  
  /* stepsize of oscillator (frequency) */
  const float dp = 3.1415f/(8.f + 42.f*val);  // pi/8 .. pi/50

  /* add non-linearity */
  const float k = 1.0f + 1.5f*dp; // useful range: 1 + 1dp ... 1 + 2dp 

  /* determine self and ring coupling */
  const float s = cos(dp) * k;
  const float r = sin(dp) * k;

  /* rotation matrix */
  w11 =  s; w12 = r;
  w21 = -r; w22 = s;
}

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(TACTBUTTON, INPUT_PULLUP);

  update_weights();
}

void start_oscillation() {
  x1 = .0001;
  x2 = .0;
}

void loop() {

  /* pause, if button was pressed before and then released */
  bool state = button_pressed();
  if (pressed and !state)
  {
    if (paused) {
      motor_1.attach(MOTOR_1);
      motor_2.attach(MOTOR_2);
      start_oscillation();
      paused = false;
    } else { 
      motor_1.detach();
      motor_2.detach();
      paused = true;
    }
  }
  pressed = state;
  
  /* propagate neural oscillator */
  update_weights();
  const float y1 = tanh(w11*x1 + w12*x2);
  const float y2 = tanh(w21*x1 + w22*x2);
  x1 = y1; 
  x2 = y2;

  /* read desired phase and create phase shifted motor signals */
  const float phase = 2*readpin(POTI_3) - 1.f;
  const float u1 = x1;
  const float u2 = x2*phase + x1*(1.f - fabs(phase));

  /* read desired amplitudes */
  const float amp1 = PREAMP * readpin(POTI_2);
  const float amp2 = PREAMP * readpin(POTI_4);

  /* calc motor outputs */
  const uint8_t out_1 = constrain(round( u1*amp1*MAX_ANGLE ) + MAX_ANGLE, 0, 2*MAX_ANGLE);
  const uint8_t out_2 = constrain(round( u2*amp2*MAX_ANGLE ) + MAX_ANGLE, 0, 2*MAX_ANGLE);

  /* write motors */
  if (!paused) {
    motor_1.write(out_1);
    motor_2.write(out_2);
  }

  /* set LED outputs */
  digitalWrite(LED_1, u1 >.0);
  digitalWrite(LED_2, u2 >.0);
  
  /* loop delay */
  //Serial.println(out_1);
  delay(WAIT_MS);
}
