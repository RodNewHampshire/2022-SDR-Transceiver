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

#include <AmpLink.h>
#include <arduino.h>
#include <DetectorExciter.h>
#include <EEProm.h>
#include <FreqControl.h>
#include <FuncLCD.h>
#include <GainDial.h>
#include <MeterLCD.h>
#include <RelayDriver.h>

#define UPPER         1
#define LOWER         0

AmpLink             __ampLink;
DetectorExciter     __DetectorExciter;
EEProm 			        __eeProm;
FuncLCD 		        __FuncLCD;
GainDial            __GainDial;
MeterLCD		        __MeterLCD;
RelayDriver         _relayDriver;

//Index to band arrays:
//0=160M, 1=80M, 2=40M, 3=30M, 4=20M, 5=17M, 6=15M, 7=12M, 8=10Ma, 9=10Mb, 10=10Mc

uint32_t _VFOA_Band_Limits[2], _VFOB_Band_Limits[2];

uint8_t S_METER_CAL[15];
uint8_t S_METER_TABLE[11][15];
extern uint8_t EE_S_METER_TABLE[11][15];

uint16_t _dummyFreqUpdateAccum = 0;

typedef struct                                                                  // structure containing specific operating parameters
{
  uint32_t Freq;
  uint8_t Band;
  uint8_t Mode;
} __FreqBandMode;

__FreqBandMode _VFO_A;

uint8_t   rate;
uint16_t  tuningRate;
uint16_t  tuningRateTable[4] = {10, 100, 250, 1000};


FreqControl::FreqControl(void)
{

}


void FreqControl::begin(void)
{
  for(uint8_t band=0; band<11; band++)                                          // Copy S-meter calibration table from EEPROM
  {
    for(uint8_t sLevel=0; sLevel<15; sLevel++)
    {
      S_METER_TABLE[band][sLevel] = EE_S_METER_TABLE[band][sLevel];
    }
  }
}


void FreqControl::initVFOA(void)
{
  _VFO_A.Freq = __eeProm.getVFOA_Freq();
  _VFO_A.Band = __eeProm.getVFOA_Band();
  _VFO_A.Mode = __eeProm.getVFOA_Mode();
  Set_Band_Limits((long*)&_VFOA_Band_Limits, _VFO_A.Band);
  for(uint8_t n=0; n<15; n++) S_METER_CAL[n] = S_METER_TABLE[_VFO_A.Band][n];
  _relayDriver.BPF(_VFO_A.Band);
  updateVFOA(&_dummyFreqUpdateAccum);
}


void FreqControl::bandChangeVFOA(uint8_t newBand)
{
  __eeProm.storeBand_Freq(_VFO_A.Freq, _VFO_A.Band);		                        // save current band frequency & mode
  __eeProm.storeBand_Mode(_VFO_A.Mode, _VFO_A.Band);

  _VFO_A.Band = newBand;									                                      // get new band frequency & mode
  _VFO_A.Freq = __eeProm.getBand_Freq(newBand);			
  _VFO_A.Mode = __eeProm.getBand_Mode(newBand);

  __eeProm.storeVFOA_Freq(_VFO_A.Freq);					                                // store new VFOA freq, band, & mode
  __eeProm.storeVFOA_Band(_VFO_A.Band);
  __eeProm.storeVFOA_Mode(_VFO_A.Mode);

  _relayDriver.BPF(_VFO_A.Band);
  __GainDial.updateDSP_IQ_Correction(_VFO_A.Band);

  Set_Band_Limits((long*)&_VFOA_Band_Limits, _VFO_A.Band);	                    // set new band limits
    
  __FuncLCD.update(BAND_Cmnd + _VFO_A.Band);				                            // update band display
  __FuncLCD.update(MODE_Cmnd + _VFO_A.Mode);				                            // update mode display
    
  updateVFOA(&_dummyFreqUpdateAccum);						                                // update VFOA
  //__ampLink.updateXmtBand();                                                  // update Amplifier band

  for(uint8_t n=0; n<15; n++) S_METER_CAL[n] = S_METER_TABLE[newBand][n];
}


void FreqControl::updateVFOA(int16_t *_freqUpdate)
{
  _VFO_A.Freq = _VFO_A.Freq + *_freqUpdate;
  if (_VFO_A.Freq < _VFOA_Band_Limits[LOWER]) _VFO_A.Freq = _VFOA_Band_Limits[LOWER];
  if (_VFO_A.Freq > _VFOA_Band_Limits[UPPER]) _VFO_A.Freq = _VFOA_Band_Limits[UPPER];
  __DetectorExciter.vfoUpdate(&_VFO_A.Freq, &_VFO_A.Mode);
  __MeterLCD.updateVFOADisplay(&_VFO_A.Freq, &_VFO_A.Mode);
  __eeProm.storeVFOA_Freq(_VFO_A.Freq);
}


