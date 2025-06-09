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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "simulith.h"
#include "libuart.h"

/* size of uart buffer */
#define NUM_USARTS 16
#define USART_RX_BUF_SIZE    4096

/* Track which ports are initialized */
static uint8_t usart_initialized[NUM_USARTS] = {0};

/* init usart */
int32_t uart_init_port(uart_info_t* device)
{
    int32_t status = OS_SUCCESS;
    int32_t sim_status;
    
    if(device->handle >= 0 && device->handle < NUM_USARTS)
    {
        if(!usart_initialized[device->handle])
        {
            /* Initialize the UART port with Simulith */
            sim_status = simulith_uart_init(device->handle, NULL);
            if(sim_status >= 0)  /* Simulith returns bytes written/read on success (>= 0) */
            {
                usart_initialized[device->handle] = 1;
                device->isOpen = PORT_OPEN;
            }
            else
            {
                OS_printf("Simulith uart_init failed with status %d\n", sim_status);
                device->isOpen = PORT_CLOSED;
                status = OS_ERR_FILE;
            }
        }
        else
        {
            /* Port already initialized */
            device->isOpen = PORT_OPEN;
        }
    }
    else
    {
        OS_printf("Handle %d out of range [0-%d]\n", device->handle, NUM_USARTS-1);
        device->isOpen = PORT_CLOSED;
        status = OS_ERR_FILE;
    }
    return status;
}

/* usart flush */
int32_t uart_flush(uart_info_t* device)
{
    int32_t sim_status;
    uint8_t temp_byte;
    int32_t bytes_available;

    if(device->handle < NUM_USARTS && usart_initialized[device->handle])
    {
        /* Get number of bytes in receive buffer */
        bytes_available = simulith_uart_available(device->handle);
        if(bytes_available < 0)
        {
            OS_printf("Simulith uart_available failed during flush with status %d\n", bytes_available);
            return OS_ERR_FILE;
        }

        /* Read and discard all pending data */
        while(bytes_available > 0)
        {
            sim_status = simulith_uart_receive(device->handle, &temp_byte, 1);
            if(sim_status < 0)
            {
                OS_printf("Simulith uart_receive failed during flush with status %d\n", sim_status);
                return OS_ERR_FILE;
            }
            
            bytes_available = simulith_uart_available(device->handle);
            if(bytes_available < 0)
            {
                OS_printf("Simulith uart_available failed during flush with status %d\n", bytes_available);
                return OS_ERR_FILE;
            }
        }
        return UART_SUCCESS;
    }
    return OS_ERR_FILE;
}

/* usart write */
int32_t uart_write_port(uart_info_t* device, uint8_t data[], const uint32_t numBytes)
{
    int32_t sim_status;

    if(device->handle < NUM_USARTS && usart_initialized[device->handle])
    {
        sim_status = simulith_uart_send(device->handle, data, numBytes);
        if(sim_status >= 0)
        {
            return sim_status; /* Return actual number of bytes written */
        }
        OS_printf("Simulith uart_send failed with status %d\n", sim_status);
    }
    return OS_ERR_FILE;
}

/* usart read */
int32_t uart_read_port(uart_info_t* device, uint8_t data[], const uint32_t numBytes)
{
    if(device->handle < NUM_USARTS && usart_initialized[device->handle] && data != NULL)
    {
        uint32_t bytes_read = 0;
        uint8_t byte;
        int32_t sim_status;
        
        while(bytes_read < numBytes)
        {
            sim_status = simulith_uart_receive(device->handle, &byte, 1);
            if(sim_status > 0)
            {
                data[bytes_read++] = byte;
            }
            else if(sim_status < 0)
            {
                OS_printf("Simulith uart_receive failed with status %d\n", sim_status);
                return (bytes_read > 0) ? bytes_read : OS_ERR_FILE;
            }
            else /* sim_status == 0, no data available */
            {
                OS_TaskDelay(1);
            }
        }
        return bytes_read;
    }
    return OS_ERR_FILE;
}

/* usart number bytes available */
int32_t uart_bytes_available(uart_info_t* device)
{
    int32_t sim_status;

    if(device->handle < NUM_USARTS && usart_initialized[device->handle])
    {
        sim_status = simulith_uart_available(device->handle);
        if(sim_status >= 0)
        {
            return sim_status;
        }
        OS_printf("Simulith uart_available failed with status %d\n", sim_status);
    }
    return 0;
}

int32_t uart_close_port(uart_info_t* device) 
{
    int32_t sim_status;

    if(device->handle >= 0 && device->handle < NUM_USARTS && usart_initialized[device->handle])
    {
        sim_status = simulith_uart_close(device->handle);
        if(sim_status >= 0)
        {
            usart_initialized[device->handle] = 0;
            device->isOpen = PORT_CLOSED;
            return OS_SUCCESS;
        }
        OS_printf("Simulith uart_close failed with status %d\n", sim_status);
    }
    return OS_ERROR;
}