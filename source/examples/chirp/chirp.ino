
#define POTI_1 A7
#define BUZ    3
#define LED 13


float w11,w12,w21,w22;

float x1 = .0f;
float x2 = .0f;

float frq = .0;

const unsigned frq_start = 6000;
const unsigned frq_end = 4000;

unsigned frq_e = 4000;
float decayf = 0.99;
float dur = 0.01;


float readpin(uint8_t pin) {
  return analogRead(pin) / 1023.f;
}


void update_weights(float val)
{
  /* stepsize of oscillator (frequency) */
  const float dp = 3.1415f/(0.1f + 200.f*val);  // pi/8 .. pi/50

  /* add non-linearity */
  const float k = 1.0f + 1.5f*dp; // useful range: 1 + 1dp ... 1 + 2dp

  /* determine self and ring coupling */
  const float s = cos(dp) * k;
  const float r = sin(dp) * k;

  /* rotation matrix */
  w11 =  s; w12 = r;
  w21 = -r; w22 = s;
}


void start_oscillation() {
  x1 = .00001f; /* a little grain to (re-)start the oscillation */
  x2 = .0f;
}

void setup() {
  Serial.begin(1000000);
  pinMode(BUZ, OUTPUT);
  pinMode(LED, OUTPUT);
  randomSeed(analogRead(POTI_1));

  frq = frq_start;
  update_weights(0.1);
  start_oscillation();
}



void loop() {
  float val = readpin(POTI_1);

  /* update oscillator */
  const float y1 = tanh(w11*x1 + w12*x2);
  const float y2 = tanh(w21*x1 + w22*x2);
  x1 = y1;
  x2 = y2;

  frq *= decayf;

  if (frq > frq_e) {
    digitalWrite(LED, HIGH);
    tone(BUZ, frq + 300 * x1); // play tone of modulated frequency
  }
  else {
    digitalWrite(LED, LOW);
    noTone(BUZ);
  }

  if (frq < dur) {
    float offset = val*frq_start/6;
    frq = frq_start + random(frq_start/10) - offset;
    frq_e = frq_end + random(frq_start/10) - offset;
    dur = random(10,1000);
    if (dur <= 15)
      delay(random(3,20)*1000);
    decayf = random(985,990)/1000.f;
    start_oscillation();
  }

  delay(2); // 50Hz
  //Serial.println(frq + 100 * x2);
}
