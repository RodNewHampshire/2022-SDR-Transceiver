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

#include <EEProm.h>
#include <GainDial.h>
#include <FuncLCD.h>
#include <MeterLCD.h>
#include <PTTControl.h>
#include <RelayDriver.h>
#include <TeensyControl.h>
#include <Wire.h>

EEProm              EE_Prom;
FuncLCD             func_LCD1;
MeterLCD            _meter_Lcd;
RelayDriver         _relay_Driver;
TeensyControl       _teensy_Control;

#define I2C_DS1807      				0x50 >> 1           // I2C addresses
#define DISABLE_ZEROCROSSING_DETECTION 	0xBE
#define AF_SPKR_GAIN         			0xA9                // DS1807 Potentiometer I2C addresses
#define AF_LINE_GAIN     				0xAA

uint8_t         gainFunc;

extern uint8_t  IQ_COR_FLAG;
extern uint8_t  IQ_SEL;
int16_t         IQ_COR_MIN = -5000;
int16_t         IQ_COR_MAX = 5000;
int16_t         IQ_COR_INC = 5;

extern boolean  pttStatus;

enum 									                    // function menu items are displayed in this order
{
	START_GAINS,					                        // START_GAINS & END_GAINS are used to auromatically define the start and end of the function list
	AF,
	RF,
	IF,
	LINE_OUT,
	END_GAINS
};


GainDial::GainDial(void)
{

}


void GainDial::begin(void)
{
  	gainFunc = 1;
  	disableZeroCrossingDetection();
  	setAFGain(EE_Prom.getAF_Gain());
  	setLineGain(EE_Prom.getLN_Gain());
    setRFAttn(EE_Prom.getRF_Attn());
    _teensy_Control.setRX_IF_Gain(EE_Prom.getIF_Gain() - 60);
    delay(5);
    updateDSP_IQ_Correction(EE_Prom.getVFOA_Band());
    func_LCD1.selectAF_Gain();
    func_LCD1.dsplyAF_Gain(EE_Prom.getAF_Gain());
    func_LCD1.dsplyIF_Gain(EE_Prom.getIF_Gain() - 60);
    func_LCD1.dsplyLN_Gain(EE_Prom.getLN_Gain());
    func_LCD1.update(ATTN_Cmnd + EE_Prom.getRF_Attn());
}

void GainDial::select(void)                                 // if Gain Dial pressed...
{
	gainFunc++;
	if(gainFunc == END_GAINS) gainFunc = START_GAINS + 1;
	switch (gainFunc)
	{
		case AF:                      
    	func_LCD1.selectAF_Gain();
    	break;

    	case RF:
    	func_LCD1.selectRF_Gain();
    	break;

    	case IF:
    	func_LCD1.selectIF_Gain();
    	break;

    	case LINE_OUT:
    	func_LCD1.selectLN_Gain();
    	break;
	}
}


void GainDial::CCW(void)                                    // if Gain Dial turned counter-clockwise
{
	if(IQ_COR_FLAG)                                         // if in IQ correction mode, handle outside standard switch function
    {
        incrementIQ_Correction();
        return;
    }

    switch (gainFunc)
  	{
    	case AF:                         
    	incrementAFGain();
    	break;

    	case RF:
    	incrementRFAttn();
    	break;

    	case IF:
    	incrementIFGain();
    	break;

    	case LINE_OUT:
    	incrementLineGain();
    	break;
   }
}


void GainDial::CW(void)                                     // if Gain Dial tunred clockwise
{
	if(IQ_COR_FLAG)                                         // if in IQ correction mode, handle outside standard switch function
    {
        decrementIQ_Correction();
        return;
    }

    switch (gainFunc)
  	{
    	case AF:                         
    	decrementAFGain();
    	break;

    	case RF:
    	decrementRFAttn();
    	break;

    	case IF:
    	decrementIFGain();
    	break;

    	case LINE_OUT:
    	decrementLineGain();
    	break;
	}
}


void GainDial::incrementAFGain(void)
{
	uint8_t AF_Gain = EE_Prom.getAF_Gain();
    if(AF_Gain == 64) return;
	AF_Gain = AF_Gain + 8;
	setAFGain(AF_Gain);
	func_LCD1.dsplyAF_Gain(AF_Gain);
    EE_Prom.storeAF_Gain(AF_Gain);
}


