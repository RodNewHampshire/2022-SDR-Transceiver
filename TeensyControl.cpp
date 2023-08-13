/******************************************************************************
*
* 2022 HF SDR Transceiver by AD5GH
* (http://www.ad5gh.com)
*
* Copyright 2023 Rod Gatehouse AD5GH
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
 *  Relay Driver U2
 *
 *  P0_0    RX_TX        0x0001
 *  PO_1    USB_LSB      0x0002
 *  PO_2    TX_AF_GAIN   0x0004
 *  PO_3    RX_IF_GAIN   0x0008
 *  PO_4    RX_IQ_GAIN   0x0010
 *  PO_5    RX_IQ_PHS    0x0020
 *  PO_6    TX_IQ_GAIN   0x0040
 *  PO_7    TX_IQ_PHS    0x0080
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
#include <TeensyControl.h>


/* Relay Driver U2*/
const uint16_t RX_TX[2]         = {0x0001, 0x0000};
const uint16_t USB_LSB[2]       = {0x0002, 0x0000};
const uint16_t TX_AF_GAIN[2]    = {0x0004, 0x0000};
const uint16_t RX_IF_GAIN[2]    = {0x0008, 0x0000};
const uint16_t RX_IQ_AMP[2]     = {0x0010, 0x0000};
const uint16_t RX_IQ_PHS[2]     = {0x0020, 0x0000};
const uint16_t TX_IQ_AMP[2]     = {0x0040, 0x0000};
const uint16_t TX_IQ_PHS[2]     = {0x0080, 0x0000};

uint8_t saveRX_TX               = 0;
uint8_t saveUSB_LSB             = 0;
uint8_t saveTX_AF_GAIN          = 0;
uint8_t saveRX_IF_GAIN          = 0;
uint8_t saveRX_IQ_AMP           = 0;
uint8_t saveRX_IQ_PHS           = 0;
uint8_t saveTX_IQ_AMP           = 0;
uint8_t saveTX_IQ_PHS           = 0;

/******************** I2C Addresses ********************/

#define I2C_TEENSY              0x80                        // I2C address
uint8_t Relay_Driver_U2         = 0x4A >> 1;                // shift datasheet I2C address 1 bit right as Wire shifts I2C addresses 1 bit left to add R/W bit

enum
{
    DISABLE,
    ENABLE
};

typedef union
{
  int16_t dataToBeSent;
  uint8_t binary[sizeof(dataToBeSent)];
} binaryData;

binaryData I2CData;


TeensyControl::TeensyControl(void)
{

}


void TeensyControl::txMode(void)                            // TRANSMIT - RECEIVE CONTROL
{
    TR_Control(XMT);
}

void TeensyControl::rxMode(void)     
{
    TR_Control(RCV);
}

void TeensyControl::TR_Control(uint8_t trState)    
{
    uint16_t temp;

    saveRX_TX = trState;

    temp = RX_TX[saveRX_TX] | USB_LSB[saveUSB_LSB] | TX_AF_GAIN[saveTX_AF_GAIN] | RX_IF_GAIN[saveRX_IF_GAIN]
             | RX_IQ_AMP[saveRX_IQ_AMP] | RX_IQ_PHS[saveRX_IQ_PHS] | TX_IQ_AMP[saveTX_IQ_AMP] | TX_IQ_PHS[saveTX_IQ_PHS];
   
    writeTeensyControl(Relay_Driver_U2, temp);
}


void TeensyControl::setRX_IF_Gain(int16_t level)              // IF GAIN
{
    RX_IF_Gain(ENABLE);
    delay(5);
    sendDataToTeensy(level);
    delay(5);
    RX_IF_Gain(DISABLE);
}

void TeensyControl::RX_IF_Gain(uint8_t state)
{
    uint16_t temp;

    saveRX_IF_GAIN = state;

    temp = RX_TX[saveRX_TX] | USB_LSB[saveUSB_LSB] | TX_AF_GAIN[saveTX_AF_GAIN] | RX_IF_GAIN[saveRX_IF_GAIN]
             | RX_IQ_AMP[saveRX_IQ_AMP] | RX_IQ_PHS[saveRX_IQ_PHS] | TX_IQ_AMP[saveTX_IQ_AMP] | TX_IQ_PHS[saveTX_IQ_PHS];
   
    writeTeensyControl(Relay_Driver_U2, temp);
}


void TeensyControl::setRX_IQ_Amp(int16_t level)               // RX IQ AMPLITUDE CORRECTION
{
    RX_IQ_Amp(ENABLE);
    delay(5);
    sendDataToTeensy(level);
    delay(5);
    RX_IQ_Amp(DISABLE);
}

