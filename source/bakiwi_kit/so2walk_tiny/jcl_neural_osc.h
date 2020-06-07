#ifndef JCL_NEURAL_OSC_H
#define JCL_NEURAL_OSC_H

/*---------------------------------+
 | Neural SO(2)-Oscillator         |
 | Jetpack Cognition Lab           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | Oct. 6th 2019                   |
 +---------------------------------*/

/* Neural-SO(2)-Oscillator.
 * The theoretical background can be found in this paper:
 * 'SO(2)-Networks as Neural Oscillators' by Frank Pasemann, Manfred Hild and Keyan Zahedi.
 *
 * Brief Description:
 * A fully connected recurrent Two-Neuron-Network is configured as an oscillating system.
 * The weight matrix can roughly be considered a rotation matrix while the state vector is
 * rotated by a defined angle per step. Frequency and amplitude of the oscillation is
 * determined by self (s) and ring (r) coupling weights.
 *
 * update rule:
 * (x1,x2)^T  <-- tanh[ (1+eps) * r(dp) * (x1,x2)^T + (b1,b2)^T ]
 *
 * r(dp) = [ cos(dp), sin(dp)
 *          -sin(dp), cos(dp) ]
 */

/* Known issue: Amplitude gets also a little lower when setting up a low frequency. */

#include <math.h>
#include "jcl_modules.h"

namespace jcl {

class NeuralOscillator {
public:

    /*
       weights  [w11, w12
       matrix:   w21, w22]

       bias     [b1    state   [x1
       vector:   b2]   vector:  x2]
    */

    const float velmin;
    const float velmax;

    float w11, w12, w21, w22, b1, b2, x1, x2;

    NeuralOscillator(float velmin = M_PI / 64, float velmax = M_PI / 8)
    : velmin(velmin)
    , velmax(velmax)
    , w11(), w12(), w21(), w22()
    , b1(), b2(), x1(), x2()
    {
        set_weights(1.0);
    }


    /* call this function to create a new weight matrix
       according to some new desired frequency.
     */
    void set_weights(float frq)
    {
        /* stepsize of oscillator (frequency) */
        const float dp = frq * velmax + (1 - frq) * velmin;

        /* add non-linearity */
        const float k = 1.0f + 1.5f*dp; // useful range: 1 + 1dp ... 1 + 2dp

        /* determine self and ring coupling */
        const float s = cos(dp) * k;
        const float r = sin(dp) * k;

        /* rotation matrix */
        w11 =  s; w12 = r;
        w21 = -r; w22 = s;
    }

    /* start and restart */
    void restart(void) {
        x1 = .00001f; /* a little grain is needed to gently start the oscillation */
        x2 = .0f;
    }

    /* stop the oscillation immediately */
    void stop(void) {
        x1 = .0f;
        x2 = .0f;
    }

    void set_frequency(float val) { set_weights(clamp(val, 0.f, 1.f)); }

    void step(void) {
        const float a1 = w11*x1 + w12*x2 + b1;
        const float a2 = w21*x1 + w22*x2 + b2;
        x1 = tanh(a1);
        x2 = tanh(a2);
    }

    float get_phase_shifted_x1(float phase) const { return x1 * cos(M_PI * phase) + x2 * sin(M_PI * phase); }

};

} /* namespace jcl */

#endif /* JCL_NEURAL_OSC_H */
