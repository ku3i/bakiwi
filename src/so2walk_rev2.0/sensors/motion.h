#ifndef BAKIWI_SENSORS_MOTION_H
#define BAKIWI_SENSORS_MOTION_H

#include <Arduino.h>
#include <Wire.h>

/* temporary MPU6050 class,
   TODO: consider using TinyMPU6050.h
 */

class Motionsensor {
    TwoWire *wire;
public:


    /* I2C address of the MPU-6050.  *
     * If AD0 pin is set to HIGH,    *
     * the I2C address will be 0x69. */
    const int MPU6050_addr=0x68;
    int16_t ax = 0,
            ay = 0,
            az = 0,
            tm = 0,
            gx = 0,
            gy = 0,
            gz = 0;

    Motionsensor(TwoWire &w) : wire(&w) {};

    void init(void) {
       wire->beginTransmission(MPU6050_addr);
       wire->write(0x6B); /* PWR_MGMT_1 register  */
       wire->write(0);    /* wake up the MPU-6050 */
       wire->endTransmission(true);
    }

    void step(void) {
        wire->beginTransmission(MPU6050_addr);
        wire->write(0x3B);                       // starting register
        wire->endTransmission(false);
        wire->requestFrom(MPU6050_addr,14,true); // read 7 x 2 = 14 bytes

        ax = wire->read() << 8 | wire->read();
        ay = wire->read() << 8 | wire->read();
        az = wire->read() << 8 | wire->read();

        tm = wire->read() << 8 | wire->read();
        tm = tm/340.00+36.53;

        gx = wire->read() << 8 | wire->read();
        gy = wire->read() << 8 | wire->read();
        gz = wire->read() << 8 | wire->read();

        //TODO what is the expected time for reading all bytes?
    }
};

#endif /* BAKIWI_SENSORS_MOTION_H */

