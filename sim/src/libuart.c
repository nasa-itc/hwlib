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

/* init usart */
int32_t uart_init_port(uart_info_t* device)
{
    int32_t status = UART_SUCCESS;

    /* Initialize the UART port with Simulith */
    status = simulith_uart_init(device->handle, &(device->handle));
    if(status == SIMULITH_UART_SUCCESS)
    {
        device->isOpen = PORT_OPEN;
    }
    else
    {
        OS_printf("HWLIB: simulith_uart_init failed with status %d\n", status);
        device->isOpen = PORT_CLOSED;
        status = UART_ERROR;
    }
    return status;
}

/* usart flush */
int32_t uart_flush(uart_info_t* device)
{
    simulith_uart_flush(device->handle);
    return UART_SUCCESS;
}

/* usart write */
int32_t uart_write_port(uart_info_t* device, uint8_t data[], const uint32_t numBytes)
{
    int32_t status = UART_SUCCESS;

    status = simulith_uart_send(device->handle, data, numBytes);
    if((uint32_t) status != numBytes)
    {
        OS_printf("HWLIB: simulith_uart_send failed with status %d\n", status);
        status = UART_ERROR;
    }
    return status;
}

/* usart read */
int32_t uart_read_port(uart_info_t* device, uint8_t data[], const uint32_t numBytes)
{
    int32_t status = UART_SUCCESS;

    status = simulith_uart_receive(device->handle, data, numBytes);
    if(status < 0)
    {
        OS_printf("HWLIB: simulith_uart_receive failed with status %d\n", status);
    }
    return status;
}

/* usart number bytes available */
int32_t uart_bytes_available(uart_info_t* device)
{
    int32_t status = UART_SUCCESS;

    status = simulith_uart_available(device->handle);
    if(status < 0)
    {
        OS_printf("HWLIB: simulith_uart_available failed with status %d\n", status);
    }
    return status;
}

int32_t uart_close_port(uart_info_t* device) 
{
    int32_t status = UART_SUCCESS;

    status = simulith_uart_close(device->handle);
    if(status < 0)
    {
        OS_printf("HWLIB: simulith_uart_close failed with status %d\n", status);
    }
    return status;
}
