#ifndef TeensyControl_h
#define TeensyControl_h

#include <arduino.h>
#include <Wire.h>


class TeensyControl
{
  public:
    TeensyControl(void);
    void txMode(void);
    void rxMode(void);
    void TR_Control(uint8_t trState);
    void setRX_IF_Gain(int16_t level);
    void setRX_IQ_Amp(int16_t level);
    void setRX_IQ_Phs(int16_t level);
    void setTX_IQ_Amp(int16_t level);
    void setTX_IQ_Phs(int16_t level);

  private:
    void RX_IF_Gain(uint8_t state);
    void RX_IQ_Amp(uint8_t state);
    void RX_IQ_Phs(uint8_t state);
    void TX_IQ_Amp(uint8_t state);
    void TX_IQ_Phs(uint8_t state);
    void writeTeensyControl(uint8_t address, uint16_t teensyState);
    void sendDataToTeensy(int16_t data);
};

#endif