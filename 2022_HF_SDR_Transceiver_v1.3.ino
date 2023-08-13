/******************************************************************************************************************************

  2022 HF SDR Transceiver
  High Performance HF Band Amateur Radio Transceiver
  for Arduino Mega
 
  by Rod Gatehouse, AD5GH

  Distributed under the terms of the MIT License:
  http://www.opensource.org/licenses/mit-license

  VERSION 1.3
  April 29, 2023

******************************************************************************************************************************/

#include <ft857d.h>
#include <AmpLink.h>
#include <EEProm.h>
#include <DetectorExciter.h>
#include <FreqControl.h>
#include <FuncDial.h>
#include <FuncLCD.h>
#include <GainDial.h>
#include <MeterLCD.h>
#include <PTTControl.h>
#include <RelayDriver.h>
#include <Rotary.h>
#include <Adafruit_SleepyDog.h>

ft857d          cat_Intfc = ft857d();
AmpLink         ampLink;
EEProm          eeprom;
DetectorExciter detectorExciter;
FreqControl     freqControl;
FuncLCD         funcLCD;
FuncDial        funcDial;
GainDial        gainDial;
MeterLCD        meterLCD;
PTTControl      pttControl;
RelayDriver     relay_Driver;


uint16_t currentVersion = 10201;                            // current software version X.YY.ZZ, i.e., 1.00.00

//#define DEBUG                                             // remove comment (//) to denable Arduino serial port for debug messages


/********************Frequency, Band, & Mode Variables************************************************************************/

int16_t freqUpdate = 0;                                     // current frequency increment update
int16_t lastFreqUpdate = 0;                                 // last frequency increment update
uint16_t dummyFreqUpdate = 0;                               // used to update VFOA when necessary without change of displayed frequency

int8_t gpsFreqErrorUpdate = 30;                             // counter for periodic GPS frequency correction, preset to force GPS update on start up

extern uint16_t tuningRate;                                 // 10, 100, 250, or 1000Hz, selected in FuncDial.cpp used in Tune Encoder Interrupt Service Routine (ISR)

uint8_t tuneOld = 0;                                        // used by Tune Encoder ISR to detect direction of rotation of tuning dial


/********************I2C_0 and I2C_1 Allocation*******************************************************************************

Teensy 4.1, Xmt & Rcv Audio Board v1
 - Teensy 4.1                                   I2C_1
 - PCF8575CDWR for Teensy control               I2C_0
 - DS1807 for volume control                    I2C_0


Arduino & I2C Relay Control Board
 - PCF8575CDWR for relay control                I2C_1
 - LCDs                                         I2C_0

QSD / QSE / LO Board
 - S15351                                       I2C_0
 - Metro Mini                                   I2C_0

**********************IO Port Definitions ***********************************************************************************/

#define PTT             27                                  // PTT line from CAT inteface

#define TUNE_ENCDR_A    2                                   // Tuning Rotary Encoder, refer to Front Panel Board schematic sheet 1
#define TUNE_ENCDR_B    3
#define TUNE_ENCDR_SEL  4

#define GAIN_ENCDR_A    5                                   // Gain Rotary Encoder, refer to Front Panel Board schematic sheet 1
#define GAIN_ENCDR_B    6
#define GAIN_ENCDR_SEL  7

#define FUNC_ENCDR_A    8                                   // Function Rotary Encoder, refer to Front Panel Board schematic sheet 1
#define FUNC_ENCDR_B    9
#define FUNC_ENCDR_SEL  10

#define ON_AIR_LED      11                                  // On Air LED indicator, refer to Front Panel Board schematic sheet 1

#define PA_PWR_ON       23                                  // Power Amplifier Vdd enable, refer to Backplane schematic Sheet 2
#define PA_ENB          22                                  // Power Amplifier bias enable, refer to Backplane schematic Sheet 2
#define PWR_SENS        24                                  // senses the state of the front panel Power Switch, refer to Backplane schematic Sheet 2
#define PWR_HOLD        25                                  // maintains power on after Power Switch turn off to perform shutdown functions prior to removing power, refer to Backplane Schematic sheet 2

#define STATUS_LED      26                                  // LED on Arduino & I2C Control Board to indicate program running

