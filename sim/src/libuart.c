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
#include <pthread.h>

/* psp */
//#include <cfe_psp.h>

/* osal */
//#include <osapi.h>

/* nos */
#include <Uart/Client/CInterface.h>

/* hwlib API */
#include "libuart.h"

/* size of uart buffer */
#define USART_RX_BUF_SIZE    4096

/* usart device handles */
static NE_Uart *usart_device[NUM_USARTS] = {0};

/* usart mutex */
static pthread_mutex_t nos_usart_mutex;

/* public prototypes */
void nos_init_usart_link(void);
void nos_destroy_usart_link(void);
int32_t sim_MutSemCreate (pthread_mutex_t *mutex, uint32_t options);
int32_t sim_MutSemDelete (pthread_mutex_t *mutex);
int32_t sim_MutSemGive (pthread_mutex_t *mutex);
int32_t sim_MutSemTake (pthread_mutex_t *mutex);

/* private prototypes */
static NE_Uart* nos_get_usart_device(int handle);

/* initialize nos engine usart link */
void nos_init_usart_link(void)
{
    /* create mutex */
    int32_t result = sim_MutSemCreate(&nos_usart_mutex, 0);

}

/* destroy nos engine usart link */
void nos_destroy_usart_link(void)
{
    int i;

    sim_MutSemTake(&nos_usart_mutex);

    /* clean up usart buses */
    
    for(i = 0; i <= NUM_USARTS; i++)
    {
        NE_Uart *dev = usart_device[i];
        if(dev) NE_uart_close(&dev);
    }
    
    sim_MutSemGive(&nos_usart_mutex);

    /* destroy mutex */
    int32_t result = sim_MutSemDelete(&nos_usart_mutex);
}

/* init usart */
int32_t uart_init_port(uart_info_t* device)
{
    int32_t status = OS_SUCCESS;
    if(device->handle >= 0)
    {

        /* get usart device handle */
        NE_Uart **dev = &usart_device[device->handle];
        if(*dev == NULL)
        {
            /* get nos usart connection params */
            const nos_connection_t *con = &nos_usart_connection[device->handle];

            /* try to initialize usart */
            *dev = NE_uart_open3(hub, "fsw", con->uri, con->bus, device->handle);

            if(*dev)
            {
                /* set default queue size */
                NE_uart_set_queue_size(*dev, USART_RX_BUF_SIZE);

                device->isOpen = PORT_OPEN;           
	        }
            else
            {
                OS_printf("nos uart_open failed\n");
		        device->isOpen = PORT_CLOSED;
		        status = OS_ERR_FILE;
            }
        }
    }
    else
    {
        OS_printf("Handle not found\n");
        device->isOpen = PORT_CLOSED;
        status = OS_ERR_FILE;
    }
    return status;
}

/* get usart device */
static NE_Uart* nos_get_usart_device(int handle)
{
    NE_Uart *dev = NULL;
    if(handle < NUM_USARTS)
    {
        dev = usart_device[handle];
    }
    return dev;
}

/* usart flush */
int32_t uart_flush(int32_t handle)
{
    NE_Uart *dev = nos_get_usart_device((int)handle);
    if(dev)
    {
        NE_uart_flush(dev);
    }
    return UART_SUCCESS;
}

/* usart write */
int32_t uart_write_port(int32_t handle, uint8_t data[], const uint32_t numBytes)
{
    int32_t status = OS_ERR_FILE;
    NE_Uart *dev = nos_get_usart_device((int)handle);
    if(dev)
    {
        status = NE_uart_write(dev, (const uint8_t*)data, numBytes); //Can this function return -1?
    }
    return status;
}

