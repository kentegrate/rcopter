/*
PCA9685 driver code is placed under the BSD license.
Written by Mikhail Avkhimenia (mikhail.avkhimenia@emlid.com)
Copyright (c) 2014, Emlid Limited, www.emlid.com
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
      * Neither the name of the Emlid Limited nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL EMLID LIMITED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*From Navio Resository adapted for bcm_2835 library by Ken Takaki
https://github.com/emlid/Navio/blob/master/C%2B%2B/Navio/PCA9685.cpp
 */

#include <rcopter_drivers/pca9685.hpp>
#include <bcm2835.h>
#include <string.h>

/** PCA9685 constructor.
 * @param address I2C address
 * @see PCA9685_DEFAULT_ADDRESS
 */
#define PWM_ENABLE RPI_V2_GPIO_P1_13

PCA9685::~PCA9685(){
}
uint8_t PCA9685::writeByte(uint8_t regAddr, char *data){
  char send_data[2];
  send_data[0] = regAddr;
  send_data[1] = *data;
  //  uint8_t address = 0b11100000;
  //  bcm2835_i2c_setSlaveAddress(address);
  return bcm2835_i2c_write(&send_data[0],2);
}
uint8_t PCA9685::writeBytes(uint8_t regAddr, char *data, uint32_t bytes){
  char send_data[1+bytes];
  send_data[0]=regAddr;
  memcpy(send_data+1,data,bytes);
  //  uint8_t address = 0b11100000;
  //  bcm2835_i2c_setSlaveAddress(address);
  
  return bcm2835_i2c_write(send_data,bytes+1);
}
uint8_t PCA9685::readByte(uint8_t regAddr, char *data){
  char read_data = 0x00;
  ///  writeByte(regAddr,&read_data);
  char reg = regAddr;

  bcm2835_i2c_write(&reg,1);
  return bcm2835_i2c_read(data,1);
   
}
uint8_t PCA9685::writeBit(uint8_t regAddr, uint8_t bitNum, char data){
  char b;
  readByte(regAddr,&b);
  b = (data!=0) ? (b | (1<<bitNum)) : (b & ~(1 <<bitNum));
  return writeByte(regAddr, &b);
}

/** Power on and prepare for general usage.
 * This method reads prescale value stored in PCA9685 and calculate frequency based on it.
 * Then it enables auto-increment of register address to allow for faster writes.
 * And finally the restart is performed to enable clocking.
 */
void PCA9685::initialize() {

  if (!bcm2835_init())
    {
      printf("bcm2835_init failed. Are you running as root??\n");
    }
      
  // I2C begin if specified    

  if (!bcm2835_i2c_begin())
    {
      printf("bcm2835_i2c_begin failed. Are you running as root??\n");
    }

  bcm2835_i2c_setSlaveAddress(PCA9685_DEFAULT_ADDRESS);
  uint16_t clk_div = BCM2835_I2C_CLOCK_DIVIDER_626;
  bcm2835_i2c_setClockDivider(clk_div);

  bcm2835_gpio_fsel(PWM_ENABLE, BCM2835_GPIO_FSEL_OUTP);

  this->frequency = getFrequency();
  writeBit(PCA9685_RA_MODE1, PCA9685_MODE1_AI_BIT, 1);
  restart();
  enableOutput();
}

void PCA9685::finalize(){
  disableOutput();
  bcm2835_i2c_end();
  bcm2835_close();
}

/** Verify the I2C connection.
 * @return True if connection is valid, false otherwise
 */
bool PCA9685::testConnection() {
  char data;
  int8_t status = readByte(PCA9685_RA_PRE_SCALE, &data);
  if (status == BCM2835_I2C_REASON_OK)
    return true;
  else
    return false;
}

/** Enable PWM output on PXF Mini
 **/
void PCA9685::enableOutput(){
  bcm2835_gpio_write(PWM_ENABLE,LOW);
}

/** Disable PWM output on PXF Mini
 **/
void PCA9685::disableOutput(){
  bcm2835_gpio_write(PWM_ENABLE,HIGH);  
}

/** Put PCA9685 to sleep mode thus turning off the outputs.
 * @see PCA9685_MODE1_SLEEP_BIT
 */
void PCA9685::sleep() {
  writeBit(PCA9685_RA_MODE1, PCA9685_MODE1_SLEEP_BIT, 1);
}

/** Disable sleep mode and start the outputs.
 * @see PCA9685_MODE1_SLEEP_BIT
 * @see PCA9685_MODE1_RESTART_BIT
 */
