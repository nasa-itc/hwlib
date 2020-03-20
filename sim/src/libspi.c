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

/* nos */
#include <Spi/Client/CInterface.h>

/* hwlib API */
#include "libspi.h"

/* spi bus mutex */
spi_mutex_t spi_bus_mutex[MAX_SPI_BUSES];
uint32_t handle_count = 0;

/* spi device handles */
static NE_SpiHandle *spi_device[NUM_SPI_DEVICES] = {0};

/* public prototypes */
void nos_init_spi_link(void);
void nos_destroy_spi_link(void);

/* private prototypes */
static NE_SpiHandle* nos_get_spi_device(spi_info_t* device);

/* initialize nos engine spi link */
void nos_init_spi_link(void)
{
    // Do nothing
}

/* destroy nos engine spi link */
void nos_destroy_spi_link(void)
{
    /* clean up spi buses */
    int i;
    for(i = 0; i < NUM_SPI_DEVICES; i++)
    {
        NE_SpiHandle *dev = spi_device[i];
        if(dev) NE_spi_close(&dev);
    }
}

/* nos spi init */
int32 spi_init_dev(spi_info_t* device)
{
    int     status = SPI_SUCCESS;
    char    buffer[16];

    // Initialize the bus mutex
    if (device->bus < MAX_SPI_BUSES)
    {
        if (spi_bus_mutex[device->bus].users == 0)
        {
            snprintf(buffer, 16, "spi_%d_mutex", device->bus);
            status = OS_MutSemCreate(&spi_bus_mutex[device->bus].spi_mutex, buffer, 0);
            if (status != OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SPI_ERR_MUTEX_CREATE, CFE_EVS_ERROR, "HWLIB: Create spi mutex error %d", status);
                return status;
            }
        }
        spi_bus_mutex[device->bus].users++;
    }
    else
    {
        CFE_EVS_SendEvent(SPI_ERR_MUTEX_CREATE, CFE_EVS_ERROR, "HWLIB: Create spi mutex error %d, bus invalid!", status);
        return status;
    }

    if (OS_MutSemTake(spi_bus_mutex[device->bus].spi_mutex) == OS_SUCCESS)
    {
        /* get spi device handle */
        NE_SpiHandle **dev = &spi_device[device->handle];
        if(*dev == NULL)
        {
            /* get nos spi connection params */
            const nos_connection_t *con = &nos_spi_connection[(device->bus * 10) + device->cs];

            /* try to initialize master */
            *dev = NE_spi_init_master3(hub, con->uri, con->bus);
            if(*dev)
            {
                status = SPI_SUCCESS;
            }
            else
            {
                OS_MutSemGive(spi_bus_mutex[device->bus].spi_mutex);
                CFE_EVS_SendEvent(SPI_ERR_FILE_OPEN, CFE_EVS_ERROR, "HWLIB: Open SPI device \"%s\" error %d", device->deviceString, status);
                return status;
            }
        }
    }
    OS_MutSemGive(spi_bus_mutex[device->bus].spi_mutex);

    // Set open flag
    device->isOpen = SPI_DEVICE_OPEN;

    return status;
}

/* get spi device */
static NE_SpiHandle* nos_get_spi_device(spi_info_t* device)
{
    NE_SpiHandle *dev = NULL;
    if(device->handle < NUM_SPI_DEVICES)
    {
        dev = spi_device[device->handle];
        if(dev == NULL)
        {
            spi_init_dev(device);
            dev = spi_device[device->handle];
        }
    }
    return dev;
}

/* nos spi chip select */
int32 spi_select_chip(spi_info_t* device)
{
    uint32_t status = SPI_SUCCESS;

    status = OS_MutSemTake(spi_bus_mutex[device->bus].spi_mutex);

    NE_SpiHandle *dev = nos_get_spi_device(device);
    if(dev)
    {
        NE_spi_select_chip(dev, device->cs);
    }

    return status;
}

/* nos spi chip unselect */
int32 spi_unselect_chip(spi_info_t* device)
{
    uint32_t status = SPI_SUCCESS;

    status = OS_MutSemGive(spi_bus_mutex[device->bus].spi_mutex);

    NE_SpiHandle *dev = nos_get_spi_device(device);
    if(dev)
    {
        NE_spi_unselect_chip(dev);
    }

    return status;
}

/* nos spi write */
int32 spi_write(spi_info_t* device, uint8 data[], const uint32 numBytes)
{
    int status = SPI_SUCCESS;

    NE_SpiHandle *dev = nos_get_spi_device(device);
    if(dev)
    {
        if(NE_spi_write(dev, data, numBytes) != NE_SPI_SUCCESS)
        {
            status = SPI_ERROR;
        }
    }

    return status;
}

/* nos spi read */
int32 spi_read(spi_info_t* device, uint8 data[], const uint32 numBytes)
{
    int status = SPI_SUCCESS;

    NE_SpiHandle *dev = nos_get_spi_device(device);
    if(dev)
    {
        if(NE_spi_read(dev, data, numBytes) != NE_SPI_SUCCESS)
        {
            status = SPI_ERROR;
        }
    }

    return status;
}

int32 spi_transaction(spi_info_t* device, uint8_t *txBuff, uint8_t * rxBuffer, uint32_t length, uint16_t delay, uint8_t bits, uint8_t deselect)
{
    int status = SPI_SUCCESS;

    NE_SpiHandle *dev = nos_get_spi_device(device);
    if(dev)
    {
        if(NE_spi_transaction(dev, txBuff, length, rxBuffer, length) != NE_SPI_SUCCESS)
        {
            status = SPI_ERROR;
        }
    }

    return status;
}
