/******************************************************************************
*
* 2022 HF SDR Transceiver by AD5GH
* (http://www.ad5gh.com)
*
* Copyright 2022 Rod Gatehouse AD5GH
* Distributed under the terms of the MIT License:
* http://www.opensource.org/licenses/mit-license
*
* VERSION 1.3
* August 13, 2023
*
******************************************************************************/

#include <arduino.h>
#include <EEProm.h>
#include <DetectorExciter.h>
#include <Wire.h>

//#define DEBUG

#define Si5351A         0x60 
#define METRO_MINI      0x62

// Define Si5351A register addresses
#define SI5351_DEVICE_STATUS     0
#define CLK_ENABLE_CONTROL       3
#define CLK0_CONTROL            16 
#define CLK1_CONTROL            17
#define CLK2_CONTROL            18
#define SYNTH_PLL_A             26
#define SYNTH_PLL_B             34
#define SYNTH_MS_0              42
#define SYNTH_MS_1              50
#define SYNTH_MS_2              58
#define PLL_RESET              177
#define XTAL_LOAD_CAP          183

#define FREQ_ERROR_ARRAY_SIZE   10

EEProm      _EE_Prom;

// Receiver LO = CLK0 on Si5351, 2mA Drive, SYNTH_MS_0
// Transmit LO = CLK1 on Si5351, 2mA Drive, SYNTH_MS_1
// 2.5-MHz Ref = CLK2 on Si5351, 2mA Drive, SYNTH_MS_2

enum
{
    rxLO,
    txLO,
    freqRef
};

enum
{
    disable,
    enable
};

#define CLK_MULT                4                           // multiplier for quadrature clock generation

long CWFreqShift                = 650;

const unsigned long RCV_IF      = 48000;                    // 1st IF frequency, adjust for different Roofing Filter center frequency
const unsigned long REF         = 2500000;                  // GPS Stabilization reference frequency

unsigned long VFO;                                          // Variable Frequency Oscillator Frequency

const unsigned long NomXtalFreq = 25000000;
unsigned long XtalFreq          = 25000000;

typedef union
{
  int16_t integer;                                          // this union allows integers to be sent as binary representations over the I2C
  uint8_t binary[sizeof(integer)];
} binaryInt;

binaryInt freqError;

int16_t freqErrorArray[FREQ_ERROR_ARRAY_SIZE];              // array used to store GPS frequency error readings for averaging
uint8_t freqErrorArrayIndex = 0;


DetectorExciter::DetectorExciter(void)
{

}


void DetectorExciter::begin(void)
{
    #ifdef DEBUG
    Serial.println("DetectorExciter debug enabled...");
    #endif

    Wire.beginTransmission(Si5351A);                        // Check for a device on the bus, bail out if it is not there
    uint8_t reg_val;
    reg_val = Wire.endTransmission();

    if(reg_val == 0)                                        // Wait for SYS_INIT flag to be clear, indicating that device is ready
    {
        uint8_t status_reg = 0;
        do
        {
            status_reg = Si5351_read(SI5351_DEVICE_STATUS);
        }   while (status_reg >> 7 == 1);
    
    #ifdef DEBUG
    Serial.println("Si5351 detected on I2C bus");
    #endif
    
    si5351aStart();
    //si5351aSetFreq(SYNTH_MS_2, REF);                        // set CLK2 to 2.5-MHz reference frequency prior to starting GPS stabilization

    int16_t temp = _EE_Prom.getSi5351Error();
    for(uint8_t n = 0; n < FREQ_ERROR_ARRAY_SIZE; n++)      // initialize frequency error array with last value stored in EEPROM
        {
            freqErrorArray[n] = temp;
        }

    }
}


