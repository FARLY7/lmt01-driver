# LMT01 Temperature Sensor API
A simple driver for the Texas Instrument's LMT01 temperature sensor.

## Introduction
This package contains the LMT01 sensor driver.
The driver includes lmt01.c and lmt01.h files.

## Integration details
* Integrate lmt01.h and lmt01.c files into the project
* Include the lmt01.h file in your code like below.

``` c
#include "lmt_01.h"
```

## File information
* lmt01.h : This header file contains the declarations of the driver APIs.
* lmt01.c : This source file contains the definitions of the driver APIs.

## Supported interfaces
* Timer (with clock sourced mapped to GPIO)

## User Guide

### Prerequisite
* The LMT01 has a unique interface. A series of pulses outputted by the device are used to represent the temperature value. This number of pulses varies with the temperature value.
* This driver requires a timer to be preconfigured with the ability to increment it's counter with every pulse received over GPIO.

### Initialising the device
To intialise the device, the user must create a device structure. The user can do this by creating an instance of the structure lmt01_dev. The user must then fill in the various parameters as shown below.

``` c
lmt01_status_t rslt = LMT_OK;
lmt01_dev lmt;

lmt.timer = <timer context>;
lmt.start_timer = usr_start_timer;
lmt.stop_timer = usr_stop_timer;
lmt.set_timer_cnt = usr_set_timer_cnt;
lmt.get_timer_cnt = usr_get_timer_cnt;
lmt.delay_ms = usr_delay_ms;

rslt = lmt_init(&lmt);

float temp = -1;

/* Obtain reading from LMT sensor */
rslt = lmt_get_temperature(&lmt, &temp, CONV_TYPE_LUT);
````

### Templates for function pointers
``` c
void usr_start_timer(void *timer)
{
}

void usr_stop_timer(void *timer)
{
}

void usr_set_timer_cnt(void *timer, uint32_t *val)
{
}

void usr_get_timer_cnt(void *timer, uint32_t *val)
{
}

void usr_delay_ms(uint32_t period_ms)
{
}
```
