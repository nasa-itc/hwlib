/* Copyright (C) 2009 - 2019 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including, but not
limited to, any warranty that the software will conform to, specifications any implied warranties of merchantability, fitness
for a particular purpose, and freedom from infringement, and any warranty that the documentation will conform to the program, or
any warranty that the software will be error free.

In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or consequential damages,
arising out of, resulting from, or in any way connected with the software or its documentation.  Whether or not based upon warranty,
contract, tort or otherwise, and whether or not loss was sustained from, or arose out of the results of, or use of, the software,
documentation or services provided hereunder

ITC Team
NASA IV&V
ivv-itc@lists.nasa.gov
*/

#ifndef _lib_pwm_h_
#define _lib_pwm_h_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __linux__
    #include <unistd.h>
#endif

/* Enum to restrict timer creation to each axis,
 *   and to help map timer to fabric interrupt */
typedef enum {
    X, Y, Z
} AXIS;

/* Building Outside of cFS */
#ifndef OS_SUCCESS
    #define OS_SUCCESS          0
    #define OS_ERROR           -1
#endif

#define PWM_SUCCESS             OS_SUCCESS
#define PWM_ERROR               OS_ERROR
#define PWM_INIT_ERR            -1
#define PWM_SELFTEST_ERR        -2
#define PWM_CONNECT_ERR         -3
#define PWM_AXIS_UNKNOWN_ERR    -4
#define PWM_TIME_HIGH_VAL_ERR   -5

/* PWM timer info struct */
typedef struct {
    int         timerfd; 
    AXIS        which_axis;
    uint32_t    TimerPeriod;   // nanoseconds
    uint32_t    TimerTimeHigh; // nanoseconds
    bool        enabled;
} pwm_info_t;

/*
 * Initialize PWM timer
 * @param device pwm_info_t struct with all required params
 * @return Returns PWM_SUCCESS or an error code defined above
 */

int32_t pwm_init_timer(pwm_info_t* device, AXIS axis);
void    pwm_exit_timer(pwm_info_t* device); 
int32_t pwm_set_period(pwm_info_t* device, uint32_t new_period);
int32_t pwm_set_time_high(pwm_info_t* device, uint32_t new_time);

#endif
