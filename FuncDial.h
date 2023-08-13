#ifndef FuncDial_h
#define FuncDial_h

#include <arduino.h>

class FuncDial
{
  public:
    FuncDial(void);
    void begin(void); 
    void select(void);
    void CW(void);
    void CCW(void);
    void incrementBand(void);
    void decrementBand(void);
    void incrementAGC(void);
    void decrementAGC(void);
    void incrementMode(void);
    void decrementMode(void);
    void incrementRate(void);
    void decrementRate(void);
    void incrementMx1Balance(void);
    void incrementMx1Clock(void);
    void incrementMx2Balance(void);
    void incrementMx2Clock(void);
    void decrementMx1Balance(void);
    void decrementMx1Clock(void);
    void decrementMx2Balance(void);
    void decrementMx2Clock(void);
  
  private:
};

#endif