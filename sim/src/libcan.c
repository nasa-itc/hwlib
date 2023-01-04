/* Copyright (C) 2009 - 2016 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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

#include "nos_link.h"
#include <stdint.h>
#include <stdlib.h>

/* psp */
#include <cfe_psp.h>

/* osal */
#include <osapi.h>

/* nos */
#include <Can/Client/CInterface.h>

#include "libcan.h"

#define CW_CAN_BASE_CMD_LEN  8

/* can device handles */
static NE_CanHandle *can_device[NUM_CAN_DEVICES] = {0};

/* can mutex */
static uint32 nos_can_mutex = 0;

/* public prototypes */
void nos_init_can_link(void);
void nos_destroy_can_link(void);

/* get spi device */
static NE_CanHandle* nos_get_can_device(can_info_t* device)
{
    NE_CanHandle *dev = NULL;
    if(!strcmp(device->handle, CW_CAN_HANDLE_STR))
    {
        dev = can_device[CW_CAN_HANDLE];
        if(dev == NULL)
        {
            can_init_dev(device);
            dev = can_device[CW_CAN_HANDLE];
        }
    }
    else
    {
        dev = can_device[1];
        if(dev == NULL)
        {
            can_init_dev(device);
            dev = can_device[1];
        }
    }
    
    return dev;
}

/* initialize nos engine can link */
void nos_init_can_link(void)
{
    /* create mutex */
    int32 result = OS_MutSemCreate(&nos_can_mutex, "nos_can", 0);

}

/* destroy nos engine can link */
void nos_destroy_can_link(void)
{
    OS_MutSemTake(nos_can_mutex);

    /* clean up can buses */
    int i;
    for (i = 0; i < NUM_CAN_DEVICES; i++)
    {
        NE_CanHandle *dev = can_device[i];
        if (dev) 
            NE_can_close(&dev);
    }
    
    OS_MutSemGive(nos_can_mutex);

    /* destroy mutex */
    int32 result = OS_MutSemDelete(nos_can_mutex);
}

// Bring CAN network interface
int32_t can_init_dev(can_info_t* device)
{
    int32 result = OS_SUCCESS;
    NE_CanHandle **dev;
    const nos_connection_t *con;

    if(!strcmp(device->handle, CW_CAN_HANDLE_STR))
    {

        dev = &can_device[CW_CAN_HANDLE];
        if (*dev == NULL)
        {
            /* get nos can connection params */
            con = &nos_can_connection[CW_CAN_HANDLE];
        }
    }
    else
    {
        // TODO - UPDATE with mission defined device strings
        dev = &can_device[1];
        if (*dev == NULL)
        {
            /* get nos can connection params */
            con = &nos_can_connection[1];
        }
    }

    /* try to initialize master */
    *dev = NE_can_init_master3(hub, 10, con->uri, con->bus);
    if (*dev == NULL)
    {
        result = OS_ERROR;
        OS_printf("LIBCAN: %s:  FAILED TO INITIALIZE NOS CAN MASTER\n", __FUNCTION__);
    }

    OS_MutSemGive(nos_can_mutex);
    return result;        
}

// TODO: NOT IMPLEMENTED!
int32_t can_set_modes(can_info_t* device) 
{
	return CAN_SUCCESS;
}

// Write a can_frame  from `device->tx_Frame` to CAN bus from SocketCAN socket specified by `device`
int32_t can_write(can_info_t* device)
{
    return can_master_transaction(device);
}

// Read a can_frame from SocketCAN interface specified by `device` into `device->rx_frame`
int32_t can_read(can_info_t* device)
{
    return can_master_transaction(device);
}

int32_t can_master_transaction(can_info_t* device)
{
    int result = CAN_ERROR;
    int i;
    
    /* get can device handle */
    NE_CanHandle *dev = nos_get_can_device(device);

    /* can transaction */
    OS_MutSemTake(nos_can_mutex);
    if(dev)
    {
        if ( (device->tx_frame.can_id & 0xFF) == CW_WHL1_MASK || (device->tx_frame.can_id & 0xFF) == CW_WHL2_MASK || (device->tx_frame.can_id & 0xFF) == CW_WHL3_MASK )
        {
            result = NE_can_transaction(dev, CW_ADDRESS, 
                                       (uint8_t*) &device->tx_frame, device->tx_frame.can_dlc + CW_CAN_BASE_CMD_LEN, 
                                       (uint8_t*) &device->rx_frame, CW_CAN_BASE_CMD_LEN + CAN_MAX_DLEN);
        }            
        else if (device->tx_frame.can_id == CW_ADDRESS)
        {
            result = NE_can_transaction(dev, CW_ADDRESS, 
                                       (uint8_t*) &device->tx_frame, device->tx_frame.can_dlc + CW_CAN_BASE_CMD_LEN, 
                                       (uint8_t*) &device->rx_frame, CW_CAN_BASE_CMD_LEN + CAN_MAX_DLEN);
        }
        else 
        {
            result = NE_can_transaction(dev, device->tx_frame.can_id,
                                       (uint8_t*) &device->tx_frame, device->tx_frame.can_dlc + CW_CAN_BASE_CMD_LEN, 
                                       (uint8_t*) &device->rx_frame, CW_CAN_BASE_CMD_LEN + CAN_MAX_DLEN);
        }
    }
    OS_MutSemGive(nos_can_mutex);

    #ifdef LIBCAN_VERBOSE
        OS_printf("can_master_transaction: \n");
        OS_printf("  can_id = 0x%08x \t tx: 0x", device->tx_frame.can_id);
        for (i = 0; i < device->tx_frame.can_dlc; i++)
        {
        OS_printf("%02x ", device->tx_frame.data[i]);
        }
        OS_printf("\n");
        OS_printf("  can_id = 0x%08x \t rx: 0x", device->rx_frame.can_id);
        for (i = 0; i < device->rx_frame.can_dlc; i++)
        {
        OS_printf("%02x ", device->rx_frame.data[i]);
        }
        OS_printf("\n");
    #endif

    return result;
}

// Bring CAN network interface down
int32_t can_close_device(can_info_t* device)
{
    OS_MutSemTake(nos_can_mutex);

    /* clean up can device */
    NE_CanHandle *dev = can_device[CW_CAN_HANDLE];
    if(dev) NE_can_close(&dev);
    
    OS_MutSemGive(nos_can_mutex);

    /* destroy mutex */
    return NE_CAN_SUCCESS;
}