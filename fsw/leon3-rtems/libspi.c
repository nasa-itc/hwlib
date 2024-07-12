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

spi_info_t spi_device;
int spi_fd;


/* Notes
** Set the spi_info_t parameters to:
**  Fine Sun Sensor - bus = 2; cs = 1, 2, 3, 4, 5, 6, or 7; spi_mode = 3
** 
**  TLM Controller -  bus = 1; cs = 1; spi_mode = 0
*/
typedef struct {
    uint32_t baudrate;      /* tfr rate, bits per second     */
    bool cpol;              /* tfr clock polarity */
    bool cpha;              /* tfr clock phase */
    bool sb_first;
    uint8_t bits_per_word;
    uint8_t cs;
   } spi_memdrv_param_t;

typedef struct {
    rtems_libi2c_drv_t  libi2c_drv_entry;  /* general i2c/spi params */
    spi_memdrv_param_t  spi_memdrv_param;  /* private parameters     */
  } spi_memdrv_t;

  extern rtems_driver_address_table spi_memdrv_rw_ops;

/*
** Purpose:                                                                  
**  translate given minor device number to param pointer                    
**
** Input Parameters:  
**   minor number of device  
**   ptr to param ptr
**
** Return Value: 
**  0 = ok or error code 
*/                                                     
static rtems_status_code spi_memdrv_minor2param_ptr (rtems_device_minor_number minor, spi_memdrv_param_t **param_ptr)
{
  rtems_status_code   rc = RTEMS_SUCCESSFUL;
  spi_memdrv_t *drv_ptr;

  if (rc == RTEMS_SUCCESSFUL) 
  {
    rc = -rtems_libi2c_ioctl(minor, RTEMS_LIBI2C_IOCTL_GET_DRV_T,&drv_ptr);
  }
  
  if ((rc == RTEMS_SUCCESSFUL) && 
      (drv_ptr->libi2c_drv_entry.size != sizeof(spi_memdrv_t))) 
  {
    rc = RTEMS_INVALID_SIZE;
  }
  
  if (rc == RTEMS_SUCCESSFUL) 
  {
    *param_ptr = &(drv_ptr->spi_memdrv_param);
  }
  return rc;
}

/*
** Purpose:                                                                  
**  write and read a block of data                    
**
** Input Parameters:  
**   minor number of device  
**   major device number
**   ptr to to read/write argument struct
**
** Return Value: 
**  0 = ok or error code 
*/                                                     
rtems_status_code spi_hwdrv_write (rtems_device_major_number major, rtems_device_minor_number minor, void *arg)
{
  rtems_status_code          rc = RTEMS_SUCCESSFUL;
  rtems_libio_rw_args_t *rwargs = arg;
  off_t                     off = rwargs->offset;
  int                       cnt = rwargs->count;
  unsigned char            *buf = (unsigned char *)rwargs->buffer;
  int                bytes_sent = 0;
  int                  curr_cnt;
  int                   ret_cnt = 0;
  int                  cmd_size;
  spi_memdrv_param_t  *mem_param_ptr;
  rtems_libi2c_tfr_mode_t tfr_mode;

  /* Get mem parameters */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    rc = spi_memdrv_minor2param_ptr(minor,&mem_param_ptr);
  }

  /* Check arguments */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    if (cnt <= 0)  
    {
      rc = RTEMS_INVALID_SIZE;
    }
    else if (buf == NULL) 
    {
      rc = RTEMS_INVALID_ADDRESS;
    }
  }

  if (rc == RTEMS_SUCCESSFUL) 
  {
    rc = rtems_libi2c_send_start(minor);
  }
 
  /* Set transfer mode */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    tfr_mode.baudrate = mem_param_ptr->baudrate;
    tfr_mode.bits_per_char = mem_param_ptr->bits_per_word;         
    tfr_mode.lsb_first =     FALSE;    
    tfr_mode.clock_inv =     mem_param_ptr->cpol;    
    tfr_mode.clock_phs =     mem_param_ptr->cpha;    
    rc = -rtems_libi2c_ioctl(minor, RTEMS_LIBI2C_IOCTL_SET_TFRMODE, &tfr_mode);
  }
  
  /* Address device */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    rc = rtems_libi2c_send_addr(minor,TRUE);
  }

  /* Send write data */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    ret_cnt = rtems_libi2c_write_bytes(minor,buf,cnt);
    if (ret_cnt < 0) 
    {
      rc = -ret_cnt;
    }
  }
  
  /* Terminate transfer */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    rc = rtems_libi2c_send_stop(minor);
  }
  if (rc == RTEMS_SUCCESSFUL)
  {
      rc = ret_cnt;
  }

  return rc;
}

