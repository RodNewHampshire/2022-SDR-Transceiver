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

/* PCF8575C I2C I/O EXPANDER
 *
 *                                              BITS
 *                      7       6       5       4       3       2       1       0
 *
 * I2C Slave Address    L       H       L       L       A2      A1      A0      R/W
 *
 * POx I/O Data Bus     P07     P06     PO5     PO4     PO3     PO2     PO1     PO0
 *
 * p1X I/O Data Bus     P17     P16     P15     P14     P13     P12     P11     P10
 *
 * 7-Bit 12C address plus R/W bit
 */

/*
 *  Relay Driver U4
 *
 *  P0_0    TR_RLY_1        0x0001
 *  PO_1    160M_BPF        0x0002
 *  PO_2    10M_BPF         0x0004
 *  PO_3    80M_BPF         0x0008
 *  PO_4    12M_BPF         0x0010
 *  PO_5    30M_BPF         0x0020
 *  PO_6    20M_BPF         0x0040
 *  PO_7    17M_BPF         0x0080
 *  
 *  P1_0    15M_BPF         0x0100
 *  P1_1    40M_BFP         0x0200
 *  P1_2    RLY_SPARE_4     0x0400
 *  P1_3    RLY_SPARE_3     0x0800
 *  P1_4    RLY_SPARE_2     0x1000
 *  P1_5    NOT IMPLEMENTED IN HW
 *  P1_6    NOT IMPLEMENTED IN HW
 *  P1_7    TR_RLY_2        0x8000
*/

/*
 *  Relay Driver U7
 *
 *  P0_0    RLY_SPARE_1     0x0001
 *  PO_1    RF_PREAMP       0x0002
 *  PO_2    EXT_TR_SEL      0x0004
 *  PO_3    EXT_TR_SW       0x0008
 *  PO_4    RF_ATT_2DB      0x0010
 *  PO_5    RF_ATT_4DB      0x0020
 *  PO_6    RF_ATT_8DB      0x0040
 *  PO_7    RF_ATT_16DB     0x0080
 *  
 *  P1_0    NOT IMPLEMENTED IN HW
 *  P1_1    NOT IMPLEMENTED IN HW
 *  P1_2    NOT IMPLEMENTED IN HW
 *  P1_3    NOT IMPLEMENTED IN HW
 *  P1_4    NOT IMPLEMENTED IN HW
 *  P1_5    NOT IMPLEMENTED IN HW
 *  P1_6    NOT IMPLEMENTED IN HW
 *  P1_7    NOT IMPLEMENTED IN HW
*/

#include <PTTControl.h>
#include <RelayDriver.h>

//Index to band arrays:
//0=160M, 1=80M, 2=40M, 3=30M, 4=20M, 5=17M, 6=15M, 7=12M, 8=10Ma, 9=10Mb, 10=10Mc

/* Relay Driver U4*/
const uint16_t TR_RLY_1[2]  = {0x0000, 0x0001};
const uint16_t RF_BPF[11]   = {0x0002, 0x0008, 0x0200, 0x0020, 0x0040, 0x0080, 0x0100, 0x0010, 0x0004, 0x0004, 0x0004};
const uint16_t SPR_RLY_2[2]  = {0x0000, 0x1000};
const uint16_t SPR_RLY_3[2]  = {0x0000, 0x0800};
const uint16_t SPR_RLY_4[2]  = {0x0000, 0x0400};
const uint16_t TR_RLY_2[2]  = {0x0000, 0x8000};

uint8_t saveTR_RLY_1 = 0;
uint8_t saveRF_BPF = 0;
uint8_t saveSPR_RLY_2 = 0;
uint8_t saveSPR_RLY_3 = 0;
uint8_t saveSPR_RLY_4 = 0;
uint8_t saveTR_RLY_2 = 0;

