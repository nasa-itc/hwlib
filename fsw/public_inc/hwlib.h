/* Copyright (C) 2009 - 2015 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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


#ifndef _hwlib_h_
#define _hwlib_h_

/* HWLIB Event IDs for Logging */
#define HWLIB_INIT_EID  1
#define HWLIB_GPS_EID   2
#define HWLIB_CAM_EID   3

/************************************************************************
** Includes
*************************************************************************/
 
// OSAL
#include "osapi.h"

// APIs
#include "libcan.h"
#include "libi2c.h"
#include "libmem.h"
#include "libpwm.h"
#include "libspi.h"
#include "libuart.h"

// Components
#include "cam_lib.h"
#include "eps_lib.h"

#endif