void PCA9685::restart() {
  char data_1 = (1 << PCA9685_MODE1_SLEEP_BIT);
  writeByte(PCA9685_RA_MODE1, &data_1);
  char data_2 = ((1 << PCA9685_MODE1_SLEEP_BIT) | (1 << PCA9685_MODE1_EXTCLK_BIT));
  writeByte(PCA9685_RA_MODE1, &data_2);
  char data_3 = ((1 << PCA9685_MODE1_RESTART_BIT)| (1 << PCA9685_MODE1_EXTCLK_BIT)| (1 << PCA9685_MODE1_AI_BIT));
  writeByte(PCA9685_RA_MODE1, &data_3);
}

/** Calculate prescale value based on the specified frequency and write it to the device.
 * @return Frequency in Hz
 * @see PCA9685_RA_PRE_SCALE
 */
float PCA9685::getFrequency() {
  char data;
  readByte(PCA9685_RA_PRE_SCALE, &data);
  return 24576000.f / 4096.f / (data + 1);
}


/** Calculate prescale value based on the specified frequency and write it to the device.
 * @param Frequency in Hz
 * @see PCA9685_RA_PRE_SCALE
 */
void PCA9685::setFrequency(float frequency) {
  sleep();
  usleep(10000);
  char prescale = roundf(24576000.f / 4096.f / frequency)  - 1;
  writeByte(PCA9685_RA_PRE_SCALE, &prescale);
  this->frequency = getFrequency();
  restart();
}

/** Set channel start offset of the pulse and it's length
 * @param Channel number (0-15)
 * @param Offset (0-4095)
 * @param Length (0-4095)
 * @see PCA9685_RA_LED0_ON_L
 */
void PCA9685::setPWM(uint8_t channel, uint16_t offset, uint16_t length) {
  char data[4] = {0, 0, 0, 0};
  if(length == 0) {
    data[3] = 0x10;
  } else if(length >= 4096) {
    data[1] = 0x10;
  } else {
    data[0] = offset & 0xFF;
    data[1] = offset >> 8;
    data[2] = length & 0xFF;
    data[3] = length >> 8;
  }
  writeBytes(PCA9685_RA_LED0_ON_L + 4 * channel, data,4);
}

/** Set channel's pulse length
 * @param Channel number (0-15)
 * @param Length (0-4095)
 * @see PCA9685_RA_LED0_ON_L
 */
void PCA9685::setPWM(uint8_t channel, uint16_t length) {
  setPWM(channel, 0, length);
}

/** Set channel's pulse length in milliseconds
 * @param Channel number (0-15)
 * @param Length in milliseconds
 * @see PCA9685_RA_LED0_ON_L
 */
void PCA9685::setPWMmS(uint8_t channel, float length_mS) {
  setPWM(channel, round((length_mS * 4096.f) / (1000.f / frequency)));
}

/** Set channel's pulse length in microseconds
 * @param Channel number (0-15)
 * @param Length in microseconds
 * @see PCA9685_RA_LED0_ON_L
 */
void PCA9685::setPWMuS(uint8_t channel, float length_uS) {
  setPWM(channel, round((length_uS * 4096.f) / (1000000.f / frequency)));
}

/** Set start offset of the pulse and it's length for all channels
 * @param Offset (0-4095)
 * @param Length (0-4095)
 * @see PCA9685_RA_ALL_LED_ON_L
 */
void PCA9685::setAllPWM(uint16_t offset, uint16_t length) {
  char data[4] = {offset & 0xFF, offset >> 8, length & 0xFF, length >> 8};
  writeBytes(PCA9685_RA_ALL_LED_ON_L,data,4);
}

/** Set pulse length for all channels
 * @param Length (0-4095)
 * @see PCA9685_RA_ALL_LED_ON_L
 */
void PCA9685::setAllPWM(uint16_t length) {
  setAllPWM(0, length);
}

/** Set pulse length in milliseconds for all channels
 * @param Length in milliseconds
 * @see PCA9685_RA_ALL_LED_ON_L
 */
void PCA9685::setAllPWMmS(float length_mS) {
  setAllPWM(round((length_mS * 4096.f) / (1000.f / frequency)));
}

/** Set pulse length in microseconds for all channels
 * @param Length in microseconds
 * @see PCA9685_RA_ALL_LED_ON_L
 */
void PCA9685::setAllPWMuS(float length_uS) {
  setAllPWM(round((length_uS * 4096.f) / (1000000.f / frequency)));
}
