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

// Write out to CAN bus from CAN device specified by `device`.
int32_t can_write(can_info_t* device, uint32_t can_id, uint8_t* buf, const uint32_t length)
{
    return can_master_transaction(CW_CAN_HANDLE, can_id, buf, length, NULL, 0, 0, 0);
}

// Read a can_frame from CAN interface specified by `device->handle`. Does a blocking read call.
int32_t can_blocking_read(can_info_t* device, struct can_frame* readFrame, const uint32_t length)
{
    return can_master_transaction(CW_CAN_HANDLE, readFrame->can_id << 3, NULL, 0, &(readFrame->data[0]), length, 0, 0);
}

// Read a can_frame from CAN interface specified by `device->handle`. Does a nonblocking read call.
int32_t can_nonblocking_read(can_info_t* device, struct can_frame* readFrame, const uint32_t length, uint32_t second_timeout, uint32_t microsecond_timeout) 
{
    return can_master_transaction(CW_CAN_HANDLE, readFrame->can_id << 3, NULL, 0, &(readFrame->data[0]), length, 0, 0);
}

//int32 can_master_transaction(int handle, uint32_t identifier, void * txbuf, uint8_t txlen, void * rxbuf, uint8_t rxlen, uint16_t timeout)
int32_t can_master_transaction(can_info_t* device, uint32_t can_id, uint8_t* txbuf, const uint32_t txlen, uint8_t* rxbuf, const uint32_t rxlen, uint32_t second_timeout, uint32_t microsecond_timeout)
{
    int result = OS_ERROR;
    
    /* get can device handle */
    NE_CanHandle *dev = nos_get_can_device(device);

    /* can transaction */
    OS_MutSemTake(nos_can_mutex);
    if(dev)
    {
        if ( (can_id & 0xF) == CW_WHL1_MASK || (can_id & 0xF) == CW_WHL2_MASK || (can_id & 0xF) == CW_WHL3_MASK )
        {
            result = NE_can_transaction(dev, CW_ADDRESS, txbuf, txlen, rxbuf, rxlen);                
        }            

        else if (can_id == CW_ADDRESS)
        {
            result = NE_can_transaction(dev, CW_ADDRESS, txbuf, txlen, rxbuf, rxlen);            
        }

        else 
        {
            //OS_printf("LIBCAN: %s:  CAN IDENTIFIER IS NOT CW_ADDRESS, NOR CONTAINS THE WHEEL CAN MASKS\n", __FUNCTION__);
            result = NE_can_transaction(dev, can_id, txbuf, txlen, rxbuf, rxlen);
        }
    }

    OS_MutSemGive(nos_can_mutex);

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