/*
** Purpose:                                                                  
**  write and read a block of data                    
**
** Input Parameters:  
**   minor number of device  
**   major device number
**   ptr to to read/write argument struct
**
** Return Value: 
**  0 = ok or error code 
*/                                                     
rtems_status_code spi_hwdrv_read (rtems_device_major_number major, rtems_device_minor_number minor, void *arg)
{
  rtems_status_code rc = RTEMS_SUCCESSFUL;
  rtems_libio_rw_args_t *rwargs = arg;
  off_t                     off = rwargs->offset;
  int                       cnt = rwargs->count;
  unsigned char            *buf = (unsigned char *)rwargs->buffer;
  int                   ret_cnt = 0;
  int                  cmd_size;
  spi_memdrv_param_t  *mem_param_ptr;
  rtems_libi2c_tfr_mode_t tfr_mode;
  rtems_libi2c_read_write_t *rw;
 
  if (rc == RTEMS_SUCCESSFUL) 
  {
    rc =  spi_memdrv_minor2param_ptr(minor,&mem_param_ptr);
  }

  /* Check arguments */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    if (cnt <= 0)
    {
      rc = RTEMS_INVALID_SIZE;
    }
    else if (buf == NULL) 
    {
      rc = RTEMS_INVALID_ADDRESS;
    }
  }
  rw = (rtems_libi2c_read_write_t *)buf;

  /* Select device, set transfer mode, address device */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    rc = rtems_libi2c_send_start(minor);
  }
  
  /* Set transfer mode */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    tfr_mode.baudrate = mem_param_ptr->baudrate;
    tfr_mode.bits_per_char = mem_param_ptr->bits_per_word;         
    tfr_mode.lsb_first =     FALSE;    
    tfr_mode.clock_inv =     mem_param_ptr->cpol;    
    tfr_mode.clock_phs =     mem_param_ptr->cpha;    
    rc = -rtems_libi2c_ioctl(minor, RTEMS_LIBI2C_IOCTL_SET_TFRMODE, &tfr_mode);
  }

  /* Address device */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    rc = rtems_libi2c_send_addr(minor,TRUE);
  }

  /* Fetch read data */
  ret_cnt = rtems_libi2c_ioctl(minor, RTEMS_LIBI2C_IOCTL_READ_WRITE, rw);
  if (ret_cnt < 0) 
  {
    rc = -ret_cnt;
  }

  /* Terminate transfer */
  if (rc == RTEMS_SUCCESSFUL) 
  {
    rc = rtems_libi2c_send_stop(minor);
  }

  if (rc == RTEMS_SUCCESSFUL)
  {
    rc = ret_cnt;
  }

  return rc;
}

/*
 * driver operation tables
 */
rtems_driver_address_table spi_hwdrv_rw_ops_0 = { 
  .read_entry =  spi_hwdrv_read,
  .write_entry = spi_hwdrv_write
};
rtems_driver_address_table spi_hwdrv_rw_ops_1 = { 
  .read_entry =  spi_hwdrv_read,
  .write_entry = spi_hwdrv_write
};
rtems_driver_address_table spi_hwdrv_rw_ops_2 = { 
  .read_entry =  spi_hwdrv_read,
  .write_entry = spi_hwdrv_write
};

/* Tell SSSSPI driver about how the Flash is set up */
spi_memdrv_t spi_hw_drv[3] = {
{
  {/* public fields */
  .ops =         &spi_hwdrv_rw_ops_0, /* operations of general memdrv */
  .size =        sizeof (spi_hw_drv[0]),
  },
  { /* our private fields */
  .baudrate =             115000,
  }
},
{
  {/* public fields */
  .ops =         &spi_hwdrv_rw_ops_1, /* operations of general memdrv */
  .size =        sizeof (spi_hw_drv[1]),
  },
  { /* our private fields */
  .baudrate =             115000,
  }
},
{
  {/* public fields */
  .ops =         &spi_hwdrv_rw_ops_2, /* operations of general memdrv */
  .size =        sizeof (spi_hw_drv[2]),
  },
  { /* our private fields */
  .baudrate =             115000,
  }
}
};

