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

#ifndef _EPS_3UA_H_
#define _EPS_3UA_H_

#include "common_types.h"

#define EPS_ADDRESS 0x2B
#define EPS_BCR_COUNT 5
#define EPS_SWITCH_COUNT 10

#define EPS_CMD_COUNT                           28
#define EPS_CMD_GET_BOARD_STATUS                0
#define EPS_CMD_GET_LAST_ERROR                  1
#define EPS_CMD_GET_VERSION                     2
#define EPS_CMD_GET_CHECKSUM                    3
#define EPS_CMD_GET_TELEMETRY                   4
#define EPS_CMD_GET_COM_WATCHDOG_PERIOD         5
#define EPS_CMD_SET_COM_WATCHDOG_PERIOD         6
#define EPS_CMD_RESET_COM_WATCHDOG              7
#define EPS_CMD_GET_NUMBER_OF_BOR               8  //Brown-out Resets
#define EPS_CMD_GET_NUMBER_OF_ASR               9  //Auto Software Resets
#define EPS_CMD_GET_NUMBER_OF_MR                10 //Manual Resets
#define EPS_CMD_GET_NUMBER_OF_CWDR              11 //Comms Watchdog Resets
#define EPS_CMD_SWITCH_ON_ALL_PDM               12
#define EPS_CMD_SWITCH_OFF_ALL_PDM              13
#define EPS_CMD_GET_ACTUAL_STATE_ALL_PDM        14
#define EPS_CMD_GET_EXPECTED_STATE_ALL_PDM      15
#define EPS_CMD_GET_INIT_STATE_ALL_PDM          16
#define EPS_CMD_SET_ALL_PDM_TO_INIT_STATE       17
#define EPS_CMD_SWITCH_PDM_N_ON                 18 
#define EPS_CMD_SWITCH_PDM_N_OFF                19
#define EPS_CMD_SET_PDM_N_INIT_STATE_ON         20
#define EPS_CMD_SET_PDM_N_INIT_STATE_OFF        21
#define EPS_CMD_GET_PDM_N_ACTUAL_STATUS         22
#define EPS_CMD_SET_PDM_N_TIMER_LIMIT           23
#define EPS_CMD_GET_PDM_N_TIMER_LIMIT           24
#define EPS_CMD_GET_PDM_N_CURRENT_TIMER_VAL     25
#define EPS_CMD_PCM_RESET                       26 
#define EPS_CMD_MANUAL_RESET                    27


/**
 * \brief Power conditioning module (PCM) bus type
 */
enum PCMBus
{
    PCM_BUS_BAT = 0,
    PCM_BUS_5V,
    PCM_BUS_3V,
    PCM_BUS_12V,
    NUM_PCM_BUSES
};

#define EPS_ANALOG_CHANNELS 58

/* analog telemetry channel (TLE) type */
typedef enum
{
    CHANNEL_VBCR1            = 0xE110,
    CHANNEL_IBCR1A           = 0xE114,
    CHANNEL_IBCR1B           = 0xE115,
    CHANNEL_TBCR1A           = 0xE118,
    CHANNEL_TBCR1B           = 0xE119,
    
    CHANNEL_VBCR2            = 0xE120,
    CHANNEL_IBCR2A           = 0xE124,
    CHANNEL_IBCR2B           = 0xE125,
    CHANNEL_TBCR2A           = 0xE128,
    CHANNEL_TBCR2B           = 0xE129,
    
    CHANNEL_VBCR3            = 0xE130,
    CHANNEL_IBCR3A           = 0xE134,
    CHANNEL_IBCR3B           = 0xE135,
    CHANNEL_TBCR3A           = 0xE138,
    CHANNEL_TBCR3B           = 0xE139,

    CHANNEL_VBCR4            = 0xE140,
    CHANNEL_IBCR4A           = 0xE144,
    CHANNEL_IBCR4B           = 0xE145,
    CHANNEL_TBCR4A           = 0xE148,
    CHANNEL_TBCR4B           = 0xE149,

    CHANNEL_VBCR5            = 0xE150,
    CHANNEL_IBCR5A           = 0xE154,
    CHANNEL_IBCR5B           = 0xE155,
    CHANNEL_TBCR5A           = 0xE158,
    CHANNEL_TBCR5B           = 0xE159,

    CHANNEL_IIDIODE_OUT      = 0xE284,
    CHANNEL_VIDIODE_OUT      = 0xE280,
    CHANNEL_I3V3_DRW         = 0xE205,
    CHANNEL_I5V_DRW          = 0xE215,
    
    CHANNEL_IPCM12V          = 0xE234,
    CHANNEL_VPCM12V          = 0xE230,
    CHANNEL_IPCMBATV         = 0xE224,
    CHANNEL_VPCMBATV         = 0xE220,
    CHANNEL_IPCM5V           = 0xE214,
    CHANNEL_VPCM5V           = 0xE210,
    CHANNEL_IPCM3V3          = 0xE204,
    CHANNEL_VPCM3V3          = 0xE200,
    
    CHANNEL_VSW1             = 0xE410,
    CHANNEL_ISW1             = 0xE414,
    CHANNEL_VSW2             = 0xE420,
    CHANNEL_ISW2             = 0xE424,
    CHANNEL_VSW3             = 0xE430, //taken from Rev. E
    CHANNEL_ISW3             = 0xE434, //taken from Rev. E
    CHANNEL_VSW4             = 0xE440,
    CHANNEL_ISW4             = 0xE444,
    CHANNEL_VSW5             = 0xE450,
    CHANNEL_ISW5             = 0xE454,
    CHANNEL_VSW6             = 0xE460,
    CHANNEL_ISW6             = 0xE464,
    CHANNEL_VSW7             = 0xE470,
    CHANNEL_ISW7             = 0xE474,
    CHANNEL_VSW8             = 0xE480,
    CHANNEL_ISW8             = 0xE484,
    CHANNEL_VSW9             = 0xE490,
    CHANNEL_ISW9             = 0xE494,
    CHANNEL_VSW10            = 0xE4A0,
    CHANNEL_ISW10            = 0xE4A4,

    CHANNEL_TBRD             = 0xE308

} EPS_channel_t;

#endif