/* usart read */
int32_t uart_read_port(int32_t handle, uint8_t data[], const uint32_t numBytes)
{
    uint32_t status = OS_ERR_FILE;

    if (data != NULL) //Check that there is actually data to read
    { 
        char c = 0xFF;
        int  i;
        int stat;
        NE_Uart *dev = nos_get_usart_device((int)handle);
        if(dev)
        {
            for (i = 0; i < (int)numBytes; i++) //TODO: Add ability to switch between blocking and non-blocking?
            {
                /*
                //NON BLOCKING MODE
                stat = NE_uart_getc(dev, (uint8_t*)&c); //Returns 0 if byte read, 1 if no byte actually read
                if(stat == 1)
                {
                    return i; //Causes app to immediately enter service mode
                }
                else {
                    data[i] = c;
                }
                */
                //BLOCKING MODE
                do {
                    stat = NE_uart_getc(dev, (uint8_t*)&c);
                    if (stat == 1)
                    {
                        OS_TaskDelay(1);
                    }
                } while(stat); 
                data[i] = c;
            }
            status = numBytes;
            
            return status;
        }
        return status; //There is data, but can't read from device
    }
    return status; //Following arm_inux model
}

/* usart number bytes available */
int32_t uart_bytes_available(int32_t handle)
{
    int bytes = 0;
    NE_Uart *dev = nos_get_usart_device((int)handle);
    if(dev)
    {
        bytes = (int)NE_uart_available(dev);
    }
    return bytes;
}

int32_t uart_close_port(int32_t handle) 
{
    NE_UartStatus status;
    NE_Uart *dev = nos_get_usart_device((int)handle);
    if (handle >= 0)
    {
        if(dev)
        {
            status = NE_uart_close(&dev);
            usart_device[handle] = 0;
        }
    }
    if (status == NE_UART_SUCCESS) {
        return OS_SUCCESS;
    }
    else
    {
        return OS_ERROR;
    }
    
}

int32_t sim_MutSemCreate (pthread_mutex_t *mutex, uint32_t options)
{
    int                 return_code;
    pthread_mutexattr_t mutex_attr;

    /*
    ** initialize the attribute with default values
    */
    return_code = pthread_mutexattr_init(&mutex_attr);
    if ( return_code != 0 )
    {
       OS_printf("Error: Mutex could not be created. pthread_mutexattr_init failed: %s\n",
             strerror(return_code));
       return OS_ERROR;
    }

    /*
    ** Allow the mutex to use priority inheritance
    */
    return_code = pthread_mutexattr_setprotocol(&mutex_attr,PTHREAD_PRIO_INHERIT);
    if ( return_code != 0 )
    {
       OS_printf("Error: Mutex could not be created. pthread_mutexattr_setprotocol failed: %s\n",
             strerror(return_code));
       return OS_ERROR;
    }

    /*
    **  Set the mutex type to RECURSIVE so a thread can do nested locks
    */
    return_code = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    if ( return_code != 0 )
    {
       OS_printf("Error: Mutex could not be created. pthread_mutexattr_settype failed: %s\n",
             strerror(return_code));
       return OS_ERROR;
    }

    /*
    ** create the mutex
    ** upon successful initialization, the state of the mutex becomes initialized and unlocked
    */
    return_code = pthread_mutex_init(mutex,&mutex_attr);
    if ( return_code != 0 )
    {
       OS_printf("Error: Mutex could not be created: %s\n",
             strerror(return_code));
       return OS_ERROR;
    }

    return OS_SUCCESS;
}


int32_t sim_MutSemDelete (pthread_mutex_t *mutex)
{
    int status;

    status = pthread_mutex_destroy(mutex); /* 0 = success */

    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;

}


int32_t sim_MutSemGive (pthread_mutex_t *mutex)
{
   int status;

   /*
    ** Unlock the mutex
    */
   status = pthread_mutex_unlock(mutex);
   if(status != 0)
   {
      return OS_ERROR;
   }

   return OS_SUCCESS;
}


int32_t sim_MutSemTake (pthread_mutex_t *mutex)
{
    int status;

    /*
    ** Lock the mutex
    */
    status = pthread_mutex_lock(mutex);
    if( status != 0 )
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

