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

#include <fcntl.h>
#include "libtrq.h"
#include "libtrq_ioctl.h"

#define TRQ_FNAME_SIZE 50

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_set_time_high(): Configure the time high per period in nanoseconds for a TRQ device. Time high lengths
 *                      may not exceed a device's period length. 
 *
 * Inputs:              trq_info_t *device      -   TRQ device info structure 
 *                      uint32_t    new_time    -   New time high length for the device period in nanoseconds
 *
 * Outputs:             trq_info_t *device      -   High time set to new_time if successful
 *                      returns int32_t         -   TRQ_ERROR_* type on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_set_time_high(trq_info_t* device, uint32_t new_time) 
{
    return TRQ_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_set_period():    Configure the period length for a TRQ device in nanoseconds. Note that timer time high 
 *                      is set to zero before this function is called. 
 *
 * Inputs:              trq_info_t *device      -   TRQ device info structure 
 *
 * Outputs:             returns int32_t         -   TRQ_ERROR on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_set_period(trq_info_t* device) 
{
    return TRQ_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_set_direction():    Configure the direction of the TRQ device. 
 *
 * Inputs:              trq_info_t *device      -   TRQ device info structure 
 *                      bool        direction   -   New direction desired for device
 *
 * Outputs:             trq_info_t *device      -   positive_direction set to direction if successful
 *                      returns int32_t         -   TRQ_ERROR on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_set_direction(trq_info_t* device, bool direction) 
{
    return TRQ_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_init():          Opens a TRQ device and initializes it. This function can also be
 *                      used on an already opened TRQ number to reset it. 
 *
 * Inputs:              trq_info_t *device  -   TRQ Device info structure to initialize   
 * 
 * Outputs:             trq_info_t *device  -   Info structure contains AXI timer device file descriptor. 
 *                      returns int32_t     -   <0 on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_init(trq_info_t* device) 
{
    return TRQ_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_command():   Change TQR period, time high, or direction.
 *
 * Inputs:              trq_info_t* device      -   TQR device info structure to modify
 *                      uint8_t percent_high    -   Percent of the period to be high (0-100)
 *                      bool pos_dir            -   Direction - True for positive, False for negative
 *
 * Outputs:             trq_info_t *device      -   Parameters set to new values if successful
 *                      returns int32_t         -   TRQ_ERROR_* type on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_command(trq_info_t *device, uint8_t percent_high, bool pos_dir)
{
    return TRQ_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *
 * trq_close():         Disables and closes an active TRQ device. 
 * 
 * Inputs:              trq_info_t *device - TRQ device info structure for TRQ device to disable
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void trq_close(trq_info_t* device)
{
    return;
}