void DetectorExciter::vfoUpdate(uint32_t *frequency, uint8_t *mode)
{
    unsigned long VFO_A_Adjusted;

    if (*mode == 0) VFO_A_Adjusted = *frequency;                    
    else if (*mode == 1) VFO_A_Adjusted = *frequency;
    else if (*mode == 2) VFO_A_Adjusted = *frequency + CWFreqShift;

    setRcvLO((VFO_A_Adjusted + RCV_IF) * CLK_MULT); 
    setXmtLO(VFO_A_Adjusted * CLK_MULT);    
}


void DetectorExciter::setRcvLO(unsigned long freq)
{                                                      
    si5351aSetFreq(SYNTH_MS_0, freq);
}


void DetectorExciter::setXmtLO(unsigned long freq)        
{                                                     
    si5351aSetFreq(SYNTH_MS_1, freq);
}


void DetectorExciter::enableRcvLO(void)
{
    output_enable(rxLO, enable);
}


void DetectorExciter::disableRcvLO(void)
{
    output_enable(rxLO, disable);
}


void DetectorExciter::enableXmtLO(void)
{
    output_enable(txLO, enable);
}


void DetectorExciter::disableXmtLO(void)
{
    output_enable(txLO, disable);
}


void DetectorExciter::readGPSFreqCorrection(void)
{
    uint8_t n = 0;
    int16_t avgFreqError = 0;

    Wire.requestFrom(METRO_MINI, sizeof(freqError.binary));

    while(Wire.available() != 0)
    {
        freqError.binary[n] = Wire.read();
        n++;
    }

    #ifdef DEBUG
        Serial.print("Mixer Board Error: ");
        Serial.println(freqError.integer, DEC);
    #endif

    if(freqError.integer == 0) freqError.integer = _EE_Prom.getSi5351Error();
    
    else                                                                                // smooth error readings to avoid large steps in frequency
    {
        freqErrorArray[freqErrorArrayIndex] = freqError.integer;                        // add new value to error array
        if(++freqErrorArrayIndex == FREQ_ERROR_ARRAY_SIZE) freqErrorArrayIndex = 0;     // increment array index

        for(uint8_t i=0; i < FREQ_ERROR_ARRAY_SIZE; i++)                                // sum error values in the array
        {
            avgFreqError += freqErrorArray[n];
        }

        avgFreqError = avgFreqError / FREQ_ERROR_ARRAY_SIZE;                            // average error array values

        XtalFreq = NomXtalFreq + avgFreqError;
        _EE_Prom.storeSi5351Error(avgFreqError);

        avgFreqError = 0;                                                               // reset average error to zero
    }
}


