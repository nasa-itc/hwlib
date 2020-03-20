/* Copyright (C) 2009 - 2018 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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

#ifndef _lib_i2c_h_
#define _lib_i2c_h_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef __linux__
  #include <fcntl.h>
  #include <sys/time.h>
  #include <linux/i2c-dev.h>
  #include <sys/ioctl.h>
  #include <unistd.h>
#endif

/*
 * I2C device modes
*/
#define I2C_OPEN                0
#define I2C_CLOSED              1

/* Number of I2C busses */
#define NUM_I2C                 2

/* Max size of I2C transaction */
#define I2C_MAX_BYTES           128

/* Building Outside of cFS */
#ifndef OS_SUCCESS
    #define OS_SUCCESS          0
    #define OS_ERROR           -1
#endif

/*
** I2C bus info struct
*/
typedef struct
{
    int32_t  handle;   /* handle to the fd */
    uint8_t  isOpen;   /* port status */
    uint16_t speed;    
} i2c_bus_info_t;

/* 
 * Initialize I2C handle as Master with bus speed
 *
 * @param Device Which I2C bus (if more than one exists)
 * @param speed Bus speed in kbps
 * @return Returns error code: OS_SUCCESS, or OS_ERROR
*/
int32_t i2c_master_init(i2c_bus_info_t* device);

/**
 * Execute a I2C master write and slave read in one transaction
 *
 * @param handle Handle to the device
 * @param addr I2C address, not bit-shifted
 * @param txbuf pointer to tx data
 * @param txlen length of tx data
 * @param rxbuf pointer to rx data
 * @param rxlen length of rx data
 * @param timeout Number of ticks to wait for a frame
 * @return Returns error code: OS_SUCCESS if a frame is received, or OS_ERROR if timed out or if handle is not a valid device
 */
int32_t i2c_master_transaction(int32_t handle, uint8_t addr, void * txbuf, uint8_t txlen, void * rxbuf, uint8_t rxlen, uint16_t timeout);

/**
 * Execute a I2C slave read
 *
 * @param handle Handle to the device
 * @param addr I2C address, not bit-shifted
 * @param rxbuf pointer to rx data
 * @param rxlen length of rx data
 * @param timeout Number of ticks to wait for a frame
 * @return Returns error code: OS_SUCCESS if a frame is received, or OS_ERROR if timed out or if handle is not a valid device
 */
int32_t i2c_read_transaction(int32_t handle, uint8_t addr, void * rxbuf, uint8_t rxlen, uint8_t timeout);

/**
 * Execute a I2C master write 
 *
 * @param handle Handle to the device
 * @param addr I2C address, not bit-shifted
 * @param txbuf pointer to tx data
 * @param txlen length of tx data
 * @param timeout Number of ticks to wait for a frame
 * @return Returns error code: OS_SUCCESS if a frame is received, or OS_ERROR if timed out or if handle is not a valid device
 */
int32_t i2c_write_transaction(int32_t handle, uint8_t addr, void * txbuf, uint8_t txlen, uint8_t timeout);

#endif