void FreqControl::changeCurrentBandMode(uint8_t mode)
{
  _VFO_A.Mode = mode;
}

void FreqControl::setTuningRate(uint8_t rateUpdate)
{
  tuningRate = tuningRateTable[rateUpdate];
}


/********************BANDS EDGE LIMITS****************************************************************************************/

#define Bottom_160M   1800000
#define Top_160M      2000000

#define Bottom_80M    3500000
#define Top_80M       4000000

#define Bottom_40M    7000000
#define Top_40M       7300000

#define Bottom_30M    10100000
#define Top_30M       10150000

#define Bottom_20M    14000000
#define Top_20M       14350000

#define Bottom_17M    18068000
#define Top_17M       18168000

#define Bottom_15M    21000000
#define Top_15M       21450000

#define Bottom_12M    24890000
#define Top_12M       24990000

#define Bottom_10aM   28000000
#define Top_10aM      28500000

#define Bottom_10bM   28500000
#define Top_10bM      29000000

#define Bottom_10cM   29000000
#define Top_10cM      29700000


void FreqControl::Set_Band_Limits(long *Band_Limit_Array, char Band_Number)
{
  switch (Band_Number)
  {
    case 0:
      *(Band_Limit_Array) = Bottom_160M;
      *(Band_Limit_Array + 1) = Top_160M;
      break;

    case 1:
      *(Band_Limit_Array) = Bottom_80M;
      *(Band_Limit_Array + 1) = Top_80M;
      break;

    case 2:
      *(Band_Limit_Array) = Bottom_40M;
      *(Band_Limit_Array + 1) = Top_40M;
      break;

    case 3:
      *(Band_Limit_Array) = Bottom_30M;
      *(Band_Limit_Array + 1) = Top_30M;
      break;

    case 4:
      *(Band_Limit_Array) = Bottom_20M;
      *(Band_Limit_Array + 1) = Top_20M;
      break;

    case 5:
      *(Band_Limit_Array) = Bottom_17M;
      *(Band_Limit_Array + 1) = Top_17M;
      break;

    case 6:
      *(Band_Limit_Array) = Bottom_15M;
      *(Band_Limit_Array + 1) = Top_15M;
      break;

    case 7:
      *(Band_Limit_Array) = Bottom_12M;
      *(Band_Limit_Array + 1) = Top_12M;
      break;

    case 8:
      *(Band_Limit_Array) = Bottom_10aM;
      *(Band_Limit_Array + 1) = Top_10aM;
      break;

    case 9:
      *(Band_Limit_Array) = Bottom_10bM;
      *(Band_Limit_Array + 1) = Top_10bM;
      break;

    case 10:
      *(Band_Limit_Array) = Bottom_10cM;
      *(Band_Limit_Array + 1) = Top_10cM;
      break;
  }
}


uint8_t FreqControl::freqToBandNumber(int32_t freq)
{
  if(freq >= Bottom_160M && freq <= Top_160M) return (0);
  else if(freq >= Bottom_80M && freq <= Top_80M) return (1);
  else if(freq >= Bottom_40M && freq <= Top_40M) return (2);
  else if(freq >= Bottom_30M && freq <= Top_30M) return (3);
  else if(freq >= Bottom_20M && freq <= Top_20M) return (4);
  else if(freq >= Bottom_17M && freq <= Top_17M) return (5);
  else if(freq >= Bottom_15M && freq <= Top_15M) return (6);
  else if(freq >= Bottom_12M && freq <= Top_12M) return (7);
  else if(freq >= Bottom_10aM && freq <= Top_10aM) return (8);
  else if(freq >= Bottom_10bM && freq <= Top_10bM) return (9);
  else return (10);
}


void FreqControl::catVFO_A_FreqUpdate(int32_t freq)
{
  bandChangeVFOA(freqToBandNumber(freq));
  _VFO_A.Freq = freq;
  
  if (_VFO_A.Freq < _VFOA_Band_Limits[LOWER]) _VFO_A.Freq = _VFOA_Band_Limits[LOWER];
  if (_VFO_A.Freq > _VFOA_Band_Limits[UPPER]) _VFO_A.Freq = _VFOA_Band_Limits[UPPER];

  updateVFOA(&_dummyFreqUpdateAccum);
}

