#ifndef FuncLCD_h
#define FuncLCD_h

#include <arduino.h>

#define ATTN_Cmnd       0x30		// function related commands
#define BAND_Cmnd       0x40
#define MODE_Cmnd       0x50
#define RATE_Cmnd       0x60
#define TR_Cmnd			    0x70
#define FUNC_Cmnd		    0x80
#define GAIN_Cmnd		    0x90
#define GPS_Cmnd        0xA0


enum                                                            // function menu items are displayed in this order
{
  START_FUNCTIONS,                                              // START_FUNCTIONS & END_FUNCTIONS are used to automatically define the start and end of the function list
  BAND_SELECT,
  RATE_SELECT,
  MODE_SELECT,
  GPS_SELECT,
  IQ_CORRECTION,
  END_FUNCTIONS
};


class FuncLCD
{
  public:
    FuncLCD(void);
    void begin(void); 
    void clear(void);
    void splash(void);
    void init(uint8_t _RATE, uint8_t _MODE, uint8_t _BAND);
    void FuncLCD::selectAF_Gain(void);
    void FuncLCD::selectRF_Gain(void);
    void FuncLCD::selectIF_Gain(void);
    void FuncLCD::selectLN_Gain(void);
    void update(uint8_t command);
    void dsplyAF_Gain(int8_t gain);
    void dsplyIF_Gain(int8_t gain);
    void dsplyLN_Gain(int8_t gain);
    void blankGain(void);
    void setI2CForLCD(void);
    
  
  private:

};

#endif