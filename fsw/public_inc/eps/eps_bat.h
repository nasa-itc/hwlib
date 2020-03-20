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

#ifndef _EPS_BAT_H_
#define _EPS_BAT_H_

#define BAT_COUNT   2
#define BAT_CS05602 0 //0x2D
#define BAT_CS05876 1 //0x2A
#define BAT_CS05601 2 //0x2C spare (ETU)

#define BAT_DAUGHTERBOARD_COUNT 4

#define BAT_CMD_COUNT                         11
#define BAT_CMD_GET_BOARD_STATUS              0
#define BAT_CMD_GET_LAST_ERROR                1
#define BAT_CMD_GET_VERSION                   2
#define BAT_CMD_GET_CHECKSUM                  3
#define BAT_CMD_GET_TELEMETRY                 4
#define BAT_CMD_GET_NUMBER_OF_BOR             5
#define BAT_CMD_GET_NUMBER_OF_ASR             6
#define BAT_CMD_GET_NUMBER_OF_MR              7
#define BAT_CMD_GET_HEATER_CONTROLLER_STATUS  8
#define BAT_CMD_SET_HEATER_CONTROLLER_STATUS  9
#define BAT_CMD_MANUAL_RESET                  10

#define BAT_ANALOG_CHANNELS 16

typedef enum
{
    TLM_VBAT = 0,
    TLM_IBAT,
    TLM_IDIRBAT,
    TLM_TBRD,
    TLM_IPCM5V,
    TLM_VPCM5V,
    TLM_IPCM3V3,
    TLM_VPCM3V3,
    TLM_TBAT1,
    TLM_HBAT1,
    TLM_TBAT2,
    TLM_HBAT2,
    TLM_TBAT3,
    TLM_HBAT3,
    TLM_TBAT4,
    TLM_HBAT4
   
} BAT_channel_t;



#endif
