#ifndef EEProm_h
#define EEPRom_h

#include <arduino.h>

enum
{
  USB,
  LSB,
  CW
};


class EEProm
{
  public:
    EEProm(void);
    void begin(void);
    void saveSettings(void);
    void readSettings(uint16_t *currentVersion);
    uint8_t getAF_Gain(void);
    uint8_t getRF_Attn(void);
    uint8_t getIF_Gain(void);
    uint8_t getLN_Gain(void);
    void storeAF_Gain(uint8_t gain);
    void storeRF_Attn(uint8_t gain);
    void storeIF_Gain(uint8_t gain);
    void storeLN_Gain(uint8_t gain);
    uint32_t getVFOA_Freq(void);
    uint8_t getVFOA_Band(void);
    uint8_t getVFOA_Mode(void);
    uint8_t getAGC(void);
    uint8_t getRate(void);
    uint8_t getPreAmp(void);
    void storeVFOA_Freq(uint32_t frequency);
    void storeVFOA_Band(uint8_t band);
    void storeVFOA_Mode(uint8_t mode);
    uint32_t getBand_Freq(uint8_t band);
    void storeBand_Freq(uint32_t frequency, uint8_t band);
    uint8_t getBand_Mode(uint8_t band);
    void storeBand_Mode(uint8_t mode, uint8_t band);
    void storeAGC(uint8_t agc);
    void storeRate(uint8_t rate);
    void storePreAmp(uint8_t preamp);
    uint8_t getDSP_Gain(void);
    void storeDSP_Gain(uint8_t gain);
    uint8_t getDSP_BW(void);
    void storeDSP_BW(uint8_t bw);
    void storeSi5351Error(int16_t error);
    int16_t getSi5351Error(void);
    int16_t getIQ_RxAmpCorr(uint8_t band);
    int16_t getIQ_RxPhsCorr(uint8_t band);
    int16_t getIQ_TxAmpCorr(uint8_t band);
    int16_t getIQ_TxPhsCorr(uint8_t band);
    void storeIQ_RxAmpCorr(int16_t corr, uint8_t band);
    void storeIQ_RxPhsCorr(int16_t corr, uint8_t band);
    void storeIQ_TxAmpCorr(int16_t corr, uint8_t band);
    void storeIQ_TxPhsCorr(int16_t corr, uint8_t band);
    
  private:
};

#endif