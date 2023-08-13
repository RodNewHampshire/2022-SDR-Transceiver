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
#include <AmpLink.h>
#include <DetectorExciter.h>
#include <EEProm.h>
#include <GainDial.h>
#include <MeterLCD.h>
#include <PTTControl.h>
#include <RelayDriver.h>
#include <TeensyControl.h>

#define  ONE   0x00
#define  FOUR  0x01

#define  MUTE  0

AmpLink              _ampLink;
DetectorExciter      _DetectorExciter;
EEProm               _eeProm;
GainDial             _gainDial;
MeterLCD             _meter_lcd;
RelayDriver          relayDriver;
TeensyControl        _teensyControl;

extern uint8_t  IQ_COR_FLAG;
extern uint8_t  IQ_SEL;


PTTControl::PTTControl(void)
{

}


void PTTControl::begin(void)
{
  
}


void PTTControl::Xmt(void)   
{
   _gainDial.setAFGain(MUTE);
   _DetectorExciter.disableRcvLO();
   _DetectorExciter.enableXmtLO();
   relayDriver.txMode();
   _teensyControl.txMode();

  if(IQ_COR_FLAG == 1)
  {
   _meter_lcd.updateIQ_CorrectionDisplay(IQ_SEL);

   uint8_t band  = _eeProm.getVFOA_Band();
   _meter_lcd.updateIQ_AmpCorrectionDisplay((float) _eeProm.getIQ_TxAmpCorr(band) / 1000);
   _meter_lcd.updateIQ_PhsCorrectionDisplay((float) _eeProm.getIQ_TxPhsCorr(band) / 1000);
  }

   else _meter_lcd.initializeTxDisplay();
}


void PTTControl::Rcv(void)
{
   //_ampLink.ampToRxMode();
   _DetectorExciter.disableXmtLO();
   delay(10);
   relayDriver.rxMode();
   _teensyControl.rxMode();
   _DetectorExciter.enableRcvLO();
   _gainDial.setAFGain(_eeProm.getAF_Gain());

   if(IQ_COR_FLAG == 1)
  {
   _meter_lcd.updateIQ_CorrectionDisplay(IQ_SEL);

   uint8_t band  = _eeProm.getVFOA_Band();
   _meter_lcd.updateIQ_AmpCorrectionDisplay((float) _eeProm.getIQ_RxAmpCorr(band) / 1000);
   _meter_lcd.updateIQ_PhsCorrectionDisplay((float) _eeProm.getIQ_RxPhsCorr(band) / 1000);
  }
   
  else _meter_lcd.initializeRxDisplay();
}