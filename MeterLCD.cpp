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

//#define DEBUG

#include "Adafruit_LiquidCrystal.h"
#include <arduino.h>
#include <EEProm.h>
#include <FuncLCD.h>
#include <PA_INA219.h>
#include <PTTControl.h>
#include <MeterLCD.h>

EEProm              eeProm;
PA_INA219           ina219;
FuncLCD             functionLCD;


char _Disp_VFOA[21] =       {" VFO A:  ,   .   KHz"};
char _Disp_GPS[21] =        {" VFO/LO:     .   KHz"};
char _Disp_IQ_Corr[21] =    {" IQ Amp       IQ Phs"};

char _Disp_XMT_TIMEOUT[21] = {"*****XMT TIMEOUT****"};

extern uint8_t gpsSelectFlag;
extern boolean gpsDisplayFlag;

extern uint8_t  IQ_EQL_FLAG;

enum
{
    AMP,
    PHS
};

const char CLEAR[21] = {"                    "};

char SMeterCal[21] = {'A','G','C',' ','L','e','v','e','l',':',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\0'};

const char L2_S0[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S1[21] = {'1', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S2[21] = {0x20, '2', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S3[21] = {0x20, 0x20, '3', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S4[21] = {0x20, 0x20, 0x20, '4', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S5[21] = {0x20, 0x20, 0x20, 0x20, '5', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S6[21] = {0x20, 0x20, 0x20, 0x20, 0x20, '6', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S7[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '7', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S8[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '8', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S9[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '9', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S9_10[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '+', '1', '0', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S9_20[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '+', '2', '0', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S9_30[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '+', '3', '0', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S9_40[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '+', '4', '0', 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S9_50[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '+', '5', '0', 0x20, 0x20, 0x20, 0x20, '\0'};
const char L2_S9_60[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '+', '6', '0', 0x20, 0x20, 0x20, '\0'};

const char L3_S0[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S1[21] = {0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S2[21] = {0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S3[21] = {0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S4[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S5[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S6[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S7[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S8[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S9[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S9_10[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S9_20[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S9_30[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S9_40[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S9_50[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char L3_S9_60[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};


char PA_Volts[21]         = {'P','A',' ','V','o','l','t','s',':',' ',' ',' ',' ','.',' ',' ',' ','V',' ',' ','\0'};
char PA_Amps[21]          = {'P','A',' ','A','m','p','s',':',' ',' ',' ',' ',' ','.',' ',' ',' ','A',' ',' ','\0'};

const char Amps_0[21] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_02[21] = {0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_04[21] = {0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_06[21] = {0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_08[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_10[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_12[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_14[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_16[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_18[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_20[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_22[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_24[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_26[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_28[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_30[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_32[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, '\0'};
const char Amps_34[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, 0x20, '\0'};
const char Amps_36[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x20, '\0'};
const char Amps_38[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, '\0'};
const char Amps_40[21] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, '\0'};

float loadVolts = 0;
float loadAmps = 0;

float shuntVoltage = 0;
float busVoltage = 0;
float loadAmpTicks = 0;

float sumShuntVoltage = 0;
float sumBusVoltage = 0;
float sumLoadAmpTicks = 0;

int     inaReadCount = 0;

#define INA_READ_PERIOD   10

uint8_t gpsSelectFlag = 0;

enum
{
  volts,
  amps
};


Adafruit_LiquidCrystal _meterLCD(0);


MeterLCD::MeterLCD(void)
{

}


void MeterLCD::begin(void)
{
    functionLCD.setI2CForLCD();
    _meterLCD.begin(20, 4);
    ina219.begin();
    #ifdef DEBUG
    Serial.println("MeterLCD Debug Enabled");
    #endif
}


void MeterLCD::clear(void)
{
    _meterLCD.clear();
}


void MeterLCD::clearDisplayRow_1(void)
{
    _meterLCD.setCursor(0, 1);
    _meterLCD.print(CLEAR);
}


void MeterLCD::splash(void)
{
    _meterLCD.setCursor(0, 0);
 	_meterLCD.print(F("      METER LCD     "));
  	_meterLCD.setCursor(0, 1);
  	_meterLCD.print(F("  HIGH PERFORMANCE  "));
  	_meterLCD.setCursor(0, 2);
  	_meterLCD.print(F("   HF TRANSCEIVER   "));
  	_meterLCD.setCursor(0, 3);
  	_meterLCD.print(F("      BY AD5GH      "));
}


void MeterLCD::updateVFOADisplay(uint32_t *frequency, uint8_t *mode)
{
    long VFO_A_Adjusted;

    String VFOA_FreqDisp;

    VFOA_FreqDisp = String(*frequency, DEC);

    if (*frequency < 10000000)
    {
      char i;
      for (i = 6; i > 0; i--) VFOA_FreqDisp[i] = VFOA_FreqDisp[i - 1];
      VFOA_FreqDisp[0] = 0x20;
    }

    _Disp_VFOA[7] = VFOA_FreqDisp[0];
    _Disp_VFOA[8] = VFOA_FreqDisp[1];
    _Disp_VFOA[10] = VFOA_FreqDisp[2];
    _Disp_VFOA[11] = VFOA_FreqDisp[3];
    _Disp_VFOA[12] = VFOA_FreqDisp[4];
    _Disp_VFOA[14] = VFOA_FreqDisp[5];
    _Disp_VFOA[15] = VFOA_FreqDisp[6];
    _Disp_VFOA[16] = 0x30;                                        // "1" Hertz digit is always zero
    _meterLCD.setCursor(0, 0);
    _meterLCD.print(_Disp_VFOA);
}

void MeterLCD::updateGPSErrorDisplay(void)                        // Called every ~30 seconds to update GPS derived frequency error
{
  /*  int16_t error;
    int16_t errorMixer = 0;
    int16_t errorProdDet = 0;

    functionLCD.setI2CForLCD();

    errorMixer = eeProm.getSi5351MixerError();
    errorProdDet = eeProm.getSi5351ProdDetError();

    if(gpsDisplayFlag == 1)
    {
        char GPS_ErrorDisp[10];

        if(gpsSelectFlag == 0)
            {
                _Disp_GPS[1] = 'V';
                _Disp_GPS[2] = 'F';
                _Disp_GPS[3] = 'O';
                _Disp_GPS[4] = '/';
                _Disp_GPS[5] = 'L';
                _Disp_GPS[6] = 'O';

                itoa(errorMixer, GPS_ErrorDisp, 10);
                #ifdef DEBUG
                Serial.print("Mixer GPS Correction: ");
                Serial.println(GPS_ErrorDisp);
                #endif

                gpsSelectFlag = 1;
                error = errorMixer;
            }

        else
            {
                _Disp_GPS[1] = 'B';
                _Disp_GPS[2] = 'F';
                _Disp_GPS[3] = 'O';
                _Disp_GPS[4] = ' ';
                _Disp_GPS[5] = ' ';
                _Disp_GPS[6] = ' ';

                itoa(errorProdDet, GPS_ErrorDisp, 10);
                #ifdef DEBUG
                Serial.print("BFO GPS Correction: ");
                Serial.println(GPS_ErrorDisp);
                #endif

                gpsSelectFlag = 0;
                error = errorProdDet;
            }
    
        if(error >0) 
            {
                for(uint8_t n=0; n<10; n++)
                {
                    GPS_ErrorDisp[n+1] = GPS_ErrorDisp[n];
                }
                GPS_ErrorDisp[0] = ' ';
            }

        if(abs(error) < 10)
            {
                _Disp_GPS[11] = GPS_ErrorDisp[0];
                _Disp_GPS[12] = '0';
                _Disp_GPS[14] = '0';
                _Disp_GPS[15] = '0';
                _Disp_GPS[16] = GPS_ErrorDisp[1];
            }

        else if(abs(error) < 100)
            {
                _Disp_GPS[11] = GPS_ErrorDisp[0];
                _Disp_GPS[12] = '0';
                _Disp_GPS[14] = '0';
                _Disp_GPS[15] = GPS_ErrorDisp[1];
                _Disp_GPS[16] = GPS_ErrorDisp[2];
            }


        else if(abs(error) < 1000) 
            {
                _Disp_GPS[11] = GPS_ErrorDisp[0];
                _Disp_GPS[12] = '0';
                _Disp_GPS[14] = GPS_ErrorDisp[1];
                _Disp_GPS[15] = GPS_ErrorDisp[2];
                _Disp_GPS[16] = GPS_ErrorDisp[3];
            }

        else if(abs(error) >= 1000) 
            {
            _Disp_GPS[11] = GPS_ErrorDisp[0];
            _Disp_GPS[12] = GPS_ErrorDisp[1];
            _Disp_GPS[14] = GPS_ErrorDisp[2];
            _Disp_GPS[15] = GPS_ErrorDisp[3];
            _Disp_GPS[16] = GPS_ErrorDisp[4];
            }

        _meterLCD.setCursor(0, 1);
        _meterLCD.print(_Disp_GPS);
    }*/
}


void MeterLCD::updateIQ_CorrectionDisplay(uint8_t select)
{
    _meterLCD.setCursor(0, 1);
    _meterLCD.print(F("                    "));
    _meterLCD.setCursor(0, 2);
    _meterLCD.print(_Disp_IQ_Corr);
    
    if(select == AMP)
    {
        _meterLCD.setCursor(0, 2); 
        _meterLCD.print("*");
        _meterLCD.setCursor(13, 2);
        _meterLCD.print(" ");

    }
    else if(select == PHS)
    {
        _meterLCD.setCursor(13, 2);
        _meterLCD.print("*");
        _meterLCD.setCursor(0, 2);
        _meterLCD.print(" ");
    }
}

void MeterLCD::updateIQ_AmpCorrectionDisplay(float Amp)
{
    _meterLCD.setCursor(1, 3);
    _meterLCD.print(Amp, 3);
    if(Amp == 0 || Amp > 0)
    {
        _meterLCD.setCursor(6, 3);
        _meterLCD.print(" ");
    }
}


void MeterLCD::updateIQ_PhsCorrectionDisplay(float Phs)
{
    _meterLCD.setCursor(14, 3);
    _meterLCD.print(Phs, 3);
    if(Phs == 0 || Phs > 0)
    {
        _meterLCD.setCursor(19, 3);
        _meterLCD.print(" ");
    }
}


void MeterLCD::indicateVFOA(uint8_t pttStatus)
{
    char temp;

    if(pttStatus == 1) temp = '!';
    else temp = '*';

    if (_Disp_VFOA[0] == temp) _Disp_VFOA[0] = ' ';
    else _Disp_VFOA[0] = temp;
    _meterLCD.setCursor(0, 0);
    _meterLCD.print(_Disp_VFOA[0]);
}


void MeterLCD::eepromStart(void)
{
    _meterLCD.clear();
    _meterLCD.print(F("EEPROM"));
}


void MeterLCD:: eepromNoDataFound(void)                     // updates Line 2 while leaving Line 1 unchaged
{
    _meterLCD.setCursor(0, 1);
    _meterLCD.print(F("                    "));
    _meterLCD.setCursor(0, 1);
    _meterLCD.print(F("No Data Found"));
    delay(1000);
}


void MeterLCD:: eepromInitializing(void)                    // updates Line 2 while leaving Line 1 unchaged
{
    _meterLCD.setCursor(0, 1);
    _meterLCD.print(F("                    "));
    _meterLCD.setCursor(0, 1);
    _meterLCD.print(F("Initializing..."));
    delay(1000);
}


void MeterLCD:: eepromReadingData(void)                     // updates Line 2 while leaving Line 1 unchaged
{
    _meterLCD.setCursor(0, 1);
    _meterLCD.print(F("                    "));
    _meterLCD.setCursor(0, 1);
    _meterLCD.print(F("Reading Data..."));
    delay(1000);
}


void MeterLCD::functionDisplay(uint8_t functionNum)
{
    if(functionNum == 1)
    {
        uint32_t freq = eeProm.getVFOA_Freq();
        uint8_t mode = eeProm.getVFOA_Mode();
        updateVFOADisplay(&freq, &mode);
    }


    if(functionNum == GPS_SELECT)                           // display GPS measured Si5351 Xtal frequency error
    {
        gpsDisplayFlag = 1;
        updateGPSErrorDisplay();
    }
    else if(gpsDisplayFlag == 1)                            // if Function 6 not selected, clear Xtal frequency error display    
    {
        gpsDisplayFlag = 0;
        clearDisplayRow_1();
    }


    if(functionNum == IQ_CORRECTION)
    {
        updateIQ_CorrectionDisplay(AMP);

        uint8_t band  = eeProm.getVFOA_Band();
        updateIQ_AmpCorrectionDisplay((float) eeProm.getIQ_RxAmpCorr(band) / 1000);
        updateIQ_PhsCorrectionDisplay((float) eeProm.getIQ_RxPhsCorr(band) / 1000);
    }
}


void MeterLCD::initializeRxDisplay(void)
{
  _meterLCD.setCursor(0, 1);
  _meterLCD.print(Amps_0);
  _meterLCD.setCursor(0, 2);
  _meterLCD.print(Amps_0);
  _meterLCD.setCursor(0, 3);
  _meterLCD.print(Amps_0);
}


void MeterLCD::initializeTxDisplay(void)
{
  _meterLCD.setCursor(0, 1);
  _meterLCD.print(PA_Volts);
  _meterLCD.setCursor(0, 2);
  _meterLCD.print(PA_Amps);
  _meterLCD.setCursor(0, 3);
  _meterLCD.print(Amps_0);
}


void MeterLCD::pttTimeout(uint8_t display)
{
    _meterLCD.setCursor(0, 1);

    if(display == 1) _meterLCD.print(_Disp_XMT_TIMEOUT);
    else _meterLCD.print(CLEAR);
}


void MeterLCD::updatePADisplay(void)
{
    String paVolts, paAmps;

    ina219.getPAVoltAmps(&shuntVoltage, &busVoltage, &loadAmpTicks);

   if (inaReadCount == INA_READ_PERIOD)
    {
      loadVolts = sumBusVoltage/INA_READ_PERIOD + sumShuntVoltage/(INA_READ_PERIOD*5000);
      loadAmps = sumLoadAmpTicks / (INA_READ_PERIOD*1000);
      if(loadVolts < 0.0) loadVolts = 0.0;
      if(loadAmps < 0.0) loadAmps = 0.0;

      sumShuntVoltage = 0;
      sumBusVoltage = 0;
      sumLoadAmpTicks = 0;

      inaReadCount = 0;

      paVolts = String(loadVolts, 2);
      paAmps = String(loadAmps, 3);

    
      if (loadVolts < 10)
      {
        char i;
        for (i = 6; i > 0; i--) paVolts[i] = paVolts[i - 1];
        paVolts[0] = 0x20;
      }
    
      if (loadAmps < 10)
      {
        char i;
        for (i = 6; i > 0; i--) paAmps[i] = paAmps[i - 1];
        paAmps[0] = 0x20;
      }

      PA_Volts[11] = paVolts[0];
      PA_Volts[12] = paVolts[1];
      PA_Volts[13] = paVolts[2];
      PA_Volts[14] = paVolts[3];
      PA_Volts[15] = paVolts[4];
      _meterLCD.setCursor(0, 1);
      _meterLCD.print(PA_Volts);

      PA_Amps[11] = paAmps[0];
      PA_Amps[12] = paAmps[1];
      PA_Amps[13] = paAmps[2];
      PA_Amps[14] = paAmps[3];
      PA_Amps[15] = paAmps[4];
      _meterLCD.setCursor(0, 2);
      _meterLCD.print(PA_Amps);

      if(loadAmps < 0.2) updateAmpBars(0, amps);                              
      else if(loadAmps >= 0.2 && loadAmps < 0.4) updateAmpBars(1, amps);
      else if(loadAmps >= 0.4 && loadAmps < 0.6) updateAmpBars(2, amps);
      else if(loadAmps >= 0.6 && loadAmps < 0.8) updateAmpBars(3, amps);
      else if(loadAmps >= 0.8 && loadAmps < 1.0) updateAmpBars(4, amps);
      else if(loadAmps >= 1.0 && loadAmps < 1.2) updateAmpBars(5, amps);
      else if(loadAmps >= 1.2 && loadAmps < 1.4) updateAmpBars(6, amps);
      else if(loadAmps >= 1.4 && loadAmps < 1.6) updateAmpBars(7, amps);
      else if(loadAmps >= 1.6 && loadAmps < 1.8) updateAmpBars(8, amps);
      else if(loadAmps >= 1.8 && loadAmps < 2.0) updateAmpBars(9, amps);
      else if(loadAmps >= 2.0 && loadAmps < 2.2) updateAmpBars(10, amps);
      else if(loadAmps >= 2.2 && loadAmps < 2.4) updateAmpBars(11, amps);
      else if(loadAmps >= 2.4 && loadAmps < 2.6) updateAmpBars(12, amps);
      else if(loadAmps >= 2.6 && loadAmps < 2.8) updateAmpBars(13, amps);
      else if(loadAmps >= 2.8 && loadAmps < 3.0) updateAmpBars(14, amps);
      else if(loadAmps >= 3.0 && loadAmps < 3.2) updateAmpBars(15, amps);
      else if(loadAmps >= 3.2 && loadAmps < 3.4) updateAmpBars(16, amps);
      else if(loadAmps >= 3.4 && loadAmps < 3.6) updateAmpBars(17, amps);
      else if(loadAmps >= 3.6 && loadAmps < 3.8) updateAmpBars(18, amps);
      else if(loadAmps >= 3.8 && loadAmps < 4.0) updateAmpBars(19, amps);
      else if(loadAmps >= 4.0) updateAmpBars(20, amps);


     /* if(loadVolts < 12.00) updateAmpBars(0, volts);                              // ***not displaying voltage bars, but leaving code here for future use
      else if(loadVolts >= 12.1 && loadVolts < 12.2) updateAmpBars(1, volts);
      else if(loadVolts >= 12.2 && loadVolts < 12.3) updateAmpBars(2, volts);
      else if(loadVolts >= 12.3 && loadVolts < 12.4) updateAmpBars(3, volts);
      else if(loadVolts >= 12.4 && loadVolts < 12.5) updateAmpBars(4, volts);
      else if(loadVolts >= 12.5 && loadVolts < 12.6) updateAmpBars(5, volts);
      else if(loadVolts >= 12.6 && loadVolts < 12.7) updateAmpBars(6, volts);
      else if(loadVolts >= 12.7 && loadVolts < 12.8) updateAmpBars(7, volts);
      else if(loadVolts >= 12.8 && loadVolts < 12.9) updateAmpBars(8, volts);
      else if(loadVolts >= 12.9 && loadVolts < 13.0) updateAmpBars(9, volts);
      else if(loadVolts >= 13.0 && loadVolts < 13.1) updateAmpBars(10, volts);
      else if(loadVolts >= 13.1 && loadVolts < 13.2) updateAmpBars(11, volts);
      else if(loadVolts >= 13.2 && loadVolts < 13.3) updateAmpBars(12, volts);
      else if(loadVolts >= 13.3 && loadVolts < 13.4) updateAmpBars(13, volts);
      else if(loadVolts >= 13.4 && loadVolts < 13.5) updateAmpBars(14, volts);
      else if(loadVolts >= 13.5 && loadVolts < 13.6) updateAmpBars(15, volts);
      else if(loadVolts >= 13.6 && loadVolts < 13.7) updateAmpBars(16, volts);
      else if(loadVolts >= 13.7 && loadVolts < 13.8) updateAmpBars(17, volts);
      else if(loadVolts >= 13.8 && loadVolts < 13.9) updateAmpBars(18, volts);
      else if(loadVolts >= 13.9 && loadVolts < 14.0) updateAmpBars(19, volts);
      else if(loadVolts >= 14.00) updateAmpBars(20, volts);*/
    }

    else
    {
      sumShuntVoltage += shuntVoltage;
      sumBusVoltage += busVoltage;
      sumLoadAmpTicks += loadAmpTicks;
      inaReadCount++;
    }
}


void MeterLCD::updateAmpBars(uint8_t level, uint8_t display)
{
    if(display == amps)   _meterLCD.setCursor(0, 3);
    //if(display == volts)  _meterLCD.setCursor(0, 1);                              // ***not displaying voltage bars, but leaving code here for future use  

    switch (level)
    {
        case 0:
            _meterLCD.print(Amps_0);
            break;

        case 1:
            _meterLCD.print(Amps_02);
            break;

        case 2:
            _meterLCD.print(Amps_04);
            break;

        case 3:
            _meterLCD.print(Amps_06);
            break;

        case 4:
            _meterLCD.print(Amps_08);
            break;

        case 5:
            _meterLCD.print(Amps_10);
            break;

        case 6:
            _meterLCD.print(Amps_12);
            break;

        case 7:
            _meterLCD.print(Amps_14);
            break;

        case 8:
            _meterLCD.print(Amps_16);
            break;

        case 9:
            _meterLCD.print(Amps_18);
            break;

        case 10:
            _meterLCD.print(Amps_20);
            break;

        case 11:
            _meterLCD.print(Amps_22);
            break;

        case 12:
            _meterLCD.print(Amps_24);
            break;

        case 13:
            _meterLCD.print(Amps_26);
            break;

        case 14:
            _meterLCD.print(Amps_28);
            break;

        case 15:
            _meterLCD.print(Amps_30);
            break;

        case 16:
            _meterLCD.print(Amps_32);
            break;

        case 17:
            _meterLCD.print(Amps_34);
            break;

        case 18:
            _meterLCD.print(Amps_36);
            break;

        case 19:
            _meterLCD.print(Amps_38);
            break;

        case 20:
            _meterLCD.print(Amps_40);
            break;
    }
}
