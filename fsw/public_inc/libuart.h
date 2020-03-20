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

#ifndef _lib_uart_h_
#define _lib_uart_h_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#if defined __linux__ || defined __rtems__
    #include <fcntl.h>         /* for posix open()  */
    #include <unistd.h>        /* for posix close() */
    #include <string.h>
    #include <sys/ioctl.h>
    #include <termios.h>
#endif

/*
** Misc Defines
*/
#define PORT_OPEN    1
#define PORT_CLOSED  0

#ifndef OS_SUCCESS
    // Building Outside of cFS
    #define OS_SUCCESS          0
    #define OS_ERROR           -1
    #define OS_ERR_FILE        -2    
#endif

/* data types */
typedef struct
{
    char  *deviceString;       /* uart string descriptor of the port  */
    int32_t  handle;           /* handle to device */
    uint8_t  isOpen;           /* port status */
    uint32_t baud;             /* baud rate */
#if defined __linux__ || defined __rtems__
    struct termios options;
#endif
} uart_info_t;

/*
 * Generic uart initialization/ port open
 * 
 * @param device uart_info_t struct with all uart params
 * @return Returns error code: OS_SUCCESS, or OS_ERROR
*/
int32_t uart_init_port(uart_info_t* device);

/*
 * Get the number of bytes waiting to be read for a given port
 * 
 * @param device number of the uart port
 * @return Returns error code: OS_SUCCESS, or OS_ERROR
*/
int32_t uart_bytes_available(int32_t handle);

/*
 * Read a number of bytes off of a given uart port
 * 
 * @param handle handle to the uart port when open
 * @param data array to store the read data
 * @param numBytes number of bytes to read off the port
 * @return Returns error code: OS_SUCCESS, or OS_ERROR
*/
int32_t uart_read_port(int32_t handle, uint8_t data[], const uint32_t numBytes);

/*
 * Write a number of bytes to of a given uart port
 * 
 * @param deviceString string descriptor of the port
 * @param data array of the data to write
 * @param numBytes number of bytes to write to the port
 * @return Returns error code: OS_SUCCESS, or OS_ERROR
*/
int32_t uart_write_port(int32_t handle, uint8_t data[], const uint32_t numBytes);

/*
 * Generic uart initialization/ port close
 * 
 * @param device uart_info_t struct with all uart params
 * @return Returns error code: OS_SUCCESS, or OS_ERROR
*/
int32_t uart_close_port(int32_t handle);

#endif