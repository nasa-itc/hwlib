/* Copyright (C) 2009 - 2019 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including, but not
limited to, any warranty that the software will conform to, specifications any implied warranties of merchantability, fitness
for a particular purpose, and freedom from infringement, and any warranty that the documentation will conform to the program, or
any warranty that the software will be error free.

In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or consequential damages,
arising out of, resulting from, or in any way connected with the software or its documentation.  Whether or not based upon warranty,
contract, tort or otherwise, and whether or not loss was sustained from, or arose out of the results of, or use of, the software,
documentation or services provided hereunder

*/


#ifdef __rtems__
    #include <rtems.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <grlib/grcan.h>
#endif

#include "cfe.h"

#include "libcan.h"



#define CAN_HWLIB_MAX_GRCAN_DEVICES    8
#define CAN_HWLIB_TASK_DELAY_VALUE     10


typedef struct 
{
    void * device;         /* device identifier */
    const char* handle;    /*  name of the device */
    int   state;           /* device state */
} grcan_info_t;


static grcan_info_t CAN_GrcanDevices[CAN_HWLIB_MAX_GRCAN_DEVICES];



void can_convert_read_msg(struct can_frame* readFrame, CANMsg *msg, const uint32_t length);
void can_convert_write_msg(uint32_t can_id, uint8_t* buf, const uint32_t length, CANMsg *msg);
void can_check_start_mode(void * dev);

void * can_convert_name_to_dev_ptr( char *name);


/*
 * The RTEMS GRCAN driver is accessed with functions based interface defined by the header file grcan.h 
 * Documentation here: https://www.gaisler.com/anonftp/rcc/rcc-1.3/1.3-rc7/rcc-1.3-rc7.pdf 
 */

// Bring CAN network interface
int32_t can_init_dev(can_info_t* device) 
{
    int retVal;
    void * dev;
    int devNum;
    struct grcan_timing btrs = {0x1, 0xf, 0x7, 0x1, 0x0}; // 1Mbps at 48MHz clock
    // struct grcan_timing btrs = {0x1, 0xf, 0x8, 0x1, 0x0}; // 1Mbps at 50MHz clock
  
    dev = grcan_open_by_name(device->handle, &devNum);
    if ( !dev )
    {
         return CAN_ERROR;
    }
    else
    {
  
         /* set baudrate */
         retVal = grcan_set_btrs(dev, &btrs);
         //retVal = grcan_set_speed(dev,  device->bitrate);
         if (retVal < 0) 
         {
              return CAN_SET_BITRATE_ERR;
         }
                       
         CAN_GrcanDevices[devNum].device = dev;
         CAN_GrcanDevices[devNum].handle = device->handle;

         /* get CAN state */
         CAN_GrcanDevices[devNum].state = grcan_get_state(dev);

         retVal = can_set_modes(device);
         if (retVal < 0) 
         {
              return CAN_SET_MODES_ERR;
         }
    }


    device->isUp = CAN_INTERFACE_UP;

    return CAN_SUCCESS;
}

// Call the `libsocketcan` function to set each CAN mode on/off
int32_t can_set_modes(can_info_t* device) 
{
    void * dev;

    dev = can_convert_name_to_dev_ptr(device->handle);
    if ( !dev )
    {
         return CAN_ERROR;
    }
    else
    {
        grcan_stop(dev);
        grcan_set_abort(dev, device->berrReporting);
        grcan_set_afilter(dev, NULL);
        grcan_start(dev);
    }
    
  return CAN_SUCCESS;
}

/* Write out to CAN bus from GRCAN driver */
int32_t can_write(can_info_t* device) 
{
    int ret;
    CANMsg msg;
    void *dev;
  
    dev = can_convert_name_to_dev_ptr(device->handle);
    if(dev)
    {
         can_convert_write_msg(device->tx_frame.can_id, device->tx_frame.data, device->tx_frame.can_dlc, &msg);        
     
         /* 
         ** make sure that the driver is in GRCAN_START mode 
         ** and clear any errors
         */
         can_check_start_mode(dev);
                  
         /* write one CAN message */
         ret = grcan_write(dev, &msg, 1);
         
         if (ret <= 0 )
         {
              ret = CAN_WRITE_ERR;
         }
         else
         {
             ret = CAN_SUCCESS;
             //grcan_flush(dev);
         }
    
    }
    else
    {
         ret = CAN_SOCK_BIND_ERR;
    }

    return ret;
}

