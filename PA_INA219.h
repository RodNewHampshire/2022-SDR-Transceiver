#ifndef PA_INA219_h
#define PA_INA219_h


class PA_INA219
{
  public:
  	PA_INA219(void);
  	void begin(void);
    void getPAVoltAmps(float *shuntVoltage, float *busVoltage, float *loadAmpTicks);
  private:
  	
};


#endif