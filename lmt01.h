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
 * File        lmt01.h
 * Created by  Sean Farrelly
 * Version     1.0
 * 
 */

/*! @file lmt01.h
 * @brief Driver for LMT01 temperature sensor.
 */

#ifndef _LMT01_H_
#define _LMT01_H_

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*!
  * @brief  Enum defining the different temperature conversion techniques.
  *         These are either by Equation, or by Lookup Table.
  */
typedef enum {
    CONV_TYPE_EQU,
    CONV_TYPE_LUT
}   lmt_conv_t;

/*!
 * @brief lmt API return codes
 */
typedef enum {
    LMT_OK,
    LMT_E_NULL_PTR,
    LMT_E_DEV_NOT_FOUND,
    LMT_E_TIMEOUT
} lmt_status_t;

/*!
 * @brief Type definitions
 */
typedef void (*lmt_timer_mode_fptr_t)(void* timer);
typedef void (*lmt_timer_cnt_fptr_t)(void* timer, uint32_t *cnt);
typedef void (*lmt_delay_ms_fptr_t)(uint32_t ms);

/*!
 * @brief  lmt01 device structure
 */
typedef struct
{
    /* Timer context pointer */
    void *timer;
    
    /* Start timer function pointer */
    lmt_timer_mode_fptr_t start_timer;

    /* Stop timer function pointer */
    lmt_timer_mode_fptr_t stop_timer;

    /* Set timer count function pointer */
    lmt_timer_cnt_fptr_t set_timer_cnt;

    /* Get timer count function pointer */
    lmt_timer_cnt_fptr_t get_timer_cnt;

    /* Delay (ms) function pointer */
    lmt_delay_ms_fptr_t delay_ms;    

} lmt01_dev_t;


/**
  * @brief  Initialise lmt01 device and check if alive.
  * 
  * @param[in] dev : LMT01 device structure
  * 
  * @return result of API execution status
  * @retval lmt_status_t
  */
lmt_status_t lmt_init(const lmt01_dev_t *dev);

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
lmt_status_t lmt_get_temperature(const lmt01_dev_t *dev, float *temp, lmt_conv_t type);


/**
  * @brief Obtain one temperature reading from device by counting pulses.
  * 
  * @param[in] dev : LMT01 device structure.
  * @param[out] pulses : Pointer to variable to store pulse count.
  * 
  * @return result of API execution status
  * @retval lmt_status_t
  */
lmt_status_t lmt_get_pulse_count(const lmt01_dev_t *dev, uint32_t *pulses);

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
float lmt_pulses_to_temperature(uint32_t pulses, lmt_conv_t type);

#ifdef __cplusplus
}
#endif /* End of CPP guard */
#endif /* LMT01_H_ */
/** @}*/