/* Relay Driver U7*/
const uint16_t SPR_RLY_1[2] = {0x0000, 0x0001};
const uint16_t PRE_AMP[2]   = {0x0000, 0x0002};
const uint16_t EXT_TR_SEL[2]= {0x0000, 0x0004};
const uint16_t EXT_TR_SW[2] = {0x0000, 0x0008};
//const uint16_t RF_ATTN[16]  = {0x0000, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x0080, 0x0090, 0x00A0, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0};
//adjusted for error in PCB
const uint16_t RF_ATTN[16] = {0x0000, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x0080, 0x0090, 0x00A0, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0};

uint8_t saveSPR_RLY_1 = 0;
uint8_t savePRE_AMP = 0;
uint8_t saveEXT_TR_SEL = 0;
uint8_t saveEXT_TR_SW = 0;
uint8_t saveRF_ATTN = 0;

/******************** I2C Addresses ********************/

uint8_t Relay_Driver_U4 = 0x4E >> 1;                        // shift datasheet I2C address 1 bit right as Wire shifts I2C addresses 1 bit left to add R/W bit
uint8_t Relay_Driver_U7 = 0x4C >> 1;


RelayDriver::RelayDriver(void)
{

}


/*void RelayDriver::begin(void)
{
  Wire1.begin();
}*/


void RelayDriver::txMode(void)                              // put bulk relay actions here for Tx mode
{
    TR_RLY1(XMT);
    TR_RLY2(XMT);
    EXTTRSW(XMT);
}


void RelayDriver::rxMode(void)                              // put bulk relay actions here for Rx mode
{
    TR_RLY1(RCV);
    TR_RLY2(RCV);
    EXTTRSW(RCV);
}


void RelayDriver::TR_RLY1(uint8_t trState)                  // Drives TR Relays on: RF BPF, Tx Driver Pwr
{
    uint16_t temp;

    saveTR_RLY_1 = trState;
    temp = TR_RLY_1[saveTR_RLY_1] | RF_BPF[saveRF_BPF] | TR_RLY_2[saveTR_RLY_2] | SPR_RLY_2[saveSPR_RLY_2] | SPR_RLY_3[saveSPR_RLY_3] | SPR_RLY_4[saveSPR_RLY_4];
    writeRelayDriver(Relay_Driver_U4, temp);
}


void RelayDriver::TR_RLY2(uint8_t trState)                  // Drives TR Relays on: Product Det. Audio Board, 1st & 2nd Mixer Board
{
    uint16_t temp;

    saveTR_RLY_2 = trState;
    temp = TR_RLY_1[saveTR_RLY_1] | RF_BPF[saveRF_BPF] | TR_RLY_2[saveTR_RLY_2] | SPR_RLY_2[saveSPR_RLY_2] | SPR_RLY_3[saveSPR_RLY_3] | SPR_RLY_4[saveSPR_RLY_4];
    writeRelayDriver(Relay_Driver_U4, temp);
}

void RelayDriver::BPF(uint8_t bpfBand)
{
    uint16_t temp;

    saveRF_BPF = bpfBand;
    temp = TR_RLY_1[saveTR_RLY_1] | RF_BPF[saveRF_BPF] | TR_RLY_2[saveTR_RLY_2] | SPR_RLY_2[saveSPR_RLY_2] | SPR_RLY_3[saveSPR_RLY_3] | SPR_RLY_4[saveSPR_RLY_4];
    writeRelayDriver(Relay_Driver_U4, temp);
}

void RelayDriver::RELAY_SPARE_2(uint8_t rlyState)
{
    uint16_t temp;

    saveSPR_RLY_2 = rlyState;
    temp = TR_RLY_1[saveTR_RLY_1] | RF_BPF[saveRF_BPF] | TR_RLY_2[saveTR_RLY_2] | SPR_RLY_2[saveSPR_RLY_2] | SPR_RLY_3[saveSPR_RLY_3] | SPR_RLY_4[saveSPR_RLY_4];
    writeRelayDriver(Relay_Driver_U4, temp);
}

