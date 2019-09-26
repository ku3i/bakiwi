/**
 * Reads supply voltage of Arduino Nano without any additional ADC pin.
 * This example works for Atmega 328p!
 * Other architectures might have a different set of ADC mux bits to select.
 * 
 * Source: https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
 * by Scott Daniels
 * 
 * 
 * From Atmega 328p datasheet:
 * 
 * REFS1 | REFS0 | Voltage Reference Selection
 *     0 |     0 | AREF, Internal V ref turned off
 *     0 |     1 | AV CC with external capacitor at AREF pin
 *     1 |     0 | Reserved
 *     1 |     1 | Internal 1.1V Voltage Reference with external capacitor at AREF pin
 * 
 * 
 * MUX3...0 Single Ended Input
 * 0000 ADC0
 * 0001 ADC1
 * 0010 ADC2
 * 0011 ADC3
 * 0100 ADC4
 * 0101 ADC5
 * 0110 ADC6
 * 0111 ADC7
 * 1000 ADC8 (1)
 * 1001 (reserved)
 * 1010 (reserved)
 * 1011 (reserved)
 * 1100 (reserved)
 * 1101 (reserved)
 * 1110 1.1V (V BG )
 * 1111 0V (GND)
 * 
 */



long read_supply_voltage_mV()
{  
  /* Read 1V1 reference against AVcc
   * set the reference to Vcc and the measurement to the internal 1.1V reference */
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  //delay(2); /* Wait for Vref to settle (optionally) */
  
  ADCSRA |= _BV(ADSC);             /* start conversion */
  while (bit_is_set(ADCSRA,ADSC)); /* measuring */
  uint8_t lo = ADCL;               /* read ADCL first, it locks ADCH */
  uint8_t hi = ADCH;               /* unlocks both */

  long result = (hi<<8) | lo;      /* combine */
  result = 1125300L / result;      /* calculate AVcc in mV 
                                    * 1125300 = 1.1*1023*1000 */
  return result; 
}

void setup() {
  Serial.begin(9600);
  delay(1000);
}

void loop() { 
  Serial.println(read_supply_voltage_mV(), DEC);
  delay(100);
}
