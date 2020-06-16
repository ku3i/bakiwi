#ifndef JCL_CAPSENSE_H
#define JCL_CAPSENSE_H

/*---------------------------------+
 | CapSense (Wrapper)              |
 | Jetpack Cognition Lab           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | June 7th 2020                   |
 +---------------------------------*/

#include <CapacitiveSensor.h>

namespace jcl {

class CapSense {
  CapacitiveSensor cs;

public:

  float x,y,b,w;
  static const uint8_t NREAD = 8; /* each reading approx. takes 0.3 ms */
  static const float eta = 0.005f;
  static const float eta_w = .0000005f;
  static const float lp = 0.1f;
  static const float tar = .34f;

  CapSense(uint8_t sendpin, uint8_t recvpin)
  : cs(sendpin, recvpin)
  , x(), y(), w(0.05f), b()
  {
    cs.set_CS_AutocaL_Millis(0xFFFFFFFF);
    cs.set_CS_Timeout_Millis(2000);
    x = _read();
    b = -w*x;
  }

  /* positiv return value signals touch,
     while a negative one signals release.
     return value is always bounded by [-1,+1] */
  float step(void) {
    x = lp*_read() + (1-lp)*x;
    y = tanh(w*x + b); /* single tanh neuron */

    /* low-pass out the bias/DC component */
    b += (b > -w*x) ? -eta*y : -10*eta*y;

    /* decrease weight w when amp is near saturation,
       or increase w when amp is too low */
    if (fabs(y) > tar) w *= 1-eta_w;
    if (fabs(y) < tar) w *= 1+eta_w;

    return y;
  }

  float get(void) const { return y; }

  /* getting and setting the weight parameter for saving and loading.
     param w is transmitted as 1/w to fit in one byte [val=0..255] */
  uint8_t get_weight() const { return clamp(1.f/w, 1.f, 255.f); }

  void set_weight(uint8_t val) {
    if (0!=val) {
      w = 1.f/val;
      b = -w*x;    // update b according to weight change
    }
  }

private:
  float _read(void) { return jcl::max(0L, cs.capacitiveSensorRaw(NREAD)/NREAD); }

};


} /* namespace jcl */

#endif /* JCL_CAPSENSE_H */
