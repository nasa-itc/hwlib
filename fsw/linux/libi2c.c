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

#include <errno.h>
#include <string.h>

#include "libi2c.h"

/* Call to configure a specific i2c device from /dev
** i2c_bus- struct with bus configuration
** speed  - currently unused
*/
int32_t i2c_master_init(i2c_bus_info_t* device)
{
    char devname[20];
    int32_t status = I2C_SUCCESS;

    snprintf(devname, 19, "/dev/i2c-%d", device->handle);
    device->handle = open(devname, O_RDWR);
    if (device->handle < 0)
    {
        printf("i2c bus open failed for handle = %d, %s\n", device->handle, strerror(errno));
        status = I2C_ERROR;
        device->isOpen = I2C_CLOSED;
    }
    else
    {
        device->isOpen = I2C_OPEN;
        printf("i2c bus open passed for handle = %d\n", device->handle);
    }
    return status;
}

int32_t i2c_master_transaction(i2c_bus_info_t* device, uint8_t addr, void * txbuf, uint8_t txlen, void * rxbuf, uint8_t rxlen, uint16_t timeout)
{
    int32_t status = I2C_SUCCESS;
    int32_t resp;

    /* Set I2C slave address */
    if (ioctl(device->handle, I2C_SLAVE, addr) < 0)
    {
        printf("i2c-%d setting slave address = 0x%X FAILED, %s\n", device->handle, addr, strerror(errno));
        status = I2C_ERROR;
        return status;
    }

    /* Perform write if needed */
    if (txlen > 0 )
    {
        resp = write(device->handle, txbuf, txlen);
        if (resp != txlen)
        {
            printf("i2c-%d write to address 0x%X FAILED, [%u] %s\n", device->handle, addr, errno, strerror(errno));
            status = I2C_ERROR;
        }
    }
    /* Perform read if needed */
    if (rxlen > 0 )
    {
        resp = read(device->handle, rxbuf, rxlen);
        if (resp != rxlen)
        {
            printf("i2c-%d read from address 0x%X FAILED, %s\n", device->handle, addr, strerror(errno));
            status = I2C_ERROR;
        }
    }

    return status;
}

int32_t i2c_read_transaction(i2c_bus_info_t* device, uint8_t addr, void * rxbuf, uint8_t rxlen, uint8_t timeout)
{
    int32_t resp;
    int32_t status = I2C_SUCCESS;

    /* Set I2C slave address */
    if (ioctl(device->handle, I2C_SLAVE, addr) < 0)
    {
        printf("i2c-%d setting slave address = 0x%X FAILED, %s\n", device->handle, addr, strerror(errno));
        status = I2C_ERROR;
        return status;
    }

    resp = read(device->handle, rxbuf, rxlen); //<-- ACK would need to go in here
    if (resp != rxlen)
    {
        printf("i2c-%d read from address 0x%X FAILED, %s\n", device->handle, addr, strerror(errno));
        status = I2C_ERROR;
    }

    return status;
}

int32_t i2c_write_transaction(i2c_bus_info_t* device, uint8_t addr, void * txbuf, uint8_t txlen, uint8_t timeout)
{
    int32_t resp;
    int32_t status = I2C_SUCCESS;

    /* Set I2C slave address */
    if (ioctl(device->handle, I2C_SLAVE, addr) < 0)
    {
        printf("i2c-%d setting slave address = 0x%X FAILED, %s\n", device->handle, addr, strerror(errno));
        status = I2C_ERROR;
        return status;
    }

    resp = write(device->handle, txbuf, txlen);
    if (resp != txlen)
    {
        printf("i2c-%d write from address 0x%X FAILED, %s\n", device->handle, addr, strerror(errno));
        status = I2C_ERROR;
    }

    return status;
}

int32_t i2c_multiple_transaction(i2c_bus_info_t* device, uint8_t addr, struct i2c_rdwr_ioctl_data* rdwr_data, uint16_t timeout)
{
    /* Do combined read/write transaction without stop (simply restarts) in between. */
    int32_t status = I2C_SUCCESS;

    /* Set I2C slave address */
    if (ioctl(device->handle, I2C_SLAVE, addr) < 0)
    {
        printf("i2c-%d setting slave address = 0x%X FAILED, %s\n", device->handle, addr, strerror(errno));
        status = I2C_ERROR;
        return status;
    }

    /* Make the ICOTL call */
    if (ioctl(device->handle, I2C_RDWR, rdwr_data) < 0)
    {
        printf("i2c-%d multple transaction error = 0x%X FAILED, %s\n", device->handle, addr, strerror(errno));
        status = I2C_ERROR;
        return status;
    }

    return status;
}
