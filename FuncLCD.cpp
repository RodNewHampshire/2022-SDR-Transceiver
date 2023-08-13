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

#include "Adafruit_LiquidCrystal.h"
#include <arduino.h>
#include <FuncLCD.h>
#include <MeterLCD.h>
#include <Wire.h>

Adafruit_LiquidCrystal  _funcLCD(1);


char Function_Row0[21] = {" AF:      Band:     "};
char Function_Row1[21] = {" RF:      Mode:     "};
char Function_Row2[21] = {" IF:      Rate:     "};
char Function_Row3[21] = {" LN       Func:     "};

const char BAND_1[12] = {"1       111"};
const char BAND_2[12] = {"68432111000"};
const char BAND_3[12] = {"00000752abc"};

const char MODE_1[4] = {"ul "};
const char MODE_2[4] = {"ssc"};
const char MODE_3[4] = {"bbw"};

const char RATE_1[5] = {"   1"};
const char RATE_2[5] = {" 120"};
const char RATE_3[5] = {"1050"};
const char RATE_4[5] = {"0000"};

const char ATTN_1[17] = {"0000011111222223"};
const char ATTN_2[17] = {"0246802468024680"};

const char TR_1[3] = {" *"};
const char TR_2[3] = {"RX"};
const char TR_3[3] = {"CM"};
const char TR_4[3] = {"VT"};
const char TR_5[3] = {" *"};

const char FUNC_1[10] = {"BRMGI"};
const char FUNC_2[10] = {"aaoPQ"};
const char FUNC_3[10] = {"ntdS_"};
const char FUNC_4[10] = {"dee C"};


char GAIN_DISP[3];


FuncLCD::FuncLCD(void)
{

}


void FuncLCD::begin(void)
{
  setI2CForLCD();
  _funcLCD.begin(20, 4);
}


void FuncLCD::clear(void)
{
  _funcLCD.clear();
}


void FuncLCD::splash(void)
{
  _funcLCD.setCursor(0, 0);
 	_funcLCD.print(F("    FUNCTION LCD    "));
  _funcLCD.setCursor(0, 1);
  _funcLCD.print(F("  HIGH PERFORMANCE  "));
  _funcLCD.setCursor(0, 2);
  _funcLCD.print(F("   HF TRANSCEIVER   "));
  _funcLCD.setCursor(0, 3);
  _funcLCD.print(F("      BY AD5GH      "));
}


void FuncLCD::init(uint8_t _RATE, uint8_t _MODE, uint8_t _BAND)
{
  _funcLCD.setCursor(0, 0);
  _funcLCD.print(Function_Row0);

  _funcLCD.setCursor(0, 1);
  _funcLCD.print(Function_Row1);

  _funcLCD.setCursor(0, 2);
  _funcLCD.print(Function_Row2);

  _funcLCD.setCursor(0, 3);
  _funcLCD.print(Function_Row3);

  update(RATE_Cmnd + _RATE);
  update(MODE_Cmnd + _MODE);
  update(BAND_Cmnd + _BAND);
  update(FUNC_Cmnd + 0);
}


void FuncLCD::update(uint8_t command)                                             // update LCD functions display
{
  switch (command >> 4)
  {
    case 1:                                                                       
      
      break;

    case 2:                                                                       

      break;

    case 3:                                                                       // RF ATTENUATOR
      command = command & 0x0F;
      if(command == 0) Function_Row1[5] = 0x20;
      else Function_Row1[5] = 0x2D;
      Function_Row1[6] = ATTN_1[command];
      Function_Row1[7] = ATTN_2[command];
      _funcLCD.setCursor(0, 1);
      _funcLCD.print(Function_Row1);
      break;

    case 4:                                                                       // BAND
      command = command & 0x0F;
      Function_Row0[17] = BAND_1[command];
      Function_Row0[18] = BAND_2[command];
      Function_Row0[19] = BAND_3[command];
      _funcLCD.setCursor(0, 0);
      _funcLCD.print(Function_Row0);
      break;

    case 5:                                                                       // MODE
      command = command & 0x0F;
      Function_Row1[17] = MODE_1[command];
      Function_Row1[18] = MODE_2[command];
      Function_Row1[19] = MODE_3[command];
      _funcLCD.setCursor(0, 1);
      _funcLCD.print(Function_Row1);
      break;

    case 6:                                                                      // RATE
      command = command & 0x0F;
      Function_Row2[16] = RATE_1[command];
      Function_Row2[17] = RATE_2[command];
      Function_Row2[18] = RATE_3[command];
      Function_Row2[19] = RATE_4[command];
      _funcLCD.setCursor(0, 2);
      _funcLCD.print(Function_Row2);
      break;

    case 7:                                                                      // TR
      command = command & 0x0F;
      break;

    case 8:                                                                      // FUNCTION
      command = command & 0x0F;
      Function_Row3[16] = FUNC_1[command];
      Function_Row3[17] = FUNC_2[command];
      Function_Row3[18] = FUNC_3[command];
      Function_Row3[19] = FUNC_4[command];
      _funcLCD.setCursor(0, 3);
      _funcLCD.print(Function_Row3);
      break;

    case 10:                                                                   // GPS ERROR DISPLAY
      break;
  }
}


