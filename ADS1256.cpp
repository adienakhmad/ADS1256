/*
        ADS1256.h - Arduino Library for communication with Texas Instrument
   ADS1256 ADC
        Written by Adien Akhmad, August 2015
*/

#include "ADS1256.h"
#include "Arduino.h"
#include "SPI.h"

// Use this if RESET PIN is connected to one of Arduino Digital pin
ADS1256::ADS1256(float clockspdMhz, byte DRDY, byte CS, byte RESETPIN,
                 float VREF) {
  _tscaler = (clockspdMhz)*10;
  pinMode(DRDY, INPUT);
  pinMode(CS, OUTPUT);
  pinMode(RESETPIN, OUTPUT);
  digitalWrite(RESETPIN, HIGH);

  _CS = CS;
  _VREF = VREF;
  _DRDY = DRDY;
  _conversionFactor = 1.00;
  _RESET = RESETPIN;

  SPI.begin();
  SPI.beginTransaction(
      SPISettings(clockspdMhz / 4 * 1000000, MSBFIRST, SPI_MODE1));
}

// Use this if RESET PIN is permanently tied to HIGH

ADS1256::ADS1256(float clockspdMhz, byte DRDY, byte CS, float VREF) {
  _tscaler = (clockspdMhz)*10;
  pinMode(DRDY, INPUT);
  pinMode(CS, OUTPUT);

  _CS = CS;
  _VREF = VREF;
  _DRDY = DRDY;
  _conversionFactor = 1.00;
  _RESET = 255;  // mark that this PIN is not used

  SPI.begin();
  SPI.beginTransaction(
      SPISettings(clockspdMhz / 4 * 1000000, MSBFIRST, SPI_MODE1));
}

void ADS1256::writeRegister(unsigned char reg, unsigned char wdata) {
  CSON();
  SPI.transfer(WREG | reg);
  SPI.transfer(0);
  SPI.transfer(wdata);
  delayMicroseconds(4 * 10 / _tscaler +
                    1);  // t11 delay (4*tCLKIN) after WREG command
  CSOFF();
}

unsigned char ADS1256::readRegister(unsigned char reg) {
  unsigned char readValue;

  CSON();
  SPI.transfer(RREG | reg);
  SPI.transfer(0);
  delayMicroseconds(50 * 10 / _tscaler + 1);  // t6 delay (50*tCLKIN)
  readValue = SPI.transfer(0);
  delayMicroseconds(4 * 10 / _tscaler + 1);  // t11 delay after RREG command
  CSOFF();

  return readValue;
}

void ADS1256::sendCommand(unsigned char reg) {
  CSON();
  waitDRDY();
  SPI.transfer(reg);
  delayMicroseconds(4 * 10 / _tscaler + 1);
  CSOFF();
}

void ADS1256::setConversionFactor(float val) { _conversionFactor = val; }

float ADS1256::readCurrentChannel() {
  CSON();
  SPI.transfer(RDATA);
  delayMicroseconds(50 * 10 / _tscaler + 1);  // t6 delay (50*tCLKIN)
  float adsCode = read_float32();
  CSOFF();
  return ((adsCode / 0x800000) * ((2 * _VREF) / (float)_pga)) * _conversionFactor;
}

// Call this ONLY after RDATA command
unsigned long ADS1256::read_uint24() {
  unsigned char _highByte, _midByte, _lowByte;
  unsigned long value;

  _highByte = SPI.transfer(WAKEUP);
  _midByte = SPI.transfer(WAKEUP);
  _lowByte = SPI.transfer(WAKEUP);

  // Combine all 3-bytes to 24-bit data using byte shifting.
  value = ((long)_highByte << 16) + ((long)_midByte << 8) + ((long)_lowByte);
  return value;
}

// Call this ONLY after RDATA command
long ADS1256::read_int32() {
  long value = read_uint24();

  if (value & 0x00800000) {
    value |= 0xff000000;
  }

  return value;
}

// Call this ONLY after RDATA command
float ADS1256::read_float32() {
  long value = read_int32();
  return (float)value;
}

// Channel switching for single ended mode. Negative input channel are
// automatically set to AINCOM
void ADS1256::switchChannel(byte channel) { switchChannel(channel, -1); }

// Channel Switching for differential mode. Use -1 to set input channel to
// AINCOM
void ADS1256::switchChannel(byte AIN_P, byte AIN_N) {
  unsigned char MUX_CHANNEL;
  unsigned char MUXP;
  unsigned char MUXN;

  switch (AIN_P) {
    case 0:
      MUXP = ADS1256_MUXP_AIN0;
      break;
    case 1:
      MUXP = ADS1256_MUXP_AIN1;
      break;
    case 2:
      MUXP = ADS1256_MUXP_AIN2;
      break;
    case 3:
      MUXP = ADS1256_MUXP_AIN3;
      break;
    case 4:
      MUXP = ADS1256_MUXP_AIN4;
      break;
    case 5:
      MUXP = ADS1256_MUXP_AIN5;
      break;
    case 6:
      MUXP = ADS1256_MUXP_AIN6;
      break;
    case 7:
      MUXP = ADS1256_MUXP_AIN7;
      break;
    default:
      MUXP = ADS1256_MUXP_AINCOM;
  }

  switch (AIN_N) {
    case 0:
      MUXN = ADS1256_MUXN_AIN0;
      break;
    case 1:
      MUXN = ADS1256_MUXN_AIN1;
      break;
    case 2:
      MUXN = ADS1256_MUXN_AIN2;
      break;
    case 3:
      MUXN = ADS1256_MUXN_AIN3;
      break;
    case 4:
      MUXN = ADS1256_MUXN_AIN4;
      break;
    case 5:
      MUXN = ADS1256_MUXN_AIN5;
      break;
    case 6:
      MUXN = ADS1256_MUXN_AIN6;
      break;
    case 7:
      MUXN = ADS1256_MUXN_AIN7;
      break;
    default:
      MUXN = ADS1256_MUXN_AINCOM;
  }

  MUX_CHANNEL = MUXP | MUXN;

  CSON();
  writeRegister(MUX, MUX_CHANNEL);
  sendCommand(SYNC);
  sendCommand(WAKEUP);
  CSOFF();
}

void ADS1256::begin(unsigned char drate, unsigned char gain, bool buffenable) {
  _pga = 1 << gain;
  sendCommand(
      SDATAC);  // send out SDATAC command to stop continous reading mode.
  writeRegister(DRATE, drate);  // write data rate register
  uint8_t bytemask = B00000111;
  uint8_t adcon = readRegister(ADCON);
  uint8_t byte2send = (adcon & ~bytemask) | gain;
  writeRegister(ADCON, byte2send);
  if(buffenable){
    uint8_t status = readRegister(STATUS);
    bitSet(status,1);
    writeRegister(STATUS, status);
  }
  sendCommand(SELFCAL);  // perform self calibration
  while (digitalRead(_DRDY))
    ;  // wait ADS1256 to settle after self calibration
}

void ADS1256::CSON() { digitalWrite(_CS, LOW); }

void ADS1256::CSOFF() { digitalWrite(_CS, HIGH); }

void ADS1256::waitDRDY() {
  while (digitalRead(_DRDY))
    ;
}
