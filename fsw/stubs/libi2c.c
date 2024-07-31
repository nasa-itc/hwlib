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
    return I2C_SUCCESS;
}

int32_t i2c_master_transaction(i2c_bus_info_t* device, uint8_t addr, void * txbuf, uint8_t txlen, void * rxbuf, uint8_t rxlen, uint16_t timeout)
{
    return I2C_SUCCESS;
}

int32_t i2c_read_transaction(i2c_bus_info_t* device, uint8_t addr, void * rxbuf, uint8_t rxlen, uint8_t timeout)
{
    return I2C_SUCCESS;
}

int32_t i2c_write_transaction(i2c_bus_info_t* device, uint8_t addr, void * txbuf, uint8_t txlen, uint8_t timeout)
{
    return I2C_SUCCESS;
}

int32_t i2c_multiple_transaction(i2c_bus_info_t* device, uint8_t addr, struct i2c_rdwr_ioctl_data* rdwr_data, uint16_t timeout)
{
    return I2C_SUCCESS;
}