Rotary gainRotary = Rotary(GAIN_ENCDR_A, GAIN_ENCDR_B);     // Rotary callback routines for Gain and Function encoders
Rotary funcRotary = Rotary(FUNC_ENCDR_A, FUNC_ENCDR_B);

extern uint8_t  IQ_COR_FLAG;


/*********************Loop State Machine**************************************************************************************/

uint32_t HEART_BEAT = 4000;                                 // heart beat period
uint32_t heartBeat = 0;                                     // heart beat counter

boolean pttStatus = RCV;                                    // initialize PTT status to receive, lastPPTStatus to transmit
boolean lastPTTStatus = XMT;

uint32_t  PTT_TIMEOUT = 180000;                             // set transmit timeout to 3 minutes (360K milliseconds)
uint32_t  pttTimer = 0;
uint32_t  pttStartTime = 0;

uint8_t   tuneEncoderFlag = 0;

enum {
  FALSE,
  TRUE
};


/*********************setup() ************************************************************************************************/

void setup() 
{
#ifdef DEBUG
  Serial.begin(38400);
  Serial.println("Main Program Debug Enabled");
#endif

  pinMode(TUNE_ENCDR_A, INPUT);                             // note, external pullup resistors on Front Panel Board
  pinMode(TUNE_ENCDR_B, INPUT);
  pinMode(TUNE_ENCDR_SEL, INPUT);  

  attachInterrupt(digitalPinToInterrupt(TUNE_ENCDR_A), tuneEncdrISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(TUNE_ENCDR_B), tuneEncdrISR, CHANGE);

  pinMode(GAIN_ENCDR_A, INPUT);                             // note, external pullup resistors on Front Panel Board
  pinMode(GAIN_ENCDR_B, INPUT);
  pinMode(GAIN_ENCDR_SEL, INPUT);

  pinMode(FUNC_ENCDR_A, INPUT);                             // note, external pullup resistors on Front Panel Board
  pinMode(FUNC_ENCDR_B, INPUT);
  pinMode(FUNC_ENCDR_SEL, INPUT);

  pinMode(ON_AIR_LED, OUTPUT);
  digitalWrite(ON_AIR_LED, HIGH);

  pinMode(PA_ENB, OUTPUT);
  digitalWrite(PA_ENB, LOW);

  pinMode(PA_PWR_ON, OUTPUT);
  digitalWrite(PA_PWR_ON, HIGH);

  pinMode(PWR_SENS, INPUT);

  pinMode(PWR_HOLD, OUTPUT);
  digitalWrite(PWR_HOLD, HIGH);

  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);

  pinMode(PTT, INPUT);

  if (digitalRead(TUNE_ENCDR_A)) bitSet(tuneOld, 4);        // initialize tuneOld
  if (digitalRead(TUNE_ENCDR_B)) bitSet(tuneOld, 5);

  meterLCD.begin();                                         // this has to be called twice, don't know why?
  funcLCD.begin();                                          // initalize Function (right) LCD
  meterLCD.begin();                                         // initialize Meter (left) LCD

  funcLCD.splash();                                         // dislay spalsh screen for both LCDs. Spalsh screen content can be edited in MeterLCD.cpp and FuncLCD.cpp
  meterLCD.splash();
  delay(2000);                                              // splash screens dislayed for ~2 seconds

  funcLCD.clear();                                          // clear LCDs ready for operational displays
  meterLCD.clear();

  Watchdog.enable(8000);                                    // enable Arduino Watchdog timer with 8 second timeout

  eeprom.readSettings(&currentVersion);                     // recall EEPROM settings or write and recall EEPROM defaults if new Arduino or software version
  meterLCD.clear();                                         // clear Meter LCD for operational display

  funcDial.begin();                                         // get from EEPROM and set initial band, mode, AGC, & tuning rate settings
  gainDial.begin();                                         // get from EEPROM and set initial Spkr & Line Out audio gain, IF gain, RF attn, and DSP IQ Correction 
  
  detectorExciter.begin();                                  // start Si5351 and set 2.5-MHz reference frequency on CLK2
  
  freqControl.begin();                                      // copy S Meter calibration table from EEPROM (***S Meter function not currently implemented***)
  freqControl.initVFOA();                                   // initialize VFOA frequency, band, and mode settings; set band limits; get band specific S Meter calibration
  
  //ampLink.begin();                                          // start serial3 for amplifier link

  pttControl.Rcv();                                         // configure transceiver for receive operation

  cat_Intfc.addCATFSet(catSetFreq);                         // start CAT interface
  cat_Intfc.begin();
}

