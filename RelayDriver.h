#ifndef RelayDriver_h
#define RelayDriver_h

#include <arduino.h>
#include <Wire.h>


class RelayDriver
{
  public:
    RelayDriver(void);
    void begin(void); 
    void BPF(uint8_t bpfBand);
    void LPF(uint8_t lpfBand);
    void RELAY_SPARE_1(uint8_t rlySpare);
    void RELAY_SPARE_2(uint8_t rlyState);
    void RELAY_SPARE_3(uint8_t rlyState);
    void RELAY_SPARE_4(uint8_t rlyState);
    void PREAMP(uint8_t preAmp);
    void EXTTRSEL(uint8_t extTrSel);
    void EXTTRSW(uint8_t extTrSw);
    void RFATTN(uint8_t rfAttn);
    void txMode(void);
    void rxMode(void);
  
  private:
    void writeRelayDriver(uint8_t address, uint16_t relayState);
    void TR_RLY1(uint8_t trState);
    void TR_RLY2(uint8_t trState);
};

#endif