#ifndef MeterLCD_h
#define MeterLCD_h

#include <arduino.h>

class MeterLCD
{
  public:
    MeterLCD(void);
    void begin(void); 
    void clear(void);
    void clearDisplayRow_1(void);
    void splash(void);
    void functionDisplay(uint8_t functionNum);
    void displayValue(uint8_t functionNum);
    void updateVFOADisplay(uint32_t *frequency, uint8_t *mode);
    void updateGPSErrorDisplay(void);
    void updateDSPBandwidthDisplay(void);
    void updateDSPGainDisplay(void);
    void updateIQ_CorrectionDisplay(uint8_t select);
    void updateIQ_AmpCorrectionDisplay(float Amp);
    void updateIQ_PhsCorrectionDisplay(float Phs);
    void indicateVFOA(uint8_t status);
    void eepromStart(void);
    void eepromNoDataFound(void);
    void eepromInitializing(void);
    void eepromReadingData(void);
    void startSMeterCalibration(void);
    void stopSMeterCalibration(void);
    void calibrateSMeter(void);
    void updateSMeterDsply(uint8_t S_Level);
    void initializeRxDisplay(void);
    void initializeTxDisplay(void);
    void pttTimeout(uint8_t display);
    void updatePADisplay(void);
    void updateAmpBars(uint8_t level, uint8_t display);
  
  private:
  	String intToASCII(uint16_t value);
  	uint8_t ToASCII(uint8_t value);
};

#endif