void GainDial::decrementAFGain(void)
{
	uint8_t AF_Gain = EE_Prom.getAF_Gain();
    if(AF_Gain == 0) return;
	else AF_Gain = AF_Gain - 8;
	setAFGain(AF_Gain);
	func_LCD1.dsplyAF_Gain(AF_Gain);
    EE_Prom.storeAF_Gain(AF_Gain);
}


void GainDial::incrementRFAttn(void)
{
	uint8_t RF_Attn = EE_Prom.getRF_Attn();
    uint8_t Pre_Amp = EE_Prom.getPreAmp();
    if(RF_Attn == 0) return;
    else RF_Attn = RF_Attn - 1;
    setRFAttn(RF_Attn);
    func_LCD1.update(ATTN_Cmnd + RF_Attn);
    EE_Prom.storeRF_Attn(RF_Attn);
}


void GainDial::decrementRFAttn(void)
{
	uint8_t RF_Attn = EE_Prom.getRF_Attn();
    uint8_t Pre_Amp = EE_Prom.getPreAmp();
    if(RF_Attn == 15) return;
    else RF_Attn = RF_Attn + 1;
    setRFAttn(RF_Attn);
    func_LCD1.update(ATTN_Cmnd + RF_Attn);
    EE_Prom.storeRF_Attn(RF_Attn);
}


void GainDial::incrementIFGain(void)                        // store values from 0 to 70, display and send values -60 to +30 dB
{
	int16_t IF_Gain = EE_Prom.getIF_Gain();
    if(IF_Gain == 90) IF_Gain = 90;
    else IF_Gain = IF_Gain + 5;                        
    func_LCD1.dsplyIF_Gain(IF_Gain - 60);
    _teensy_Control.setRX_IF_Gain(IF_Gain - 60);
    EE_Prom.storeIF_Gain(IF_Gain);
}


void GainDial::decrementIFGain(void)
{
	int8_t IF_Gain = EE_Prom.getIF_Gain();
    if(IF_Gain == 0) IF_Gain = 0;
    else IF_Gain = IF_Gain - 5;
    func_LCD1.dsplyIF_Gain(IF_Gain - 60);
    _teensy_Control.setRX_IF_Gain(IF_Gain - 60);
    EE_Prom.storeIF_Gain(IF_Gain);
}


void GainDial::incrementLineGain(void)
{
	uint8_t LN_Gain = EE_Prom.getLN_Gain();
    if(LN_Gain == 64) LN_Gain = 64;
	else LN_Gain = LN_Gain + 4;
	setLineGain(LN_Gain);
	func_LCD1.dsplyLN_Gain(LN_Gain);
    EE_Prom.storeLN_Gain(LN_Gain);
}


void GainDial::decrementLineGain(void)
{
	uint8_t LN_Gain = EE_Prom.getLN_Gain();
    if(LN_Gain == 0) LN_Gain = 0;
	else LN_Gain = LN_Gain - 4;
	setLineGain(LN_Gain);
	func_LCD1.dsplyLN_Gain(LN_Gain);
    EE_Prom.storeLN_Gain(LN_Gain);
}


void GainDial::setAFGain(uint8_t level)
{
    Wire.beginTransmission(I2C_DS1807);
    Wire.write(AF_SPKR_GAIN);
    Wire.write(0x40-level);
    Wire.endTransmission();
}


void GainDial::setLineGain(uint8_t level)
{
    Wire.beginTransmission(I2C_DS1807);
    Wire.write(AF_LINE_GAIN);
    Wire.write(0x40-level);
    Wire.endTransmission();
}


void GainDial::setRFAttn(uint8_t level)
{
    _relay_Driver.RFATTN(level);
}