/*********************loop()**********************************************************************************************/

void loop() 
{
  if (digitalRead(PTT) == LOW) pttStatus = RCV;             // check PTT status
  if (digitalRead(PTT) == HIGH) pttStatus = XMT;

  //ampLink.available();                                      // check if amplifier sending requests and execute
  cat_Intfc.check();                                        // check if CAT interface sending requests and execute


  if (pttStatus == RCV)                                     // RECEIVE LOOP************************************************
  {
    if (lastPTTStatus != RCV)                               // things to do if status changes from XMT to RCV
    {
      digitalWrite(PA_ENB, LOW);                            // disable PA bias
      digitalWrite(ON_AIR_LED, HIGH);
      pttControl.Rcv();                                     // configure transceiver for receive operation
      lastPTTStatus = RCV;
      HEART_BEAT = 4000;
      heartBeat = 0;
    }

    if (freqUpdate != 0)                                    // update VFO frequency if main tuning dial turned
    {
      freqControl.updateVFOA(&freqUpdate);
      freqUpdate = 0;
    }

    if (!digitalRead(FUNC_ENCDR_SEL))                       // check if Function Encoder pressed
    {
      funcDial.select();                                    // change to new function
      delay(250);
    }

    if (!digitalRead(GAIN_ENCDR_SEL))                       // check if Gain Encoder pressed
    {
      gainDial.select();                                    // change to new Gain function
      delay(250);
    }

    if (!digitalRead(TUNE_ENCDR_SEL))                       // check if Tune Encoder pressed
    {
      tuneEncoder();
      delay(250);
    }

    if (gainEncdr() != 0)                                   // check if gain dial turned
    {
    }

    if (funcEncdr() != 0)                                   // check if function dial turned
    {
    }

    if (++heartBeat == HEART_BEAT)                          // RCV HEART BEAT FUNCTIONS********************************************
    {
      heartBeat = 0;
      digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));   // flash status LED on Arduino and I2C Relay Control Board
      meterLCD.indicateVFOA(RCV);                           // flash "*" for RCV on first character of VFOA display
    
      //ifBoard.updateSMeter();                             // update S Meter display
      if (gpsFreqErrorUpdate == 30)                         // update GPS freq error correction every ~30 seconds when in RCV mode
      {
        gpsFreqErrorUpdate = 0;
        //localOscillator.readGPSFreqCorrection();                              // read Mixer Board Si5351 Xtal GPS frequency error
        //meterLCD.updateGPSErrorDisplay();                                     // update error display
        //freqControl.updateVFOA(&dummyFreqUpdate);
        //localOscillator.adjust();                                             // apply error correction to VFO and LO frequency
      } else gpsFreqErrorUpdate++;
    }
  }


  if (pttStatus == XMT)                                     // TRANSMIT LOOP***********************************************
  {                                                         // NOTE: all tuning, gain, and other functions are disabled during XMT
    if (lastPTTStatus != XMT)                               // things to do if status changed from RCV to XMT
    {
      pttControl.Xmt();                                     // configure transceiver for transmit operation
      digitalWrite(PA_ENB, HIGH);                           // enable PA bias
      digitalWrite(ON_AIR_LED, LOW);
      pttStartTime = millis();
      lastPTTStatus = XMT;
      HEART_BEAT = 2000;
      heartBeat = 0;
    }

    if(IQ_COR_FLAG)                                         // check if IQ correction function selected
    {
      if (funcEncdr() != 0)                                 // check if function dial turned for IQ correction
      {
      }
      
      if (gainEncdr() != 0)                                 // check if gain dial turned for IQ correction
      {
      }
    }

    if (++heartBeat == HEART_BEAT)                          // XMT HEART BEAT FUNCTIONS********************************************
    {
      heartBeat = 0;
      digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));   // flash status LED on Arduino and I2C Relay Control Board
      meterLCD.indicateVFOA(XMT);                           // flash "!" for XMT on first character of VFOA display

      if(IQ_COR_FLAG == 0) meterLCD.updatePADisplay();
    }      
   
    pttTimer = millis() - pttStartTime;                     // check for PTT timeout
    if (pttTimer > PTT_TIMEOUT)                             // if timeout period exceeded, ignore PTT line, lock transceiver in Receive mode
    {
      digitalWrite(PA_ENB, LOW);                            // disable PA bias
      pttControl.Rcv();                                     // configure transceiver for receive operation
      digitalWrite(ON_AIR_LED, HIGH);                       // turn off On Air LED
      digitalWrite(PWR_HOLD, LOW);                          // release PWR_HOLD to allow transceiver to be powered down
      meterLCD.clear();
      while (1)                                             // infinite loop, requires power cycling of transceiver to clear this state
      {
        delay(125);
        meterLCD.pttTimeout(TRUE);                          // flash ITMEOUT ERROR on Meter LCD
        delay(125);                                         // receiver continues to function when in this state, but no controls
        meterLCD.pttTimeout(FALSE);
      }
    }
  }

  if (!digitalRead(PWR_SENS))                               // POWER DOWN FUNCTIONS********************************************
  {
    eeprom.saveSettings();                                  // save settings
    
    for(uint8_t n=0; n<20; n++)
    {
      digitalWrite(ON_AIR_LED, LOW);                        // flash On-air LED 10 times
      delay(50); 
      digitalWrite(ON_AIR_LED, HIGH);
      delay(50);   
    }
    
    digitalWrite(PWR_HOLD, LOW);                            // release PWR_HOLD to power down transceiver
  }

  Watchdog.reset();                                         // reset Arduino Watchdog timer each time through Loop  


}
/*********************Gain Encoder Routine************************************************************************************/

