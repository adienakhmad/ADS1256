// Arudino Sample Code to use ADS1256 library
// Read 4 sensor using effiecient input cycling
// Written by Adien Akhmad, August 2015

#include <ADS1256.h>
#include <SPI.h>

byte DRDYPIN = 9;
byte CSPIN = 8;
float clockMHZ = 7.68; // crystal frequency used on ADS1256

// Define ADS1256 PIN that is connected to Arduino
ADS1256 adc(clockMHZ,DRDYPIN,CSPIN,2.5); // RESETPIN is permanently tied to 3.3v

/* Variable to store sensor reading
  sensor1 = connected on AIN0 - AIN1
  sensor2 = connected on AIN2 - AIN3
  sensor3 = connected on AIN4 - AIN5
  sensor4 = connected on AIN6 - AIN7
*/

float sensor1, sensor2, sensor3, sensor4;


void setup()
{
  Serial.begin(9600);
  
  Serial.println("Starting ADC");

  // start the ADS1256 with data rate of 15 SPS
  // other data rates: 
  // ADS1256_DRATE_30000SPS
  // ADS1256_DRATE_15000SPS
  // ADS1256_DRATE_7500SPS
  // ADS1256_DRATE_3750SPS
  // ADS1256_DRATE_2000SPS
  // ADS1256_DRATE_1000SPS
  // ADS1256_DRATE_500SPS
  // ADS1256_DRATE_100SPS
  // ADS1256_DRATE_60SPS
  // ADS1256_DRATE_50SPS
  // ADS1256_DRATE_30SPS
  // ADS1256_DRATE_25SPS
  // ADS1256_DRATE_15SPS
  // ADS1256_DRATE_10SPS
  // ADS1256_DRATE_5SPS
  // ADS1256_DRATE_2_5SPS
  // 
  // NOTE : Data Rate vary depending on crystal frequency. Data rates listed below assumes the crystal frequency is 7.68Mhz
  //        for other frequency consult the datasheet.

  adc.start(ADS1256_DRATE_15SPS); 

  Serial.println("ADC Started");
  
  // Set MUX Register to AINO and AIN1
  adc.switchChannel(0,1);
}

void loop()
{ 

  // Efficient Input Cycling
  // to learn further, read on datasheet page 21, figure 19 : Cycling the ADS1256 Input Multiplexer
  
  adc.waitDRDY(); // wait for DRDY to go low before changing multiplexer register
  adc.switchChannel(2,3);
  sensor1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and AIN1

  adc.waitDRDY();
  adc.switchChannel(4,5);
  sensor2 = adc.readCurrentChannel(); //// DOUT arriving here are from MUX AIN2 and AIN3

  adc.waitDRDY();
  adc.switchChannel(6,7);
  sensor3 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN4 and AIN5

  adc.waitDRDY();
  adc.switchChannel(0,1); // switch back to MUX AIN0 and AIN1
  sensor4 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN6 and AIN7

  //print the result.
  Serial.print(sensor1,10);
  Serial.print("\t");
  Serial.print(sensor2,10);
  Serial.print("\t");
  Serial.print(sensor3,10);
  Serial.print("\t");
  Serial.println(sensor4,10);
}