void GainDial::incrementIQ_Correction(void)
{
   
    int16_t correction;
    uint8_t band  = EE_Prom.getVFOA_Band();

    if(IQ_SEL == 0)                                                                 // increment IQ amplitude correction
    {
        if(pttStatus == RCV) correction = EE_Prom.getIQ_RxAmpCorr(band);   
        else correction = EE_Prom.getIQ_TxAmpCorr(band);
        
        correction = correction + IQ_COR_INC;               
        if(correction > IQ_COR_MAX) correction = IQ_COR_MAX;
        
        if(pttStatus == RCV) 
        {
            EE_Prom.storeIQ_RxAmpCorr(correction, band); 
            _teensy_Control.setRX_IQ_Amp(correction);
        }
        else 
        {
            EE_Prom.storeIQ_TxAmpCorr(correction, band);
            _teensy_Control.setTX_IQ_Amp(correction);
        }

        _meter_Lcd.updateIQ_AmpCorrectionDisplay((float)correction/1000);
    }
    
    else if(IQ_SEL == 1)                                                            // increment IQ phase correction
    {
        if(pttStatus == RCV) correction = EE_Prom.getIQ_RxPhsCorr(band); 
        else correction = EE_Prom.getIQ_TxPhsCorr(band);

        correction = correction + IQ_COR_INC;
        if(correction > IQ_COR_MAX) correction = IQ_COR_MAX;

        if(pttStatus == RCV)
        {
            EE_Prom.storeIQ_RxPhsCorr(correction, band);
            _teensy_Control.setRX_IQ_Phs(correction);
        }
        else 
        {
            EE_Prom.storeIQ_TxPhsCorr(correction, band);
            _teensy_Control.setTX_IQ_Phs(correction);
        }
        
        _meter_Lcd.updateIQ_PhsCorrectionDisplay((float)correction/1000);
    }
}


void GainDial::decrementIQ_Correction(void)
{
    int16_t correction;
    uint8_t band  = EE_Prom.getVFOA_Band();

    if(IQ_SEL == 0)                                                                 // decrement IQ amplitude correction
    {
        if(pttStatus == RCV) correction = EE_Prom.getIQ_RxAmpCorr(band); 
        else correction = EE_Prom.getIQ_TxAmpCorr(band);

        correction = correction - IQ_COR_INC;
        if(correction < IQ_COR_MIN) correction = IQ_COR_MIN;
        
        if(pttStatus == RCV) EE_Prom.storeIQ_RxAmpCorr(correction, band);   
        else EE_Prom.storeIQ_TxAmpCorr(correction, band);

        if(pttStatus == RCV) 
        {
            EE_Prom.storeIQ_RxAmpCorr(correction, band); 
            _teensy_Control.setRX_IQ_Amp(correction);
        }
        else 
        {
            EE_Prom.storeIQ_TxAmpCorr(correction, band);
            _teensy_Control.setTX_IQ_Amp(correction);
        }

        _meter_Lcd.updateIQ_AmpCorrectionDisplay((float)correction/1000);
    }
    
    else if(IQ_SEL == 1)                                                            // decrement IQ phase correction
    {
        if(pttStatus == RCV) correction = EE_Prom.getIQ_RxPhsCorr(band); 
        else correction = EE_Prom.getIQ_TxPhsCorr(band);

        correction = correction - IQ_COR_INC;
        if(correction < IQ_COR_MIN) correction = IQ_COR_MIN;
        
        if(pttStatus == RCV) EE_Prom.storeIQ_RxPhsCorr(correction, band);
        else EE_Prom.storeIQ_TxPhsCorr(correction, band);

        if(pttStatus == RCV)
        {
            EE_Prom.storeIQ_RxPhsCorr(correction, band);
            _teensy_Control.setRX_IQ_Phs(correction);
        }
        else 
        {
            EE_Prom.storeIQ_TxPhsCorr(correction, band);
            _teensy_Control.setTX_IQ_Phs(correction);
        }

        _meter_Lcd.updateIQ_PhsCorrectionDisplay((float)correction/1000);
    }
}


void GainDial::updateDSP_IQ_Correction(uint8_t band)
{
    _teensy_Control.setRX_IQ_Amp(EE_Prom.getIQ_RxAmpCorr(band));
    delay(5);
    _teensy_Control.setRX_IQ_Phs(EE_Prom.getIQ_RxPhsCorr(band));
    delay(5);
    _teensy_Control.setTX_IQ_Amp(EE_Prom.getIQ_TxAmpCorr(band));
    delay(5);
    _teensy_Control.setTX_IQ_Phs(EE_Prom.getIQ_TxPhsCorr(band));
}


void GainDial::disableZeroCrossingDetection(void)
{
    Wire.beginTransmission(I2C_DS1807);
    Wire.write(DISABLE_ZEROCROSSING_DETECTION);
    Wire.endTransmission();
}