// Read a can_frame from SocketCAN interface specified by `device->handle`. Does a nonblocking read call
int32_t can_read(can_info_t* device) 
{		
    int ret = CAN_SUCCESS;
    CANMsg msg;
    void *dev;
    uint32_t total_ms;
    uint32_t ms = 0;
    uint8_t timedout = FALSE;

    dev = can_convert_name_to_dev_ptr(device->handle);
    if(dev)
    {
         total_ms = (device->second_timeout * 1000) + (device->microsecond_timeout/1000);
         can_check_start_mode(dev); /* make sure GRCAN is in start mode */
      
         grcan_set_rxblock(dev, 0); /* turn off block mode */

         while(1)
         {
              /* read one CAN message */
              ret = grcan_read(dev, &msg, 1);

             if (timedout || ret != GRCAN_RET_TIMEOUT)
              {
                   break;
              }

              OS_TaskDelay(CAN_HWLIB_TASK_DELAY_VALUE);

              ms += CAN_HWLIB_TASK_DELAY_VALUE;
              if (ms >= total_ms )
              {
                   timedout = TRUE;
              }
       
         }
 
         if (ret == GRCAN_RET_TIMEOUT || ret == 0)
         {
               ret = CAN_READ_TIMEOUT_ERR;
         }
         else if (ret <= 0)
         {
               ret = CAN_READ_ERR;
         }
         else 
         {
               can_convert_read_msg(&device->rx_frame, &msg, 8);
               ret = CAN_SUCCESS;
         }                
    }
    else
    {
         ret = CAN_SOCK_BIND_ERR;
    }

    return ret;

}

// Bring CAN network interface down
int32_t can_close_device(can_info_t* device) 
{
    void * dev;

    dev = can_convert_name_to_dev_ptr(device->handle);
    if(dev)
    {
       grcan_close(dev);
    }
    else
    {
        return CAN_ERROR;
    }
    
    
  return CAN_SUCCESS;
}

// Perform non-blocking can transaction
int32_t can_master_transaction(can_info_t* device) 
{
     int32_t status;
     struct can_frame readframe;
  
     status = can_write(device);
     if (status != CAN_SUCCESS)
     {
          return status;
     }

     usleep(device->xfer_us_delay);
     
     if (status == CAN_SUCCESS)
     {
          status = can_read(device);
     }

  return status;
}



void * can_convert_name_to_dev_ptr( char *name)
{
    int i;
    
    for (i = 0; i < CAN_HWLIB_MAX_GRCAN_DEVICES; i++)
    {

         if (strncmp(CAN_GrcanDevices[i].handle, name, strlen(name)) == 0) 
         {
             return CAN_GrcanDevices[i].device;
         }

    }
    
    return NULL;
}

/* The GRCAN driver should always be in STATE_STARTED mode */
void can_check_start_mode(void * dev)
{
   int ret;
   
   ret = grcan_get_state(dev);

   if (ret == STATE_STARTED)
   {
       return;
   }
   else if (ret == GRCAN_RET_AHBERR || ret == GRCAN_RET_BUSOFF )
   {    
         grcan_stop(dev);
   }

    grcan_start(dev);

}



void can_convert_write_msg(uint32_t can_id, uint8_t* buf, const uint32_t length, CANMsg *msg)
{
    if (can_id > 0x7FF)
    {
         msg->extended = 1;
    }
    else 
    {
         msg->extended = 0;
    }
         
    if (can_id & 0x40000000)
    {
         msg->rtr = 1;  /* RTR - Remote Transmission Request */
    }
    else
    {
         msg->rtr = 0;  /* RTR - Remote Transmission Request */
    }
         
    can_id &= (0x1FFFFFFF); /* clear the frame error, rtr, and format bits */
        
    msg->unused = 0;
    msg->id = can_id;
         
    msg->len = length;
    
    
    memcpy(&msg->data[0], buf, length);
     
}



void can_convert_read_msg(struct can_frame* readFrame, CANMsg *msg, const uint32_t length)
{
    memset(readFrame, 0, sizeof(struct can_frame));
 
    if ( msg->extended )
    {
         readFrame->can_id |= 1 << 31;
    }
         
    if ( msg->rtr )
    {
         readFrame->can_id |= 1 << 30;
    }
       
    readFrame->can_id  += msg->id; 
    readFrame->can_dlc = msg->len;
     
    memcpy(&readFrame->data, &msg->data, length);   

}

