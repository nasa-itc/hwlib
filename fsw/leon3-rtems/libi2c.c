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



#include "libi2c.h"

i2c_bus_info_t i2cDevice;

static rtems_status_code
i2c_hwlib_write (rtems_device_major_number major,
                     rtems_device_minor_number minor, void *arg)
{
    rtems_libio_rw_args_t *rwargs = arg;
    int rc;
    int ret_cnt;
  
    if (rwargs->count <= 0)
    {
         return RTEMS_SUCCESSFUL;
    }
    
    ret_cnt = rtems_libi2c_start_write_bytes (minor, (unsigned char *)rwargs->buffer, rwargs->count);
    if (ret_cnt < 0)
    {
         return -ret_cnt;
    }
       
    rc = rtems_libi2c_send_stop (minor);
    if (rc)
    {
         return rc;
    }
    else
    {
         return ret_cnt;
    }
    
}

static rtems_status_code
i2c_hwlib_read (rtems_device_major_number major,
                    rtems_device_minor_number minor, void *arg)
{
    int rc;
    rtems_libio_rw_args_t *rwargs = arg;
    int ret_cnt;
 
    if (rwargs->count <= 0)
    {
         return RTEMS_SUCCESSFUL;
    }
 
    ret_cnt = rtems_libi2c_start_read_bytes(minor, (unsigned char *)rwargs->buffer, rwargs->count);

    if (ret_cnt < 0)
    {
         return -ret_cnt;
    }
  
    rc = rtems_libi2c_send_stop (minor);
  
    if (rc)
    {
         return rc;
    }
    else
    {
         return ret_cnt;
    }
}


/* Configure the interface and do simultaneous READ/WRITE operations */
static rtems_status_code i2c_hwlib_ioctl(rtems_device_major_number major, rtems_device_minor_number minor, void *arg)
{
    struct i2c_rdwr_ioctl_data* rdwr_data;
    rtems_libio_ioctl_args_t *ioarg = arg;
    unsigned int i;
    int status = RTEMS_SUCCESSFUL;
 
    switch (ioarg->command) {
         case I2C_RDWR:

              rdwr_data = (struct i2c_rdwr_ioctl_data *)ioarg->buffer;

              for (i = 0; i < rdwr_data->nmsgs; i++)
              {
                   if (rdwr_data->msgs[i].flags & I2C_M_RD)
                   {
                        status = rtems_libi2c_start_read_bytes(minor, rdwr_data->msgs[i].buf, rdwr_data->msgs[i].len);
                        if (status != rdwr_data->msgs[i].len)
                        {
                            //printf("i2c read FAILED, %s\n",  strerror(errno));
                            return status;
                        }
     
                   }
                   else if (rdwr_data->msgs[i].flags == 0)
                   {
                        status =  rtems_libi2c_start_write_bytes (minor, rdwr_data->msgs[i].buf, rdwr_data->msgs[i].len);
                        if (status != rdwr_data->msgs[i].len)
                        {
                            //printf("i2c write FAILED, %s\n",  strerror(errno));
                            return status;
                        }
                   }
              }

              status = rtems_libi2c_send_stop (minor);
              if (status)
              {
                  return status;
              }

         break;
         default:
             return -ENOTTY;
         break;
    }

  return status;

}

static rtems_driver_address_table i2c_hwlib_ops = {
  .read_entry =  i2c_hwlib_read,
  .write_entry = i2c_hwlib_write,
  .control_entry = i2c_hwlib_ioctl,
};

static rtems_libi2c_drv_t i2c_hwlib_tbl = {
  .ops =         &i2c_hwlib_ops,
  .size =        sizeof (i2c_hwlib_tbl),
};



rtems_libi2c_drv_t *i2c_hwlib_driver_descriptor = &i2c_hwlib_tbl;


/* Call to configure a specific i2c device from /dev
** i2c_bus- struct with bus configuration
** speed  - currently unused
*/
int32_t i2c_master_init(i2c_bus_info_t* device)
{
    int32_t status = I2C_SUCCESS;
 
    char *i2c_dev = "/dev/i2c1.hwlib";
    
    /* Check if device node exists */
    if (access(i2c_dev, W_OK) < 0)
    { 
        printf("Registering I2C driver: ");
        status = rtems_libi2c_register_drv("hwlib", i2c_hwlib_driver_descriptor,
                        0, device->addr);
        if (status < 0)
        {
            printf("ERROR: Could not register I2C driver\n");
            return I2C_ERROR;
        }
    }
    
    device->handle= open(i2c_dev, O_RDWR);
    if (device->handle < 0) 
    {
        printf("i2c bus open failed for addr handle = %d, %s\n", (int)device->addr, strerror(errno));
        status = I2C_ERROR;
        device->isOpen = I2C_CLOSED;
    }
    else
    {
        device->isOpen = I2C_OPEN;
        status = I2C_SUCCESS;
        printf("i2c bus open passed for handle = %d\n", (int)device->addr);
    }
    return status;
}

int32_t i2c_master_transaction(int32_t handle, uint8_t addr, void * txbuf, uint8_t txlen, void * rxbuf, uint8_t rxlen, uint16_t timeout)
{
    int32_t status = I2C_SUCCESS;
    int32_t resp;

 
    /* Perform write if needed */
    if (txlen > 0 )
    {
        resp = write(handle, txbuf, txlen);
        if (resp != txlen)
        {
            printf("i2c-%d write to address 0x%X FAILED, [%u] %s\n", (int)handle, addr, errno, strerror(errno));
            status = I2C_ERROR;
        }
    }
    /* Perform read if needed */
    if (rxlen > 0 )
    {
        resp = read(handle, rxbuf, rxlen);
        if (resp != rxlen)
        {
            printf("i2c-%d read from address 0x%X FAILED, %s\n", (int)handle, addr, strerror(errno));
            status = I2C_ERROR;
        }
    }

    return status;
}

int32_t i2c_read_transaction(int32_t handle, uint8_t addr, void * rxbuf, uint8_t rxlen, uint8_t timeout)
{
    int32_t resp;
    int32_t status = I2C_SUCCESS;

 
    resp = read(handle, rxbuf, rxlen); //<-- ACK would need to go in here
    if (resp != rxlen)
    {
        printf("i2c-%d read from address 0x%X FAILED, %s\n", (int)handle, addr, strerror(errno));
        status = I2C_ERROR;
    }

    return status;
}

int32_t i2c_write_transaction(int32_t handle, uint8_t addr, void * txbuf, uint8_t txlen, uint8_t timeout)
{
    int32_t resp;
    int32_t status = I2C_SUCCESS;

 
    resp = write(handle, txbuf, txlen);
    if (resp != txlen)
    {
        printf("i2c-%d write from address 0x%X FAILED, %s\n", (int)handle, addr, strerror(errno));
        status = I2C_ERROR;
    }

    return status;
}

int32_t i2c_multiple_transaction(int32_t handle, uint8_t addr, struct i2c_rdwr_ioctl_data* rdwr_data, uint16_t timeout)
{
  /* Do combined read/write transaction without stop (simply restarts) in between. */
    int32_t status = I2C_SUCCESS;

    /* Make the ICOTL call */
    if (ioctl(handle, I2C_RDWR, rdwr_data) < 0)
    {
        //printf("i2c-%d multiple transaction error = 0x%X FAILED, %s\n", (int)handle, addr, strerror(errno));
        status = I2C_ERROR;
        return status;
    }

    return status;
}
