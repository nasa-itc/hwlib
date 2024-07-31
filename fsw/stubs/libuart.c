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

#include "libuart.h"

int32_t uart_init_port(uart_info_t* device)
{
  return UART_SUCCESS;
}

int32_t uart_bytes_available(uart_info_t* device)
{
  return 1;
}

int32_t uart_flush(uart_info_t* device)
{
  return UART_SUCCESS;
}

int32_t uart_read_port(uart_info_t* device, uint8_t data[], const uint32_t numBytes)
{
  return numBytes;
}

int32_t uart_write_port(uart_info_t* device, uint8_t data[], const uint32_t numBytes)
{
  return numBytes;
}

int32_t uart_close_port(uart_info_t* device)
{
  return UART_SUCCESS;
}
