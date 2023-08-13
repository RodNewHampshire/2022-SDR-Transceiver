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
#include <EEProm.h>

EEProm _EEPROM;

char currentBand[5] = "BD0 ;";
char ampToTx[5] = "MDTR;";
char ampToRx[5] = "MDRX;";

char ampCmnd[20];
uint8_t ampCntr = 0;
uint8_t ampChar = 0;


AmpLink::AmpLink(void)
{

}


void AmpLink::begin(void)
{
	Serial3.begin(115200);																		// amplifier serial link, pin 19 RX, pin 18 TX
}


void AmpLink::available(void)
{
	if(Serial3.available())
	{
		ampChar = Serial3.read();

		if(ampChar == ';') 
		{
			ampCmnd[ampCntr] = ampChar;
			executeAmp();
			ampCntr = 0;
		}

		else
		{
			ampCmnd[ampCntr] = ampChar;
			ampCntr++;
			if(ampCntr == 21) ampCntr = 0;
		}
	}
}


void AmpLink::ampToTxMode(void)															// Amplifier to On Air Mode
{
	sendAmp(ampToTx, 5);				
}


void AmpLink::ampToRxMode(void)
{
	sendAmp(ampToRx, 5);																			//Amplifier to Receive Mode
}


void AmpLink::executeAmp(void)
{
	switch(ampCmnd[0])
	{
		case 'B':
			switch(ampCmnd[1])
			{
				case 'D':																						//amplifier request for frequency band
				updateXmtBand();																		//send band update to amplifier
				break;
			}
			break;
	}
}


void AmpLink::updateXmtBand(void)														//send current band to amplifier
{
	currentBand[3] = _EEPROM.getVFOA_Band();
	sendAmp(currentBand, 5);
}


void AmpLink::sendAmp(uint8_t data[], uint8_t num)
{
	for (uint8_t i=0; i<num; i++) 
		{
			Serial3.write(data[i]);
		}
}
