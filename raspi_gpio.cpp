#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "wiringPi.h"
#include "raspi_gpio.h"

RaspiGPIO::RaspiGPIO()
{
	wiringPiSetup () ;
	for (int pin = 0; pin < NPIN; ++pin)
		pullUpDnControl(pin, PUD_UP);
}

void RaspiGPIO::setCallback(int pin, callback* cb){
    	wiringPiISR (pin, INT_EDGE_FALLING, cb) ;
}

#if 0

void myInterrupt0 (void) { ++globalCounter [0] ; }
void myInterrupt1 (void) { ++globalCounter [1] ; }
static volatile int globalCounter [NPIN] ;

int main (void)
{
  int gotOne, pin ;
  int myCounter [NPIN] ;

  for (pin = 0 ; pin < NPIN ; ++pin) 
    globalCounter [pin] = myCounter [pin] = 0 ;

  wiringPiSetup () ;

  for (pin = 0; pin < NPIN; ++pin)
    pullUpDnControl(pin, PUD_UP);

  callback* callbacks[NPIN];
  callbacks[0] = &myInterrupt0;
  callbacks[1] = &myInterrupt1;
  
  for (pin = 0; pin < NPIN; ++pin)
    wiringPiISR (pin, INT_EDGE_FALLING, callbacks[pin]) ;

  for (;;)
  {
    gotOne = 0 ;
    printf ("Waiting ... ") ; fflush (stdout) ;

    for (;;)
    {
      for (pin = 0 ; pin < NPIN ; ++pin)
      {
	if (globalCounter [pin] != myCounter [pin])
	{
	  printf (" Int on pin %d: Counter: %5d\n", pin, globalCounter [pin]) ;
	  myCounter [pin] = globalCounter [pin] ;
	  ++gotOne ;
	}
      }
      if (gotOne != 0)
	break ;
    }
  }

  return 0 ;
}
#endif

