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

#ifndef _lib_can_h_
#define _lib_can_h_

/* Includes */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <libsocketcan.h>

#ifdef __linux__
    #include <unistd.h>
    #include <net/if.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <linux/can.h>
    #include <linux/can/raw.h>
    #include <linux/if.h>
    #include <sys/time.h>
    #include <errno.h>
#endif

/* Definitions */
#define CAN_INTERFACE_UP        1
#define CAN_INTERFACE_DOWN      0

#ifdef __linux__
    #define CAN_CTRLMODE_LOOPBACK		0x01	/* Loopback mode */
    #define CAN_CTRLMODE_LISTENONLY		0x02	/* Listen-only mode */
    #define CAN_CTRLMODE_3_SAMPLES		0x04	/* Triple sampling mode */
    #define CAN_CTRLMODE_ONE_SHOT		0x08	/* One-Shot mode */
    #define CAN_CTRLMODE_BERR_REPORTING	0x10	/* Bus-error reporting */
    #define CAN_CTRLMODE_FD			    0x20	/* CAN FD mode */
    #define CAN_CTRLMODE_PRESUME_ACK	0x40	/* Ignore missing CAN ACKs */
    #define CAN_CTRLMODE_FD_NON_ISO		0x80	/* CAN FD in non-ISO mode */
#endif

/* CAN device modes */
#define CAN_MASTER 	0
#define CAN_SLAVE 	1

#define CAN_MAX_DLEN 8

/* Building Outside of cFS */
#ifndef OS_SUCCESS
    #define OS_SUCCESS          0
    #define OS_ERROR           -1
#endif

#define CAN_SUCCESS             OS_SUCCESS
#define CAN_ERROR               OS_ERROR
#define CAN_UP_ERR              -2
#define CAN_DOWN_ERR            -3
#define CAN_SET_MODES_ERR       -4
#define CAN_SET_BITRATE_ERR     -5
#define CAN_SOCK_OPEN_ERR       -6
#define CAN_SOCK_FLAGSET_ERR    -7
#define CAN_SOCK_BIND_ERR       -8
#define CAN_WRITE_ERR           -9
#define CAN_READ_ERR            -10
#define CAN_READ_TIMEOUT_ERR    -11
#define CAN_SOCK_SETOPT_ERR     -12

/*
** Defines specific to the Cubewheel can IDs to function with the RW sims
** These defines work around the multiple wheels talking to a single sim
*/
#define CW_CAN_HANDLE        0
#define CW_CAN_HANDLE_STR    "/dev/can0"
#define CW_ADDRESS           0x14
#define CW_WHL1_MASK         0x01
#define CW_WHL2_MASK         0x02
#define CW_WHL3_MASK         0x03
#define CW_TIMEOUT	         1
#define CW_BASE_CMD_LEN      5
#define CW_CAN_BASE_CMD_LEN  4
// end cubewheel specific defines

/*
 * Controller Area Network Identifier structure
 *
 * bit 0-28	: CAN identifier (11/29 bit)
 * bit 29	: error message frame flag (0 = data frame, 1 = error message)
 * bit 30	: remote transmission request flag (1 = rtr frame)
 * bit 31	: frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
 */
typedef uint32_t canid_t;

/* CAN device info struct */
typedef struct 
{
    const char* handle;   /* handle to the name of the network interface */
    uint8_t     isUp;     /* if the interface is up */
    /* Modes: each one can be on/off 
     * See section 6.5.1 of the socketcan kernel docs for reference 
     * https://www.kernel.org/doc/Documentation/networking/can.txt */

    bool        loopback;
    bool        listenOnly;
    bool        tripleSampling;
    bool        oneShot;
    bool        berrReporting;
    bool        fd;
    bool        presumeAck;
    uint32_t    bitrate;  /* bitrate for CAN device */  
} can_info_t;

#ifdef __rtems__
/**
 * struct can_frame - basic CAN frame structure
 * TODO: not tested with GRCAN
 */
struct can_frame{
    uint32_t  can_id;  /* 32 bit CAN_ID*/
    uint8_t   can_dlc; /* frame payload length in byte (0 .. CAN_MAX_DLEN) */
    uint8_t   data[CAN_MAX_DLEN];
};
#endif

/**
 * Initialize CAN device
 * @param device can_info_t struct with all can params  
 * @return Returns CAN_SUCCESS or CAN_ERROR
*/
int32_t can_init_dev(can_info_t* device);

/**
 * Sets all of the CAN devices modes (specified in the `can_info_t` device struct) in one go
 * @param device can_info_t struct with all can params
 * @return Returns CAN_SUCCESS or CAN_ERROR
 */
int32_t can_set_modes(can_info_t* device);

/**
 * Write a number of bytes to a given CAN interface
 * 
 * @param device can_info_t struct with all can params
 * @param can_id what to set can_id of frame as
 * @param buf pointer to data to write
 * @param length number of bytes to write the port (length of buf)
 * @return Returns CAN_SUCCESS or CAN_ERROR
*/
int32_t can_write(can_info_t* device, uint32_t can_id, uint8_t* buf, const uint32_t length);

/**
 * Blocking read for a number of bytes off of a given CAN interface
 * 
 * @param device can_info_t struct with all can params
 * @param can_frame readFrame pointer to read CAN frame
 * @param length number of bytes to read off the port
 * @return Returns CAN_SUCCESS or CAN_ERROR
*/
int32_t can_blocking_read(can_info_t* device, struct can_frame* readFrame, const uint32_t length);

/**
 * Non-blocking read for a number of bytes off of a given CAN interface
 * 
 * @param device can_info_t struct with all can params
 * @param can_frame readFrame pointer to read CAN frame
 * @param length number of bytes to read off the port
 * @param second_timeout seconds to wait for before timing out. Used in combo with microsecond_timeout
 * @param microsecond_timeout microseconds to wait for before timing out. Used in combo with second_timeout 
 * @return Returns CAN_SUCCESS or CAN_ERROR
*/
int32_t can_nonblocking_read(can_info_t* device, struct can_frame* readFrame, const uint32_t length, uint32_t second_timeout, uint32_t microsecond_timeout);

/**
 * Close the CAN device 
 * 
 * @param can_info_t struct with all can params
 * @return Returns CAN_SUCCESS or CAN_ERROR
*/
int32_t can_close_device(can_info_t* device);

/**
 * Excecute a CAN master write and slave read in one transaction
 *
 * @param device can_info_t struct with all can params
 * @param can_id what to set can_id of frame as
 * @param txbuf pointer to data to write
 * @param txlen number of bytes to write the port (length of buf)
 * @param rxbuf readFrame pointer to read CAN frame
 * @param rxlen number of bytes to read off the port
 * @param second_timeout seconds to wait for before timing out. Used in combo with microsecond_timeout
 * @param microsecond_timeout microseconds to wait for before timing out. Used in combo with second_timeout 
 * @return Returns CAN_SUCCESS or CAN_ERROR
 */
int32_t can_master_transaction(can_info_t* device, uint32_t can_id, uint8_t* txbuf, const uint32_t txlen, uint8_t* rxbuf, const uint32_t rxlen, uint32_t second_timeout, uint32_t microsecond_timeout);

#endif // _lib_can_h_