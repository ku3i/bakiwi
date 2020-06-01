#ifndef JCL_CAPSENSE_H
#define JCL_CAPSENSE_H


#include <CapacitiveSensor.h>

/*-----------------------+
 | CapSense              |
 | Jetpack Cognition Lab |
 | Matthias Kubisch      |
 | v.0.1 Dec. 19th 2019  |
 +-----------------------*/


namespace jcl {

class CapSense {
  CapacitiveSensor cs;

public:

  float x,y,b,w;
  const uint8_t NREAD = 8; /* each reading approx. takes 0.3 ms */
  const float eta = 0.005f;
  const float eta_w = .0000005f;
  const float lp = 0.1f;


  CapSense(uint8_t sendpin, uint8_t recvpin)
  : cs(sendpin, recvpin)
  , x(), y(), w(0.1f), b()
  {
    cs.set_CS_AutocaL_Millis(0xFFFFFFFF);
    cs.set_CS_Timeout_Millis(2000);
    x = _read();
    b = -w*x;
  }

  float step(void) {
    x = lp*_read() + (1-lp)*x;
    y = tanh(w*x + b); /* single tanh neuron */

    /* low-pass out the bias/DC component */
    b += (b > -w*x) ? -eta*y : -10*eta*y;
    
    /* decrease weight w when amp is near saturation,
       or increase w when amp is too lower */
    if (fabs(y) > .6f) w *= 1-eta_w;
    if (fabs(y) < .4f) w *= 1+eta_w;

    return y;
  }

  float get(void) const { return y; }

  uint8_t get_weight() const { return 1/w; }
  void set_weight(uint8_t val) { w = 1.0/val; }

private:
  float _read(void) { return cs.capacitiveSensorRaw(NREAD)/NREAD; }

};


} /* namespace jcl */

#endif /* JCL_CAPSENSE_H */
