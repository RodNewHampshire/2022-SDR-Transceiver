#ifndef GainDial_h
#define GainDial_h

#include <arduino.h>


class GainDial
{
  public:
    GainDial(void);
    void begin(void); 
    void select(void);
    void CW(void);
    void CCW(void);
    void setAFGain(uint8_t level);
    void updateDSP_IQ_Correction(uint8_t band);
  
  private:
  	void incrementAFGain(void);
  	void decrementAFGain(void);
  	void incrementRFAttn(void);
  	void decrementRFAttn(void);
  	void incrementIFGain(void);
  	void decrementIFGain(void);
  	void incrementLineGain(void);
  	void decrementLineGain(void);
  	void setLineGain(uint8_t level);
  	void disableZeroCrossingDetection(void);
    void setRFAttn(uint8_t level);
    void setRX_IF_Gain(float level);
    void incrementIQ_Correction(void);
    void decrementIQ_Correction(void);
    void setRX_IQ_Amp(float level);
    void setRX_IQ_Phs(float level);
    void setTX_IQ_Amp(float level);
    void setTX_IQ_Phs(float level);
    void sendDataToTeensy(float data);
};

#endif