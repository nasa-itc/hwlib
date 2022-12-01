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

/************************************************************************
** Includes
*************************************************************************/
#include "libcan.h"
#include "libi2c.h"
#include "libmem.h"
#include "libtrq.h"
#include "libspi.h"
#include "libuart.h"
#include "libgpio.h"
#include "libsocket.h"

/************************************************************************
** Outside of cFS build
*************************************************************************/
#ifndef OS_SUCCESS
    // Building Outside of cFS
    #define OS_PACK
    #define OS_SUCCESS          0
    #define OS_ERROR           -1
    #define OS_ERR_FILE        -2
#endif

/************************************************************************
** CFE_IS_LITTLE16/32
** Note: Use to specify a value is a 16bit or 32 bit little endian value
*************************************************************************/
#ifdef SOFTWARE_BIG_BIT_ORDER
  #define CFE_IS_LITTLE16(n) ( (((n) << 8) & 0xFF00) | (((n) >> 8) & 0x00FF) )
  #define CFE_IS_LITTLE32(n) ( (((n) << 24) & 0xFF000000) | (((n) << 8) & 0x00FF0000) | (((n) >> 8) & 0x0000FF00) | (((n) >> 24) & 0x000000FF) )
#else
  #define CFE_IS_LITTLE16(n) (n)
  #define CFE_IS_LITTLE32(n) (n)
#endif

/************************************************************************
** CFE_IS_BIG16/32
** Note: Use to specify a value is a 16bit or 32 bit big endian value
*************************************************************************/
#define CFE_IS_BIG16(n) CFE_MAKE_BIG16(n)
#define CFE_IS_BIG32(n) CFE_MAKE_BIG32(n)

#endif