void TeensyControl::RX_IQ_Amp(uint8_t state)
{
    uint16_t temp;

    saveRX_IQ_AMP = state;

    temp = RX_TX[saveRX_TX] | USB_LSB[saveUSB_LSB] | TX_AF_GAIN[saveTX_AF_GAIN] | RX_IF_GAIN[saveRX_IF_GAIN]
             | RX_IQ_AMP[saveRX_IQ_AMP] | RX_IQ_PHS[saveRX_IQ_PHS] | TX_IQ_AMP[saveTX_IQ_AMP] | TX_IQ_PHS[saveTX_IQ_PHS];
   
    writeTeensyControl(Relay_Driver_U2, temp);
}


void TeensyControl::setRX_IQ_Phs(int16_t level)               // RX IQ PHASE CORRECTION
{
    RX_IQ_Phs(ENABLE);
    delay(5);
    sendDataToTeensy(level);
    delay(5);
    RX_IQ_Phs(DISABLE);
}

void TeensyControl::RX_IQ_Phs(uint8_t state)
{
    uint16_t temp;

    saveRX_IQ_PHS = state;

    temp = RX_TX[saveRX_TX] | USB_LSB[saveUSB_LSB] | TX_AF_GAIN[saveTX_AF_GAIN] | RX_IF_GAIN[saveRX_IF_GAIN]
             | RX_IQ_AMP[saveRX_IQ_AMP] | RX_IQ_PHS[saveRX_IQ_PHS] | TX_IQ_AMP[saveTX_IQ_AMP] | TX_IQ_PHS[saveTX_IQ_PHS];
   
    writeTeensyControl(Relay_Driver_U2, temp);
}


void TeensyControl::setTX_IQ_Amp(int16_t level)               // TX IQ AMPLITUDE CORRECTION
{
    TX_IQ_Amp(ENABLE);
    delay(5);
    sendDataToTeensy(level);
    delay(5);
    TX_IQ_Amp(DISABLE);
}

void TeensyControl::TX_IQ_Amp(uint8_t state)
{
    uint16_t temp;

    saveTX_IQ_AMP = state;

    temp = RX_TX[saveRX_TX] | USB_LSB[saveUSB_LSB] | TX_AF_GAIN[saveTX_AF_GAIN] | RX_IF_GAIN[saveRX_IF_GAIN]
             | RX_IQ_AMP[saveRX_IQ_AMP] | RX_IQ_PHS[saveRX_IQ_PHS] | TX_IQ_AMP[saveTX_IQ_AMP] | TX_IQ_PHS[saveTX_IQ_PHS];
   
    writeTeensyControl(Relay_Driver_U2, temp);
}


void TeensyControl::setTX_IQ_Phs(int16_t level)               // TX IQ PHASE CORRECTION
{
    TX_IQ_Phs(ENABLE);
    delay(5);
    sendDataToTeensy(level);
    delay(5);
    TX_IQ_Phs(DISABLE);
}

void TeensyControl::TX_IQ_Phs(uint8_t state)
{
    uint16_t temp;

    saveTX_IQ_PHS = state;

    temp = RX_TX[saveRX_TX] | USB_LSB[saveUSB_LSB] | TX_AF_GAIN[saveTX_AF_GAIN] | RX_IF_GAIN[saveRX_IF_GAIN]
             | RX_IQ_AMP[saveRX_IQ_AMP] | RX_IQ_PHS[saveRX_IQ_PHS] | TX_IQ_AMP[saveTX_IQ_AMP] | TX_IQ_PHS[saveTX_IQ_PHS];
   
    writeTeensyControl(Relay_Driver_U2, temp);
}


void TeensyControl::writeTeensyControl(uint8_t address, uint16_t teensyState)
{
    Wire.beginTransmission(address);
    Wire.write(teensyState & 0x00FF);
    Wire.write((teensyState & 0xFF00) >> 8);
    Wire.endTransmission();
}


void TeensyControl::sendDataToTeensy(int16_t data)
{
    I2CData.dataToBeSent = data;

    Wire.beginTransmission(0x70);                           // I2C Mux select I2C_1 bus
    Wire.write(0x02);
    Wire.endTransmission();

    Wire.beginTransmission(I2C_TEENSY);
    Wire.write(I2CData.binary, sizeof(I2CData.binary));
    Wire.endTransmission();

    Wire.beginTransmission(0x70);                           // I2C Mux select I2C_0 bus
    Wire.write(0x01);
    Wire.endTransmission();
}