rtems_libi2c_drv_t *spi_hw_drv_desc[3] = {&spi_hw_drv[0].libi2c_drv_entry,
                                          &spi_hw_drv[1].libi2c_drv_entry,
                                          &spi_hw_drv[2].libi2c_drv_entry};

int32_t spi_init_dev(spi_info_t* device)
{
  int status = SPI_SUCCESS;
  char    csbuffer[4];
  char    buffer[16];
  char    devbuffer[20];

  // Initialize the bus mutex
  if (device->bus < MAX_SPI_BUSES)
  {
    if (spi_bus_mutex[device->bus].users == 0)
    {
      snprintf(buffer, 16, "spi_%d_mutex", device->bus);
      status = OS_MutSemCreate(&spi_bus_mutex[device->bus].spi_mutex, buffer, 0);
      if (status != OS_SUCCESS)
      {
        printf("HWLIB: Create spi mutex error %d", status);
        return status;
      }
    }
    spi_bus_mutex[device->bus].users++;
  }
  else  /* Create node for 'raw' access to PSE device */
   {
    printf("HWLIB: Create spi mutex error %d, bus invalid!", status);
    return status;
  }

  if (OS_MutSemTake(spi_bus_mutex[device->bus].spi_mutex) == OS_SUCCESS)
  {
    /* The driver has initialized the i2c library for us */
    spi_hw_drv[device->bus].spi_memdrv_param.baudrate = device->baudrate;
    spi_hw_drv[device->bus].spi_memdrv_param.sb_first = FALSE;
    spi_hw_drv[device->bus].spi_memdrv_param.bits_per_word = device->bits_per_word;
    spi_hw_drv[device->bus].spi_memdrv_param.cs = device->cs;

    // Set the mode 
    status = spi_set_mode(device);
    if(status != SPI_SUCCESS)
    {
      return status;
    }

    /* Check if device node exists */
    snprintf(csbuffer, 4, "%d", device->cs);
    snprintf(devbuffer, 20, "/dev/spi%d.%d", device->bus, device->cs);
    if (access(devbuffer, W_OK) < 0)
    { 
      printf("Registering SPI driver: ");
      status = rtems_libi2c_register_drv(csbuffer, spi_hw_drv_desc[device->bus], device->bus, device->cs); 
      if (status < 0) 
      {
        printf("ERROR: Could not register SPI driver\n");
        device->isOpen = SPI_DEVICE_CLOSED;
        return SPI_ERR_FILE_OPEN;
      }
    }

    device->handle = open(devbuffer, O_RDWR); 
    if (device->handle < 0)
    {
      device->isOpen = SPI_DEVICE_CLOSED;
      status = SPI_ERR_FILE_OPEN;
      OS_MutSemGive(spi_bus_mutex[device->bus].spi_mutex);
      printf("HWLIB: Open SPI device \"%s\" error %d", device->deviceString, status);
      return status;
    }
    else
    {
      // Set open flag
      device->isOpen = SPI_DEVICE_OPEN;
      status = SPI_SUCCESS;
    }
  }
  OS_MutSemGive(spi_bus_mutex[device->bus].spi_mutex);

  return status;
}

int32_t spi_set_mode(spi_info_t* device)
{
  uint8_t mode; 
  int32_t ret;
  int status = SPI_SUCCESS; 

  // Set mode
  switch(device->spi_mode)
  {
    case 0:
      spi_hw_drv[device->bus].spi_memdrv_param.cpol = 0;              
      spi_hw_drv[device->bus].spi_memdrv_param.cpha = 0;             
      break;
    case 1:
      spi_hw_drv[device->bus].spi_memdrv_param.cpol = 0;              
      spi_hw_drv[device->bus].spi_memdrv_param.cpha = 1;             
      break;
    case 2:
      spi_hw_drv[device->bus].spi_memdrv_param.cpol = 1;              
      spi_hw_drv[device->bus].spi_memdrv_param.cpha = 0;             
      break;
    case 3:
      spi_hw_drv[device->bus].spi_memdrv_param.cpol = 1;              
      spi_hw_drv[device->bus].spi_memdrv_param.cpha = 1;             
      break;
    default: 
      status = SPI_ERR_INVAL_MD;
      printf("HWLIB: Invalid spi mode error %d", status);
      return status;
  }
   
  return status;
}

