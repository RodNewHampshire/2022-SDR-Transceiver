#ifndef AmpLink_h
#define AmpLink_h

#include <arduino.h>

class AmpLink
{
  public:
  	AmpLink(void);
    void begin(void);
    void available(void);
    void ampToTxMode(void);
    void ampToRxMode(void);
    void updateXmtBand(void);
  
  private:
    void executeAmp(void);
    void sendAmp(uint8_t data[], uint8_t num);
};


#endif