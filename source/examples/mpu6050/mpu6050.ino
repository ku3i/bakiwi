#include<Wire.h>

/* Example code for MPU 6050 on
 * GY-521 development board
 */


/* I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.*/
const int MPU6050_addr=0x68; 

int16_t ax,ay,az,tm,gx,gy,gz;

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU6050_addr);
  Wire.write(0x6B); /* PWR_MGMT_1 register  */
  Wire.write(0);    /* wake up the MPU-6050 */ 
  Wire.endTransmission(true);
  Serial.begin(1000000);
}

void loop() {
  Wire.beginTransmission(MPU6050_addr);
  Wire.write(0x3B);                       /* starting register */
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_addr,14,true); /* read 7 x 2 = 14 bytes*/
  
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  
  tm = Wire.read() << 8 | Wire.read();
  tm = tm/340.00+36.53;
  
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
  
  Serial.print(ax); Serial.print(" ");
  Serial.print(ay); Serial.print(" ");
  Serial.print(az); Serial.print(" ");
  Serial.print(tm); Serial.print(" ");
  Serial.print(gx); Serial.print(" ");
  Serial.print(gy); Serial.print(" ");
  Serial.print(gz); Serial.print(" ");
  Serial.println();
  delay(20);
}
