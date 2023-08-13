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

//Band Indexing 0=160m, 1=80m, 2=40m, 3=30m, 4=20m, 5=17m, 6=15m, 7=12m, 8=10am, 9=10bm, 10=10cm

#include <EEProm.h>
#include <FreqControl.h>
#include <FuncDial.h>
#include <FuncLCD.h>
#include <MeterLCD.h>

EEProm              _EEProm;
FreqControl         Freq_Control;
FuncLCD 	        Func_LCD;
MeterLCD            Meter_LCD;

#define WIDE        0
#define NARROW      1

#define ONE         0
#define FOUR        1

uint8_t         functionNum;

uint8_t         agcMode;
extern uint8_t  rate;
uint8_t         mode;
extern uint16_t TuningRate;

boolean         gpsDisplayFlag = 0;
uint16_t        dummyUpdate = 0;

uint8_t TR = 0;

enum
{
    AMP,
    PHS
};

uint8_t         IQ_SEL = AMP;
uint8_t         IQ_COR_FLAG = 0;


FuncDial::FuncDial(void)
{

}

void FuncDial::begin(void)
{
  	functionNum = 1;

    agcMode = _EEProm.getAGC();
    //set inital AGC mode here
    
    rate = _EEProm.getRate();
    Freq_Control.setTuningRate(rate);

    mode = _EEProm.getVFOA_Mode();


  	Func_LCD.init(rate, mode, _EEProm.getVFOA_Band());
}

void FuncDial::select(void)                                                       // if Function Dial pressed...
{
    functionNum++;
	if(functionNum == END_FUNCTIONS)
    {
        functionNum = START_FUNCTIONS + 1;
        Meter_LCD.clear();
    }
	Func_LCD.update(FUNC_Cmnd + functionNum - 1);
    Meter_LCD.functionDisplay(functionNum);
    if(functionNum == IQ_CORRECTION) IQ_COR_FLAG = 1;
    else IQ_COR_FLAG = 0;
}

void FuncDial::CW(void)                                                           // if Function Dial turned clockwise...
{
	if (functionNum == BAND_SELECT)                                               // Increment Band
  	{
        uint8_t Band = _EEProm.getVFOA_Band();
        if(Band == 10) Band = 0;
        else Band++;
        Freq_Control.bandChangeVFOA(Band);
        _EEProm.storeVFOA_Band(Band);
    }

    else if (functionNum == MODE_SELECT)                                          // Increment Mode
    {
        uint8_t mode = _EEProm.getVFOA_Mode();
        if(mode == 2) mode = 0;
        else mode++;
        Freq_Control.changeCurrentBandMode(mode);
        Freq_Control.updateVFOA(&dummyUpdate);
        Func_LCD.update(MODE_Cmnd + mode);
        _EEProm.storeVFOA_Mode(mode);
    }

    else if (functionNum == RATE_SELECT)                                          // Increment Rate
    {
        if(rate == 3) rate = 0;
        else rate++;
        Freq_Control.setTuningRate(rate);
        Func_LCD.update(RATE_Cmnd + rate);
        _EEProm.storeRate(rate);
    }

    else if (functionNum == GPS_SELECT)                                           // GPS Frequency Error
    {
        
    }

    else if (functionNum == IQ_CORRECTION)                                        // IQ equalization
    {
        if(IQ_SEL == AMP) IQ_SEL = PHS;
        else IQ_SEL = AMP;
        Meter_LCD.updateIQ_CorrectionDisplay(IQ_SEL);
    }
}


void FuncDial::CCW(void)                                                          // if Function Dial turned counter-clockwise...
{
	if(functionNum == BAND_SELECT)                                                // Decrement Band
    {
        uint8_t Band = _EEProm.getVFOA_Band();
        if(Band == 0) Band = 10;
        else Band--;
        Freq_Control.bandChangeVFOA(Band);
        _EEProm.storeVFOA_Band(Band);
    }

    else if(functionNum == MODE_SELECT)                                           // Decrement Mode
    {
        uint8_t mode = _EEProm.getVFOA_Mode();
        if(mode == 0) mode = 2;
        else mode--;
        Freq_Control.changeCurrentBandMode(mode);
        Freq_Control.updateVFOA(&dummyUpdate);
        Func_LCD.update(MODE_Cmnd + mode);
        _EEProm.storeVFOA_Mode(mode);
    }

    else if(functionNum == RATE_SELECT)                                           // Decrement Rate
    {
        if(rate == 0) rate = 3;
        else rate--;
        Freq_Control.setTuningRate(rate);
        Func_LCD.update(RATE_Cmnd + rate);
        _EEProm.storeRate(rate);
    }

    else if (functionNum == GPS_SELECT)                                           // GPS Frequency Error
    {
        
    }

    else if (functionNum == IQ_CORRECTION)                                        // IQ Correction select either amplitude or phase
    {
       if(IQ_SEL == AMP) IQ_SEL = PHS;
        else IQ_SEL = AMP;
        Meter_LCD.updateIQ_CorrectionDisplay(IQ_SEL);
    }
}