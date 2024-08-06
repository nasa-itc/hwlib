/* Copyright (C) 2009 - 2019 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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

#include "libcan.h"

/* The `libsocketcan` library wraps many of the netlink operations to control a SocketCAN interface
 * Documentation here: https://lalten.github.io/libsocketcan/Documentation/html/group__extern.html */

// Bring CAN network interface
int32_t can_init_dev(can_info_t* device) 
{
  return CAN_SUCCESS;
}

// Call the `libsocketcan` function to set each CAN mode on/off
int32_t can_set_modes(can_info_t* device) 
{
  return CAN_SUCCESS;
}

// Write a can_frame  from `device->tx_Frame` to CAN bus from SocketCAN socket specified by `device`
int32_t can_write(can_info_t* device) 
{  
  return CAN_SUCCESS;
}

// Read a can_frame from SocketCAN interface specified by `device` into `device->rx_frame`
// Does a nonblocking read call
int32_t can_read(can_info_t* device) 
{		
  return CAN_SUCCESS;
}

// Bring CAN network interface down
int32_t can_close_device(can_info_t* device) 
{
  return CAN_SUCCESS;
}

// Perform non-blocking can transaction
int32_t can_master_transaction(can_info_t* device) 
{
  return CAN_SUCCESS;
}
