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
#include <Adafruit_INA219.h>
#include <FuncLCD.h>
#include <PA_INA219.h>
#include <Wire.h>

Adafruit_INA219 _ina219;


PA_INA219::PA_INA219(void)
{

}


void PA_INA219::begin(void)
{
    _ina219.setCalibration_32V_1A();
    _ina219.begin();
}


void PA_INA219::getPAVoltAmps(float *shuntVoltage, float *busVoltage, float *loadAmpTicks)
{ 
    *shuntVoltage = _ina219.getShuntVoltage_mV();
    *busVoltage	= _ina219.getBusVoltage_V();
    *loadAmpTicks = _ina219.getCurrent_mA();
}