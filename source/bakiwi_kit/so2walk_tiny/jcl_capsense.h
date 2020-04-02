#include <CapacitiveSensor.h>

/*-----------------------+
 | CapSense              |
 | Jetpack Cognition Lab |
 | Matthias Kubisch      |
 | v.0.1 Dec. 19th 2019  |
 +-----------------------*/

/*TODO add writing and reading values to/from EEPROM*/

namespace jcl {

class CapSense {
  CapacitiveSensor cs;

public:

  float x=.0f,y,b=.0f,w;
  const uint8_t NREAD = 8; /* each reading approx. takes 0.3 ms */
  const unsigned timeout_ms = 2000; /* 2000 is default */
  const float eta = 0.005f;
  const float eta_w = .0000005;
  const float lp = 0.1f;


  CapSense(uint8_t sendpin, uint8_t recvpin)
  : cs(sendpin, recvpin)
  , y(), w(0.01f), b()
  {
    cs.set_CS_AutocaL_Millis(0xFFFFFFFF);
    cs.set_CS_Timeout_Millis(timeout_ms);
    x = _read();
    b = -w*x;
  }

  float step(void) {
    x = lp*_read() + (1-lp)*x;
    y = tanh(w*x + b); /* single tanh neuron */

    /* low-pass out the bias/DC component */
    //b += -eta*y;
    b += (b > -w*x) ? -eta*y: -10*eta*y;
    
    /* decrease weight w when amp is near saturation,
       or increase w when amp is too lower */
    if (fabs(y) > .6f) w *= 1-eta_w;
    if (fabs(y) < .4f) w *= 1+eta_w;

    return y;
  }

  float get(void) const { return y; }

private:
  float _read(void) { return cs.capacitiveSensorRaw(NREAD)/NREAD; }

};


} /* namespace jcl */