void FuncLCD::selectAF_Gain(void)
{
  Function_Row3[0] = ' ';
  _funcLCD.setCursor(0, 3);
  _funcLCD.print(Function_Row3);
  Function_Row0[0] = '*';
  _funcLCD.setCursor(0, 0);
  _funcLCD.print(Function_Row0);
}


void FuncLCD::selectRF_Gain(void)
{
  Function_Row0[0] = ' ';
  _funcLCD.setCursor(0, 0);
  _funcLCD.print(Function_Row0);
  Function_Row1[0] = '*';
  _funcLCD.setCursor(0, 1);
  _funcLCD.print(Function_Row1);
}


void FuncLCD::selectIF_Gain(void)
{
  Function_Row1[0] = ' ';
  _funcLCD.setCursor(0, 1);
  _funcLCD.print(Function_Row1);
  Function_Row2[0] = '*';
  _funcLCD.setCursor(0, 2);
  _funcLCD.print(Function_Row2);
}


void FuncLCD::selectLN_Gain(void)
{
  Function_Row2[0] = ' ';
  _funcLCD.setCursor(0, 2);
  _funcLCD.print(Function_Row2);
  Function_Row3[0] = '*';
  _funcLCD.setCursor(0, 3);
  _funcLCD.print(Function_Row3);
}


void FuncLCD::dsplyAF_Gain(int8_t gain)
{
  if(gain < 0) 
    {
      GAIN_DISP[0] = 0x2D;
      gain = -gain;
    }
  else if(gain == 0) GAIN_DISP[0] = 0x20;
  else GAIN_DISP[0]= 0x2B;

  if (gain < 10) 
    {
        GAIN_DISP[1] = 0x20;
        GAIN_DISP[2] = gain + 0x30;
    }
    else
    {
        GAIN_DISP[1] = (gain/10 + 0x30);
        GAIN_DISP[2] = ((gain % 10) + 0x30);
    }
    
    Function_Row0[5] = GAIN_DISP[0];
    Function_Row0[6] = GAIN_DISP[1];
    Function_Row0[7] = GAIN_DISP[2];
    _funcLCD.setCursor(0, 0);
    _funcLCD.print(Function_Row0);
}


void FuncLCD::dsplyIF_Gain(int8_t gain)
{
  if(gain < 0) 
    {
      GAIN_DISP[0] = 0x2D;
      gain = -gain;
    }
  else if(gain == 0) GAIN_DISP[0] = 0x20;
  else GAIN_DISP[0]= 0x2B;

  if (gain < 10) 
    {
        GAIN_DISP[1] = 0x20;
        GAIN_DISP[2] = gain + 0x30;
    }
    else
    {
        GAIN_DISP[1] = (gain/10 + 0x30);
        GAIN_DISP[2] = ((gain % 10) + 0x30);
    }
    
    Function_Row2[5] = GAIN_DISP[0];
    Function_Row2[6] = GAIN_DISP[1];
    Function_Row2[7] = GAIN_DISP[2];
    _funcLCD.setCursor(0, 2);
    _funcLCD.print(Function_Row2);
}


void FuncLCD::dsplyLN_Gain(int8_t gain)
{
  if(gain < 0) 
    {
      GAIN_DISP[0] = 0x2D;
      gain = -gain;
    }
  else if(gain == 0) GAIN_DISP[0] = 0x20;
  else GAIN_DISP[0]= 0x2B;

  if (gain < 10) 
    {
        GAIN_DISP[1] = 0x20;
        GAIN_DISP[2] = gain + 0x30;
    }
    else
    {
        GAIN_DISP[1] = (gain/10 + 0x30);
        GAIN_DISP[2] = ((gain % 10) + 0x30);
    }
    
    Function_Row3[5] = GAIN_DISP[0];
    Function_Row3[6] = GAIN_DISP[1];
    Function_Row3[7] = GAIN_DISP[2];
    _funcLCD.setCursor(0, 3);
    _funcLCD.print(Function_Row3);
}


void FuncLCD::blankGain(void)
{
  Function_Row3[0] = 0x20;
  Function_Row3[1] = 0x20;
  Function_Row3[2] = 0x20;
  _funcLCD.setCursor(0, 3);
  _funcLCD.print(Function_Row3);
}


void FuncLCD::setI2CForLCD(void)
{
  Wire.beginTransmission(0x70);
  Wire.write(0x01);
  Wire.endTransmission();
}