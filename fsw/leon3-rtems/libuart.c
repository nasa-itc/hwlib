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
#include "mares_regs.h"

int32_t uart_init_port(uart_info_t* device)
{
  int32_t status = UART_SUCCESS;
  int32_t uart_base;
  int32_t config;
  struct  rki_apbuart_regs *uart_regs;

  // Set the access flag.  We default to O_RDWR if the specified access flag is
  // out of range
  int oflag = O_RDWR;

  if (device->access_option == uart_access_flag_RDONLY)
    oflag = O_RDONLY;
  else if (device->access_option == uart_access_flag_WRONLY)
    oflag = O_WRONLY;

  switch (device->handle)
  {
    case 0:
      uart_base = RKI_APBUART_0;
      break;
    case 1:
      uart_base = RKI_APBUART_1;
      break;
    case 2:
      uart_base = RKI_APBUART_2;
      break;
    case 3:
      uart_base = RKI_APBUART_3;
      break;
    case 4:
      uart_base = RKI_APBUART_4;
      break;
    case 5:
      uart_base = RKI_APBUART_5;
      break;
    default:
      status = UART_ERROR;
      break;
  }

  device->handle = open(device->deviceString, oflag);

  if (device->handle >= 0)
  {
    // Set open flag
    device->isOpen = PORT_OPEN;
    
    // Set baud rate
    tcgetattr(device->handle, &device->options);        
    cfsetispeed(&device->options, device->baud);
    cfsetospeed(&device->options, device->baud);

    if(device->baud == 921600)
    { // Enable external clock
      if (status == UART_SUCCESS)
      {  
        uart_regs = (struct rki_apbuart_regs *)uart_base;
        /* Read config */
        config = uart_regs->ctrl;
        /* Set external clock */
        config = config | 0x100;
        /* Write config */
        uart_regs->ctrl = config;
      }
    }

    //Set UART port options from Cudmore's CI_LAB_UART
    device->options.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    device->options.c_cflag &= ~CSIZE;
    device->options.c_cflag |= CS8;         /* 8-bit characters */
    device->options.c_cflag &= ~PARENB;     /* no parity bit */
    device->options.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    device->options.c_cflag &= ~CRTSCTS;     /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    device->options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    device->options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    device->options.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    device->options.c_cc[VMIN] = 0;
    device->options.c_cc[VTIME] = 1;
    //End UART Options

    // Set the options
    tcsetattr(device->handle, TCSANOW, &device->options);
  }
  else
  {
      printf("Oh no!  Open \"%s\" failed and reported: %s \n", device->deviceString, strerror(device->handle));
      device->isOpen = PORT_CLOSED;
      status = OS_ERR_FILE;
  }
  
  return status;
}

int32_t uart_bytes_available(int32_t handle)
{
  int32_t bytes_available = 0;

  ioctl(handle, FIONREAD, &bytes_available);

  return bytes_available;
}

int32_t uart_flush(int32_t handle)
{
  tcflush(handle,TCIOFLUSH);

  return UART_SUCCESS;
}

int32_t uart_read_port(int32_t handle, uint8_t data[], const uint32_t numBytes)
{
  int32_t status = UART_SUCCESS;

  if (data != NULL)
  {
    status = read(handle, data, numBytes);
  }
  else
  {
    status = OS_ERR_FILE;
  }
    
  return status;
}

int32_t uart_write_port(int32_t handle, uint8_t data[], const uint32_t numBytes)
{
  int32_t status = UART_SUCCESS;

  status = write(handle, data, numBytes);

  return status;
}

int32_t uart_close_port(int32_t handle)
{
  int32_t status = UART_SUCCESS;

  if (handle >= 0)
  {
    status = close(handle);
  }
  else
  {
    status = OS_ERR_FILE;
  }
  return status;
}
