/*
 * MIT License
 * 
 * Copyright (c) 2019 Sean Farrelly
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * File        lmt01.c
 * Created by  Sean Farrelly
 * Version     1.0
 * 
 */

/*! @file lmt01.c
 * @brief Driver for LMT01 temperature sensor.
 */
#include "lmt01.h"
#include <math.h>


#define LEN(arr) ((int)(sizeof(arr) / sizeof(arr)[0])) /* Return length of array */

/*!
 * @brief This internal API is used to validate the device pointer for
 * null conditions.
 *
 * @param[in] dev : Structure instance of lmt01_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
static lmt_status_t null_ptr_check(const lmt01_dev *dev);

/*!
 * @brief This internal API is used to count the number of pulses
 * received in a given period (ms).
 *
 * @param[in] dev : Structure instance of lmt01_dev.
 * @param[in] dev : Period in which to count pulses (ms).
 *
 * @return Number of pulses received from lmt01.
 * @retval pulses
 */
static uint32_t count_pulses_ms(const lmt01_dev *dev, uint32_t period);

/*!
 * @brief Map a value from one scale to another. Used for lookup-table.
 */
static double map(double val, double in_min, double in_max, double out_min, double out_max);

/*
 * @brief Look-up table used for converting no of pulses to *C
 */
static const int16_t lut[21][2] = {
    {-50, 26},
    {-40, 181},
    {-30, 338},
    {-20, 494},
    {-10, 651},
    {0, 808},
    {10, 966},
    {20, 1125},
    {30, 1284},
    {40, 1443},
    {50, 1602},
    {60, 1762},
    {70, 1923},
    {80, 2084},
    {90, 2245},
    {100, 2407},
    {110, 2569},
    {120, 2731},
    {130, 2893},
    {140, 3057},
    {150, 3218}
    };


/**
  * @brief  Initialise lmt01 device and check if alive.
  * 
  * @param[in] dev : LMT01 device structure
  * 
  * @return result of API execution status
  * @retval lmt_status_t
  */
lmt_status_t lmt_init(const lmt01_dev *dev)
{
    /* Check for null pointer in the device structure */
    if(null_ptr_check(dev) != LMT_OK)
        return LMT_E_NULL_PTR;

    /* Count number of pulses received over 60ms period */
    uint32_t pulses = count_pulses_ms(dev, 60);

    if(pulses == 0)
        return LMT_E_DEV_NOT_FOUND;

    return LMT_OK;
}

/**
  * @brief Obtain one temperature reading from device by counting pulses.
  * 
  * @param[in] dev : LMT01 device structure.
  * @param[out] pulses : Pointer to variable to store pulse count.
  * 
  * @return result of API execution status
  * @retval lmt_status_t
  */
lmt_status_t lmt_get_pulse_count(const lmt01_dev *dev, uint32_t *pulses)
{
    /* Check for null pointer in the device structure */
    if(null_ptr_check(dev) != LMT_OK)
        return LMT_E_NULL_PTR;

    /* If pulses are received over next 10ms period, we are in
        the middle of an output. Wait until output has finished. */
    while(count_pulses_ms(dev, 10) != 0);

    /* Expect to receive a reading over the next ~104ms,
       begin counting pulses. */
    uint32_t pulse_count = count_pulses_ms(dev, 104);

    /* Error: did not receive any pulses, device unresponsive? */
    if(pulse_count == 0)
    {
        return LMT_E_DEV_NOT_FOUND;
    }

    *pulses = pulse_count;

    return LMT_OK;
}

/**
  * @brief  Obtains a pulse count reading from the LMT device
  *         and converts this value to temperature equivalent
  *         according to the type parameter.
  * 
  * @param[in] lmt : LMT Handle
  * @param[out] temp : Temperature reading
  * @param[in] type : Conversion type (EQU, LUT)
  * 
  * @return Result of API execution status
  * @retval lmt_status_t
  */
lmt_status_t lmt_get_temperature(const lmt01_dev *dev, float *temp, lmt_conv_t type)
{
    lmt_status_t rslt;
    uint32_t pulses;

    /* Obtain temperature reading from device in pulse form */
    rslt = lmt_get_pulse_count(dev, &pulses);

    if(rslt != LMT_OK)
        return rslt;

    /* Convert reading to temperature */
    *temp = lmt_pulses_to_temperature(pulses, type);

    return LMT_OK;
}

/**
  * @brief  Converts a pulse count to temperature equivalent
  *         according to the type parameter.
  * 
  * @param[in] pulses : Number of pulses
  * @param[in] type   : Conversion type (EQU, LUT)
  * 
  * @return Result of pulses -> temperature conversion
  * @retval temp
  */
float lmt_pulses_to_temperature(uint32_t pulses, lmt_conv_t type)
{
    if (pulses == 0)
        return -1;

    float temp = 0.0;

    /* Conversion method: Equation */
    if (type == CONV_TYPE_EQU)
    {
        temp = ((pulses / 4096.0) * 256.0) - 50.0;
    }
    /* Conversion method: Lookup table */
    else if (type == CONV_TYPE_LUT)
    {
        size_t i;
        for (i = 0; i < LEN(lut); i++)
        {
            if (pulses >= lut[i][1] && pulses <= lut[i + 1][1])
                break;
        }
        
        /* Map number of pulses to degress celcius using lookup table */
        temp = map(pulses, lut[i][1], lut[i + 1][1], lut[i][0], lut[i + 1][0]);
    }

    return temp;
}

/*!
 * @brief This internal API is used to count the number of pulses
 * received in a given period (ms).
 */
static uint32_t count_pulses_ms(const lmt01_dev *dev, uint32_t period)
{
    /* Stop counting pulses */
    dev->stop_timer(dev->timer);

    /* Reset the timer pulse count */
    dev->set_timer_cnt(dev->timer, 0);

    /* Start counting pulses */
    dev->start_timer(dev->timer);

    /* Wait until period elapses */
    dev->delay_ms(period);

    /* Stop counting pulses */
    dev->stop_timer(dev->timer);

    /* Return number of pulses counted */
    return dev->get_timer_cnt(dev->timer);
}

/*!
 * @brief This internal API is used to validate the device structure pointer for
 * null conditions.
 */
static lmt_status_t null_ptr_check(const lmt01_dev *dev)
{
    lmt_status_t rslt;

    if ((dev == NULL) || (dev->start_timer == NULL) || (dev->stop_timer == NULL)  ||
        (dev->set_timer_cnt == NULL) || (dev->get_timer_cnt == NULL) || (dev->delay_ms == NULL)) 
    {
        /* Device structure pointer is not valid */
        rslt = LMT_E_NULL_PTR;
    }
    else
    {
        /* Device structure is fine */
        rslt = LMT_OK;
    }

    return rslt;
}

/*!
 * @brief map a value from one scale to another.
 */
static double map(double val, double in_min, double in_max, double out_min, double out_max)
{
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
