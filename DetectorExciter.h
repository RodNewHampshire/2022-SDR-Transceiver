#ifndef DetectorExciter_h
#define DetectorExciter_h

class DetectorExciter
{
  public:
  	DetectorExciter(void);
  	void begin(void);
    void vfoUpdate(uint32_t *frequency, uint8_t *mode);
    void readGPSFreqCorrection(void);
    void enableRcvLO(void);
    void disableRcvLO(void);
    void enableXmtLO(void);
    void disableXmtLO(void);
  
  private:
    void setRcvLO(unsigned long Freq);
    void setXmtLO(unsigned long Freq);
    void si5351aSetFreq(int synth, unsigned long  freq);
    void si5351aStart(void);
    uint8_t Si5351_write(uint8_t addr, uint8_t data);
    uint8_t Si5351_read(uint8_t addr);
    void output_enable(uint8_t clk, uint8_t enable);
};

#endif