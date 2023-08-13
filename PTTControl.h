#ifndef PTTControl_h
#define PTTControl_h

#include <arduino.h>

enum{RCV, XMT};

class PTTControl
{
  public:
    PTTControl(void);
    void begin(void); 
    void Xmt(void);
    void Rcv(void);
  
  private:
};

#endif