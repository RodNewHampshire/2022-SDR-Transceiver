#ifndef FreqControl_h
#define FreqControl_h

#include <arduino.h>


class FreqControl
{
  public:
    FreqControl(void);
    void begin(void);
    void initVFOA(void);
    void updateVFOA(int16_t *_freqUpdate);
    void setTuningRate(uint8_t rateUpdate);
    void bandChangeVFOA(uint8_t newBand);
    void catVFO_A_FreqUpdate(int32_t freq);
    void changeCurrentBandMode(uint8_t mode);
    
  
  private:
    void Set_Band_Limits(long *Band_Limit_Array, char Band_Number);
    uint8_t freqToBandNumber(int32_t freq);

};

#endif