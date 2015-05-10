#ifndef RASPI_GPIO_H
#define RASPI_GPIO_H

#include "wiringPi.h"

#define NPIN 2
typedef void (callback)(void);

class RaspiGPIO
{
public:
	RaspiGPIO();
	void setCallback(int pin, callback* cb);

};

#endif

