#ifndef JCL_LED_H
#define JCL_LED_H

/*-----------------------+
 | Simple LED Helper     |
 | Jetpack Cognition Lab |
 | Matthias Kubisch      |
 | June 5th 2020         |
 +-----------------------*/

namespace jcl {

template <uint8_t PIN>
class LED {
  public:

  void init() { pinMode(PIN, OUTPUT);    }
  void on()   { digitalWrite(PIN, HIGH); }
  void off()  { digitalWrite(PIN, LOW ); }

  void toggle() { digitalWrite(PIN, !digitalRead(PIN)); }
  void pwm(uint8_t val) { analogWrite(PIN, val); }
  
};

} /* namespace jcl */


#endif /* JCL_LED_H */