int32_t spi_get_mode(spi_info_t* device)
{
  int32_t  ret;
  int      status = SPI_SUCCESS; 
  uint8_t  spi_mode = 0;
  uint8_t  bits_per_word = 0;
  uint32_t baudrate = 0;
  
  if (OS_MutSemTake(spi_bus_mutex[device->bus].spi_mutex) == OS_SUCCESS)
  {
  }
  OS_MutSemGive(spi_bus_mutex[device->bus].spi_mutex);

  device->spi_mode = spi_mode;
  device->bits_per_word = bits_per_word;
  device->baudrate = baudrate;

  return status;
}

int32_t spi_write(spi_info_t* device, uint8_t data[], const uint32_t numBytes)
{
  int32_t status = SPI_SUCCESS;

  status = spi_transaction(device, data, NULL, numBytes, 0, device->bits_per_word * numBytes, 0);

  return status;     
}

int32_t spi_read(spi_info_t* device, uint8_t data[], const uint32_t numBytes)
{
  int status = SPI_SUCCESS;

  status = spi_transaction(device, NULL, data, numBytes, 0, device->bits_per_word * numBytes, 0);

  return status;  
}

int32_t spi_transaction(spi_info_t* device, uint8_t *txBuff, uint8_t * rxBuffer, uint32_t length, uint16_t delay, uint8_t bits, uint8_t deselect)
{
  int status = SPI_SUCCESS;
  int ret;
  rtems_libi2c_read_write_t xfer;
  
  // Clear the xfer struct
  memset((void *)&xfer, 0, sizeof(rtems_libi2c_read_write_t));

  // Setup the transfer structure for the transaction 
  xfer.rd_buf = rxBuffer;
  xfer.wr_buf = txBuff;
  xfer.byte_cnt = length;
  
  // Perform a full duplex transaction
  ret = read (device->handle, &xfer, sizeof(rtems_libi2c_read_write_t));
  if (ret != -1)
  {
    status = SPI_ERR_IOC_MSG;
  }
  
  return status;
}

int32_t spi_select_chip(spi_info_t* device)
{
  int status = SPI_SUCCESS;

  /* 
  * See "/usr/share/doc/linux-doc/spi/spidev":
  * "From userspace, you can't currently change the chip select polarity;
  *  that could corrupt transfers to other devices sharing the SPI bus.
  *  Each SPI device is deselected when it's not in active use, allowing
  *  other drivers to talk to other devices."
  */
  status = OS_MutSemTake(spi_bus_mutex[device->bus].spi_mutex);
  
  return status;
}

int32_t spi_unselect_chip(spi_info_t* device)
{
  int status = SPI_SUCCESS;

  /* 
  * See "/usr/share/doc/linux-doc/spi/spidev":
  * "From userspace, you can't currently change the chip select polarity;
  *  that could corrupt transfers to other devices sharing the SPI bus.
  *  Each SPI device is deselected when it's not in active use, allowing
  *  other drivers to talk to other devices."
  */
  status = OS_MutSemGive(spi_bus_mutex[device->bus].spi_mutex);
  
  return status;
}

int32_t spi_close_device(spi_info_t* device)
{

  int status = SPI_SUCCESS;

  // Check for valid handle
  if (device->handle >= 0)
  {
    spi_bus_mutex[device->bus].users--;
    if (spi_bus_mutex[device->bus].users == 0)
    {
      OS_MutSemDelete(spi_bus_mutex[device->bus].spi_mutex);
    }

    status = close(device->handle);
    if (status != SPI_SUCCESS) 
    {
      status = SPI_ERR_FILE_CLOSE;
      return status;
    }
    // Set open flag
    device->isOpen = SPI_DEVICE_CLOSED;
  }
  else
  {
    status = SPI_ERR_FILE_HANDLE;
    return status;
  }

  return status;
}