uint8_t gainEncdr(void) 
{
  unsigned char result = gainRotary.process();

  if (result == DIR_CW) {
    gainDial.CW();
    return 1;
  }

  else if (result == DIR_CCW) {
    gainDial.CCW();
    return 1;
  }
  return 0;
}


/*********************Function Encoder Routine********************************************************************************/

uint8_t funcEncdr(void) 
{
  unsigned char result = funcRotary.process();

  if (result == DIR_CW) {
    funcDial.CCW();
    return 1;
  }

  else if (result == DIR_CCW) {
    funcDial.CW();
    return 1;
  }
  return 0;
}


/*********************Interrupt Routines**************************************************************************************/

void tuneEncdrISR(void) 
{
  uint8_t tuneNew = 0;
  uint8_t tuneDiff;

  if (freqUpdate != 0) return;

  if (digitalRead(TUNE_ENCDR_A)) bitSet(tuneNew, 4);
  if (digitalRead(TUNE_ENCDR_B)) bitSet(tuneNew, 5);

  tuneDiff = tuneNew ^ tuneOld;

  switch (tuneDiff) {
    case 0x20:
      if (tuneNew == 0x00 || tuneNew == 0x30) freqUpdate += tuningRate;
      else freqUpdate -= tuningRate;
      break;

    case 0x10:
      if (tuneNew == 0x00 || tuneNew == 0x30) freqUpdate -= tuningRate;
      else freqUpdate += tuningRate;
      break;
  }
  tuneOld = tuneNew;

  if (lastFreqUpdate != freqUpdate) {
    lastFreqUpdate = freqUpdate;
    freqUpdate = 0;
  }
}


/*********************CAT Routines*******************************************************************************************/

void catSetFreq(long freq)                                  // Set Frequency, only CAT function currently enabled
{
  freqControl.catVFO_A_FreqUpdate(freq);
}


/*********************DEBUG Routines*****************************************************************************************/

void tuneEncoder(void)
{
  if(tuneEncoderFlag == 0) 
  {
    tuneEncoderFlag = 1;
    detectorExciter.disableRcvLO();
    digitalWrite(ON_AIR_LED, LOW);
  }

  else if(tuneEncoderFlag == 1) 
  {
    tuneEncoderFlag = 0;
    detectorExciter.enableRcvLO();
    digitalWrite(ON_AIR_LED, HIGH);
  }
}


/*********************END OF PROGRAM*****************************************************************************************/