void RelayDriver::RELAY_SPARE_3(uint8_t rlyState)
{
    uint16_t temp;

    saveSPR_RLY_3 = rlyState;
    temp = TR_RLY_1[saveTR_RLY_1] | RF_BPF[saveRF_BPF] | TR_RLY_2[saveTR_RLY_2] | SPR_RLY_2[saveSPR_RLY_2] | SPR_RLY_3[saveSPR_RLY_3] | SPR_RLY_4[saveSPR_RLY_4];
    writeRelayDriver(Relay_Driver_U4, temp);
}

void RelayDriver::RELAY_SPARE_4(uint8_t rlyState)
{
    uint16_t temp;

    saveSPR_RLY_4 = rlyState;
    temp = TR_RLY_1[saveTR_RLY_1] | RF_BPF[saveRF_BPF] | TR_RLY_2[saveTR_RLY_2] | SPR_RLY_2[saveSPR_RLY_2] | SPR_RLY_3[saveSPR_RLY_3] | SPR_RLY_4[saveSPR_RLY_4];
    writeRelayDriver(Relay_Driver_U4, temp);
}

void RelayDriver::RELAY_SPARE_1(uint8_t rlyState)
{
    uint16_t temp;

    saveSPR_RLY_1 = rlyState;
    temp = SPR_RLY_1[saveSPR_RLY_1] | PRE_AMP[savePRE_AMP] | EXT_TR_SEL[saveEXT_TR_SEL] | EXT_TR_SW[saveEXT_TR_SW] | RF_ATTN[saveRF_ATTN];
    writeRelayDriver(Relay_Driver_U7, temp);
}

void RelayDriver::PREAMP(uint8_t preAmp)
{
    uint16_t temp;

    savePRE_AMP = preAmp;
    temp = SPR_RLY_1[saveSPR_RLY_1] | PRE_AMP[savePRE_AMP] | EXT_TR_SEL[saveEXT_TR_SEL] | EXT_TR_SW[saveEXT_TR_SW] | RF_ATTN[saveRF_ATTN];
    writeRelayDriver(Relay_Driver_U7, temp);
}

void RelayDriver::EXTTRSEL(uint8_t extTrSel)
{
    uint16_t temp;

    saveEXT_TR_SEL = extTrSel;
    temp = SPR_RLY_1[saveSPR_RLY_1] | PRE_AMP[savePRE_AMP] | EXT_TR_SEL[saveEXT_TR_SEL] | EXT_TR_SW[saveEXT_TR_SW] | RF_ATTN[saveRF_ATTN];
    writeRelayDriver(Relay_Driver_U7, temp);
}

void RelayDriver::EXTTRSW(uint8_t extTrSw)
{
    uint16_t temp;

    saveEXT_TR_SW = extTrSw;
    temp = SPR_RLY_1[saveSPR_RLY_1] | PRE_AMP[savePRE_AMP] | EXT_TR_SEL[saveEXT_TR_SEL] | EXT_TR_SW[saveEXT_TR_SW] | RF_ATTN[saveRF_ATTN];
    writeRelayDriver(Relay_Driver_U7, temp);
}

void RelayDriver::RFATTN(uint8_t rfAttn)
{
    uint16_t temp;

    saveRF_ATTN = rfAttn;
    temp = SPR_RLY_1[saveSPR_RLY_1] | PRE_AMP[savePRE_AMP] | EXT_TR_SEL[saveEXT_TR_SEL] | EXT_TR_SW[saveEXT_TR_SW] | RF_ATTN[saveRF_ATTN];
    writeRelayDriver(Relay_Driver_U7, temp);
}


void RelayDriver::writeRelayDriver(uint8_t address, uint16_t relayState)
{
    Wire.beginTransmission(0x70);                           // I2C Mux select I2C_1 bus
    Wire.write(0x02);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(relayState & 0x00FF);
    Wire.write((relayState & 0xFF00) >> 8);
    Wire.endTransmission();

    Wire.beginTransmission(0x70);                           // I2C Mux select I2C_0 bus
    Wire.write(0x01);
    Wire.endTransmission();
}