//*****************************************************************************
//  Si5351 Multisynch processing
//  by Gene Marcus W3PM/GM4YRE, QEX July/August 2015, with minor changes
//*****************************************************************************
void DetectorExciter::si5351aSetFreq(int synth, unsigned long  freq)
{
  unsigned long long CalcTemp;
  unsigned long  a, b, c, p1, p2, p3;

  c = 0xFFFFF;                                                                          // Denominator derived from max bits 2^20

  a = (XtalFreq * 36) / freq;                                                           // 36 is derived from 900/25 MHz
  CalcTemp = (XtalFreq * 36) % freq;
  CalcTemp *= c;
  CalcTemp /= freq ; 
  b = CalcTemp;                                                                         // Calculated numerator


  // Refer to Si5351 Register Map AN619 for following formula
  p3  = c;
  p2  = (128 * b) % c;
  p1  = 128 * a;
  p1 += (128 * b / c);
  p1 -= 512;

  // Write data to multisynth registers
  Si5351_write(synth, 0xFF);  
  Si5351_write(synth + 1, 0xFF);
  Si5351_write(synth + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(synth + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(synth + 4, (p1 & 0x000000FF));
  Si5351_write(synth + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(synth + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(synth + 7, (p2 & 0x000000FF));
}


//*****************************************************************************
//  Si5351 initialization routines
//  by Gene Marcus W3PM/GM4YRE, QEX July/August 2015, with minor changes
//*****************************************************************************
void DetectorExciter::si5351aStart(void)
{
  // Initialize Si5351A
  Si5351_write(XTAL_LOAD_CAP,0b11000000);                                               // Set crystal load to 10pF
  Si5351_write(CLK_ENABLE_CONTROL,0b00000100);                                          // Enable CLK0 & CLK1 outputs
  Si5351_write(CLK0_CONTROL,0b00001100);                                                // Set PLLA to CLK0, 2 mA output
  Si5351_write(CLK1_CONTROL,0b00101100);                                                // Set PLLB to CLK1, 2 mA output
  Si5351_write(CLK2_CONTROL,0b00101100);                                                // Set PLLB to CLK2, 2 mA output
  Si5351_write(PLL_RESET,0b10100000);                                                   // Reset PLLA and PLLB


  // Set PLLA and PLLB to 900 MHz
  unsigned long  a, b, c, p1, p2, p3;

  a = 36;                                                                               // Derived from 900/25 MHz
  b = 0;                                                                                // Numerator
  c = 0xFFFFF;                                                                          // Denominator derived from max bits 2^20

  // Refer to Si5351 Register Map AN619 for following formula
  p3  = c;
  p2  = (128 * b) % c;
  p1  = 128 * a;
  p1 += (128 * b / c);
  p1 -= 512;

  // Write data to PLL registers
  Si5351_write(SYNTH_PLL_A, 0xFF);
  Si5351_write(SYNTH_PLL_A + 1, 0xFF);
  Si5351_write(SYNTH_PLL_A + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(SYNTH_PLL_A + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(SYNTH_PLL_A + 4, (p1 & 0x000000FF));
  Si5351_write(SYNTH_PLL_A + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(SYNTH_PLL_A + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(SYNTH_PLL_A + 7, (p2 & 0x000000FF));

  Si5351_write(SYNTH_PLL_B, 0xFF);
  Si5351_write(SYNTH_PLL_B + 1, 0xFF);
  Si5351_write(SYNTH_PLL_B + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(SYNTH_PLL_B + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(SYNTH_PLL_B + 4, (p1 & 0x000000FF));
  Si5351_write(SYNTH_PLL_B + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(SYNTH_PLL_B + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(SYNTH_PLL_B + 7, (p2 & 0x000000FF));
}


//*****************************************************************************
//Si5351 Output Enable / Disable
//*****************************************************************************

void DetectorExciter::output_enable(uint8_t clk, uint8_t enable)              // enable - set to 1 to enable, 0 to disable
{
  uint8_t reg_val;

  reg_val = Si5351_read(CLK_ENABLE_CONTROL);

  if(enable == 1)
  {
    reg_val &= ~(1<<(uint8_t)clk);
  }
  else
  {
    reg_val |= (1<<(uint8_t)clk);
  }

  Si5351_write(CLK_ENABLE_CONTROL, reg_val);

}

//*****************************************************************************
//Write I2C data routine
//*****************************************************************************
uint8_t DetectorExciter::Si5351_write(uint8_t addr, uint8_t data)
{   
  uint8_t reg_val;
    
  Wire.beginTransmission(Si5351A);
  Wire.write(addr);
  Wire.write(data);
  reg_val = Wire.endTransmission();

  #ifdef DEBUG
    Serial.print("Wire.endTransmission response: ");
    Serial.println(reg_val);
  #endif

  return(reg_val);                                                                      // critical to return a value here; if you define a function with a return value, have to return a value!
}

//*****************************************************************************
//Read I2C data routine
//*****************************************************************************
uint8_t DetectorExciter::Si5351_read(uint8_t addr)
{
    uint8_t reg_val = 0;

    Wire.beginTransmission(Si5351A);
    Wire.write(addr);
    Wire.endTransmission();

    Wire.requestFrom(Si5351A, (uint8_t)1, (uint8_t)false);

    while(Wire.available())
    {
        reg_val = Wire.read();
    }

    return reg_val;
}
