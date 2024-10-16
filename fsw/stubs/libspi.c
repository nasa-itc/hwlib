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

#include "libspi.h"

spi_mutex_t spi_bus_mutex[MAX_SPI_BUSES];

int32_t spi_init_dev(spi_info_t* device)
{
  return SPI_SUCCESS;
}

int32_t spi_set_mode(spi_info_t* device)
{
  return SPI_SUCCESS;
}

int32_t spi_get_mode(spi_info_t* device)
{
  return SPI_SUCCESS;
}

int32_t spi_write(spi_info_t* device, uint8_t data[], const uint32_t numBytes)
{
  return SPI_SUCCESS;  
}

int32_t spi_read(spi_info_t* device, uint8_t data[], const uint32_t numBytes)
{
  return SPI_SUCCESS; 
}

int32_t spi_transaction(spi_info_t* device, uint8_t *txBuff, uint8_t * rxBuffer, uint32_t length, uint16_t delay, uint8_t bits, uint8_t deselect)
{
  return SPI_SUCCESS;
}

int32_t spi_select_chip(spi_info_t* device)
{
  return SPI_SUCCESS;
}

int32_t spi_close_device(spi_info_t* device)
{
  return SPI_SUCCESS;
}
