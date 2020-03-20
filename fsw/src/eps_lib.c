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


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include "cfe_evs.h"
#include "osapi.h"
#include "common_types.h"
#include "eps/eps_3ua.h"
#include "eps_lib.h"
#include "libi2c.h"

#define EPS_I2C_HANDLE 0
#define EPS_I2C_TIMEOUT 100 //ms

uint32 eps_i2c_transaction_mutex;

static const uint8 EPS_COMMANDS[EPS_CMD_COUNT][2] = {
    //Return values are calibrated to match EPS 
    //testing CS05495 on 2016/05/11
    //cmd  //return size
    {0x01,      4},  // EPS_CMD_GET_BOARD_STATUS
    {0x03,      4},  // EPS_CMD_GET_LAST_ERROR
    {0x04,      4},  // EPS_CMD_GET_VERSION
    {0x05,      4},  // EPS_CMD_GET_CHECKSUM
    {0x10,      2},  // EPS_CMD_GET_TELEMETRY
    {0x20,      2},  // EPS_CMD_GET_COM_WATCHDOG_PERIOD
    {0x21,      0},  // EPS_CMD_SET_COM_WATCHDOG_PERIOD
    {0x22,      0},  // EPS_CMD_RESET_COM_WATCHDOG
    {0x31,      4},  // EPS_CMD_GET_NUMBER_OF_BOR
    {0x32,      4},  // EPS_CMD_GET_NUMBER_OF_ASRx
    {0x33,      4},  // EPS_CMD_GET_NUMBER_OF_MR
    {0x34,      2},  // EPS_CMD_GET_NUMBER_OF_CWDR
    {0x40,      0},  // EPS_CMD_SWITCH_ON_ALL_PDM
    {0x41,      0},  // EPS_CMD_SWITCH_OFF_ALL_PDM
    {0x42,      4},  // EPS_CMD_GET_ACTUAL_STATE_ALL_PDM
    {0x43,      4},  // EPS_CMD_GET_EXPECTED_STATE_ALL_PDM
    {0x44,      4},  // EPS_CMD_GET_INIT_STATE_ALL_PDM
    {0x45,      0},  // EPS_CMD_SET_ALL_PDM_TO_INIT_STATE
    {0x50,      0},  // EPS_CMD_SWITCH_PDM_N_ON
    {0x51,      0},  // EPS_CMD_SWITCH_PDM_N_OFF
    {0x52,      0},  // EPS_CMD_SET_PDM_N_INIT_STATE_ON
    {0x53,      0},  // EPS_CMD_SET_PDM_N_INIT_STATE_OFF
    {0x54,      2},  // EPS_CMD_GET_PDM_N_ACTUAL_STATUS
    {0x60,      0},  // EPS_CMD_SET_PDM_N_TIMER_LIMIT
    {0x61,      2},  // EPS_CMD_GET_PDM_N_TIMER_LIMIT 
    {0x62,      2},  // EPS_CMD_GET_PDM_N_CURRENT_TIMER_VAL
    {0x70,      0},  // EPS_CMD_PCM_RESET
    {0x80,      0}   // EPS_CMD_MANUAL_RESET   
};


float EPS_telemetry_conversion(uint16 chan, uint16 read_value);

static int EPS_i2c_transaction(int handle, uint8_t addr, void *txbuf, uint8_t txlen, void *rxbuf, uint8_t rxlen, uint16_t timeout, uint8 delay);

int32 EPS_LibInit(void)
{

    int status;

    if ( (status = OS_MutSemCreate(&eps_i2c_transaction_mutex, "EPS_I2C_MUTEX", 0)) != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(EPSLIB_MUTEX_ERR_EID, CFE_EVS_ERROR, 
                          "EPS LIB: Error - failed to create i2c transaction mutex");
    }

    return status; 
}

int32 EPS_get_last_error(uint8 channel)
{
    uint8 message[2];
    uint32 error;
    uint16 motherboard_error;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_LAST_ERROR][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &error,
                             EPS_COMMANDS[EPS_CMD_GET_LAST_ERROR][1], EPS_I2C_TIMEOUT, 2)) != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - EPS_get_last_error: I2C error");
        rval = OS_ERROR;
    }
    else
    {
        motherboard_error = (uint16)(error >> 16);
        switch(motherboard_error)
        {
        case 0x10:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: EPS command 0x%02X error: CRC code does not match data", channel);
            break;
        case 0x01:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: command 0x%02X error: Unknown command received", channel);
            break;
        case 0x02:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB command 0x%02X error: Supplied data incorrect when processing command", channel);
            break;
        case 0x03:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB command 0x%02X error: Selected channel does not exist", channel);
            break;
        case 0x04:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB command 0x%02X error: Selected channel is currently inactive", channel);
            break;
        case 0x13:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB command 0x%02X error: A reset had to occur", channel);
            break;
        case 0x14:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB command 0x%02X error: There was an error with the ADC acquisition", channel);
            break;
        case 0x20:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB command 0x%02X error: Reading from EEPROM generated an error", channel);
            break;
        case 0x30:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB command 0x%02X error: Error on the internal SPI bus", channel);
            break;
        default:
            CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB command 0x%02X error: Undefined error occured", channel);
        }
    }

    return rval;
}

int32 EPS_get_board_status(uint8 *status)
{
    uint8 message[2];
    uint32 response;
    uint8 motherboard_status;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_BOARD_STATUS][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response,
                             EPS_COMMANDS[EPS_CMD_GET_BOARD_STATUS][1], EPS_I2C_TIMEOUT, 2)) != OS_SUCCESS)
    {
        //override status in the
        //event an error occurs
        *status = 0xFF;
        rval = OS_ERROR; 
    }
    else
    {
        motherboard_status = (uint8)((response & 0x007F0000) >> 16);
        if ((response >> 16) != 0xFFFF)
        {
            *status = motherboard_status;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_BOARD_STATUS);
            //override status in the
            //event an error occurs
            *status = 0xFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 EPS_get_version(uint16 *data_output)
{
    uint8 message[2];
    uint32 response;
    uint16 motherboard_version;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_VERSION][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response,
                             EPS_COMMANDS[EPS_CMD_GET_VERSION][1], EPS_I2C_TIMEOUT, 2)) != OS_SUCCESS)
    {
        //override version in the 
        //event an error occurs
        *data_output = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    {
        motherboard_version = (uint16)(response >> 16);
        if (motherboard_version != 0xFFFF)
        {
            *data_output = motherboard_version;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_VERSION);
            //override version in the 
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 EPS_get_checksum(uint16 *data_output)
{
    uint8 message[2];
    uint32 response;
    uint16 motherboard_checksum;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_CHECKSUM][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response,
                             EPS_COMMANDS[EPS_CMD_GET_CHECKSUM][1], EPS_I2C_TIMEOUT, 70)) != OS_SUCCESS)
    {
        //override checksum in the
        //event an error occurs
        *data_output = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    {
        motherboard_checksum = (uint16)(response >> 16);
        if (motherboard_checksum != 0xFFFF)
        {
            *data_output = motherboard_checksum;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_CHECKSUM);
            //override checksum in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 EPS_get_telemetry(uint16 channel, float *data_output)
{

    uint16 response;
    uint8 message[3];
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_TELEMETRY][0];
    message[1] = channel >> 8;
    message[2] = channel & 0xFF;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 3, &response, 
                             EPS_COMMANDS[EPS_CMD_GET_TELEMETRY][1], EPS_I2C_TIMEOUT, 15)) != OS_SUCCESS)
    {
        if (channel != CHANNEL_VIDIODE_OUT)
            *data_output = 0.0;
        rval = OS_ERROR;
    }
    else
    {
        if (response != 0xFFFF)
        {
            *data_output = EPS_telemetry_conversion(channel, response);
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_TELEMETRY);
            if (channel != CHANNEL_VIDIODE_OUT)
                *data_output = 0.0;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 EPS_get_comms_watchdog_period(uint16 *data_output)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_COM_WATCHDOG_PERIOD][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response, 
                             EPS_COMMANDS[EPS_CMD_GET_COM_WATCHDOG_PERIOD][1], EPS_I2C_TIMEOUT, 1)) != OS_SUCCESS)
    {
        //override period in the 
        //event an error occurs
        *data_output = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    {
        if (response != 0xFFFF)
        {
            *data_output = response;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_COM_WATCHDOG_PERIOD);
            //override period in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        } 
    }

    return rval;
}

//TODO this value will reset back to default 4 minutes
//if a reset occurs. Need to set this in app init if 
//a different value is needed.
int32 EPS_set_comms_watchdog_period(uint8 period)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    if (period < 1 || period > 90) //minutes
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - watchdog period out of range");
        rval = OS_ERROR;
    }
    else
    {
        message[0] = EPS_COMMANDS[EPS_CMD_SET_COM_WATCHDOG_PERIOD][0];
        message[1] = period;

        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL, 
                                 EPS_COMMANDS[EPS_CMD_SET_COM_WATCHDOG_PERIOD][1], EPS_I2C_TIMEOUT, 0)) != OS_SUCCESS)
        {
            rval = OS_ERROR;
        }
    }

    return rval;
}

//TODO this commands does not return bytes, but will it 
//return 0xFFFF if an error occurs?
int32 EPS_reset_comms_watchdog(void)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_RESET_COM_WATCHDOG][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL, 
                             EPS_COMMANDS[EPS_CMD_RESET_COM_WATCHDOG][1], EPS_I2C_TIMEOUT, 0)) != OS_SUCCESS)
    {
        rval = OS_ERROR;
    }
 
    return rval;
}

int32 EPS_get_num_brownout_resets(uint16 *data_output)
{
    uint8 message[2];
    uint32 response;
    uint16 motherboard_bor;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_NUMBER_OF_BOR][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response, 
                             EPS_COMMANDS[EPS_CMD_GET_NUMBER_OF_BOR][1], EPS_I2C_TIMEOUT, 2)) != OS_SUCCESS)
    {
        //override resets in the
        //event an error occurs
        *data_output = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    { 
        motherboard_bor = (uint16)(response >> 16);
        if (motherboard_bor != 0xFFFF)
        {
            *data_output = motherboard_bor;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_NUMBER_OF_BOR);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 EPS_get_num_auto_software_resets(uint16 *data_output)
{
    uint8 message[2];
    uint32 response;
    uint16 motherboard_asr;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_NUMBER_OF_ASR][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response, 
                             EPS_COMMANDS[EPS_CMD_GET_NUMBER_OF_ASR][1], EPS_I2C_TIMEOUT, 2)) != OS_SUCCESS)
    {
        //override resets in the
        //event an error occurs
        *data_output = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    { 
        motherboard_asr = (uint16)(response >> 16);
        if (motherboard_asr != 0xFFFF)
        {
            *data_output = motherboard_asr;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_NUMBER_OF_ASR);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 EPS_get_num_manual_resets(uint16 *data_output)
{
    uint8 message[2];
    uint32 response;
    uint16 motherboard_manual_resets;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_NUMBER_OF_MR][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response, 
                             EPS_COMMANDS[EPS_CMD_GET_NUMBER_OF_MR][1], EPS_I2C_TIMEOUT, 2)) != OS_SUCCESS)
    {
        //override resets in the
        //event an error occurs
        *data_output = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    {
        motherboard_manual_resets = (uint16)(response >> 16);
        if ((response >> 16) != 0xFFFF)
        {
            *data_output = motherboard_manual_resets;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_NUMBER_OF_MR);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
    }
 
    return rval;
}

int32 EPS_get_num_comms_watchdog_resets(uint16 *data_output)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_NUMBER_OF_CWDR][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response, 
                             EPS_COMMANDS[EPS_CMD_GET_NUMBER_OF_CWDR][1], EPS_I2C_TIMEOUT, 1)) != OS_SUCCESS)
    {
        //override resets in the
        //event an error occurs
        *data_output = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    {
        if (response != 0xFFFF)
        {
            *data_output = response;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_NUMBER_OF_CWDR);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
    }
 
    return rval;
}

//TODO this commands does not return bytes, but will it 
//return 0xFFFF if an error occurs?
int32 EPS_switch_all_PDM_on(void)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_SWITCH_ON_ALL_PDM][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL, 
                             EPS_COMMANDS[EPS_CMD_SWITCH_ON_ALL_PDM][1], EPS_I2C_TIMEOUT, 0)) != OS_SUCCESS)
    {
        rval = OS_ERROR;
    }
 
    return rval;
}

//TODO this commands does not return bytes, but will it 
//return 0xFFFF if an error occurs?
int32 EPS_switch_all_PDM_off(void)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_SWITCH_OFF_ALL_PDM][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL, 
                             EPS_COMMANDS[EPS_CMD_SWITCH_OFF_ALL_PDM][1], EPS_I2C_TIMEOUT, 0)) != OS_SUCCESS)
    {
        rval = OS_ERROR;
    }
 
    return rval;
}

int32 EPS_get_all_PDM_actual_state(uint32 *data_output)
{
    uint8 message[2];
    uint32 response;
    uint32 actual_state;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_ACTUAL_STATE_ALL_PDM][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response, 
                             EPS_COMMANDS[EPS_CMD_GET_ACTUAL_STATE_ALL_PDM][1], EPS_I2C_TIMEOUT, 20)) != OS_SUCCESS)
    {
        //override state in the
        //event an error occurs
        *data_output = 0xFFFFFFFF;
        rval = OS_ERROR;
    }
    else
    {
        actual_state = response & 0x000007FE; 
        if ((response >> 16) != 0xFFFF)
        {
            *data_output = actual_state;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_ACTUAL_STATE_ALL_PDM);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFFFFFF;
            rval = OS_ERROR;
        }
    }
 
    return rval;
}

int32 EPS_get_all_PDM_expected_state(uint32 *data_output)
{
    uint8 message[2];
    uint32 response;
    uint32 expected_state;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_EXPECTED_STATE_ALL_PDM][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response, 
                             EPS_COMMANDS[EPS_CMD_GET_EXPECTED_STATE_ALL_PDM][1], EPS_I2C_TIMEOUT, 1)) != OS_SUCCESS)
    {
        //override state in the
        //event an error occurs
        *data_output = 0xFFFFFFFF;
        rval = OS_ERROR;
    }
    else
    {
        expected_state = response & 0x000007FE;
        if ((response >> 16) != 0xFFFF)
        {
            *data_output = expected_state;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_EXPECTED_STATE_ALL_PDM);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFFFFFF;
            rval = OS_ERROR;
        }
    }
 
    return rval;
}

int32 EPS_get_PDM_init_state(uint32 *data_output)
{
    uint8 message[2];
    uint32 response;
    uint32 init_state;
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_GET_INIT_STATE_ALL_PDM][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response, 
                             EPS_COMMANDS[EPS_CMD_GET_INIT_STATE_ALL_PDM][1], EPS_I2C_TIMEOUT, 20)) != OS_SUCCESS)
    {
        //override state in the
        //event an error occurs
        *data_output = 0xFFFFFFFF;
        rval = OS_ERROR;
    }
    else
    {
        init_state = response & 0x000007FE;
        if ((response >> 16) != 0xFFFF)
        {
            *data_output = init_state;
        }
        else
        {
            EPS_get_last_error(EPS_CMD_GET_INIT_STATE_ALL_PDM);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFFFFFF;
            rval = OS_ERROR;
        }
    }
 
    return rval;
}

//TODO this commands does not return bytes, but will it 
//return 0xFFFF if an error occurs?
int32 EPS_set_all_PDM_init_state(void)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_SET_ALL_PDM_TO_INIT_STATE][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL, 
                             EPS_COMMANDS[EPS_CMD_SET_ALL_PDM_TO_INIT_STATE][1], EPS_I2C_TIMEOUT, 20)) != OS_SUCCESS)
    {
        rval = OS_ERROR;
    } 
  
    return rval;
}

//TODO this commands does not return bytes, but it WILL 
//return 0xFFFF if an error occurs. See manual pg 49.
int32 EPS_switch_PDM_on(uint8 pdm_channel)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    if (pdm_channel < 1 || pdm_channel > 10)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - PDM channel out of range");
        rval = OS_ERROR;
    }
    else
    {
        message[0] = EPS_COMMANDS[EPS_CMD_SWITCH_PDM_N_ON][0];
        message[1] = pdm_channel;

        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL,
                                 EPS_COMMANDS[EPS_CMD_SWITCH_PDM_N_ON][1], EPS_I2C_TIMEOUT, 0)) != OS_SUCCESS)
        {
            rval = OS_ERROR;
        }
    }

    return rval;
}

//TODO this commands does not return bytes, but it WILL 
//return 0xFFFF if an error occurs. See manual pg 49.
int32 EPS_switch_PDM_off(uint8 pdm_channel)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    if (pdm_channel < 1 || pdm_channel > 10)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - PDM channel out of range");
        rval = OS_ERROR;
    }
    else
    {

        message[0] = EPS_COMMANDS[EPS_CMD_SWITCH_PDM_N_OFF][0];
        message[1] = pdm_channel;
    
        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL,
                                 EPS_COMMANDS[EPS_CMD_SWITCH_PDM_N_OFF][1], EPS_I2C_TIMEOUT, 0)) != OS_SUCCESS)
        {
            rval = OS_ERROR;
        }
    }

    return rval;
}

//TODO this commands does not return bytes, but it WILL 
//return 0xFFFF if an error occurs. See manual pg 49.
int32 EPS_set_PDM_init_state_on(uint8 pdm_channel)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    if (pdm_channel < 1 || pdm_channel > 10)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - PDM channel out of range");
        rval = OS_ERROR;
    }
    else
    {
        message[0] = EPS_COMMANDS[EPS_CMD_SET_PDM_N_INIT_STATE_ON][0];
        message[1] = pdm_channel;

        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL,
                                 EPS_COMMANDS[EPS_CMD_SET_PDM_N_INIT_STATE_ON][1], EPS_I2C_TIMEOUT, 200)) != OS_SUCCESS)
        {
            rval = OS_ERROR;
        }
    }

    return rval;
}

//TODO this commands does not return bytes, but it WILL 
//return 0xFFFF if an error occurs. See manual pg 49.
int32 EPS_set_PDM_init_state_off(uint8 pdm_channel)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    if (pdm_channel < 1 || pdm_channel > 10)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - PDM channel out of range");
        rval = OS_ERROR;
    }
    else
    {
        message[0] = EPS_COMMANDS[EPS_CMD_SET_PDM_N_INIT_STATE_OFF][0];
        message[1] = pdm_channel;
    
        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL,
                                 EPS_COMMANDS[EPS_CMD_SET_PDM_N_INIT_STATE_OFF][1], EPS_I2C_TIMEOUT, 200)) != OS_SUCCESS)
        {
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 EPS_get_PDM_actual_status(uint8 pdm_channel, uint16 *data_output)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    if (pdm_channel < 1 || pdm_channel > 10)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS_LIB: Error - PDM channel out of range");
        rval = OS_ERROR;
    }
    else
    {

        message[0] = EPS_COMMANDS[EPS_CMD_GET_PDM_N_ACTUAL_STATUS][0];
        message[1] = pdm_channel;

        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response,
                                 EPS_COMMANDS[EPS_CMD_GET_PDM_N_ACTUAL_STATUS][1], EPS_I2C_TIMEOUT, 2)) != OS_SUCCESS)
        {
            //override status in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
        else
        {
            if (response != 0xFFFF)
            {
                *data_output = response;
            }
            else
            {
                EPS_get_last_error(EPS_CMD_GET_ACTUAL_STATE_ALL_PDM);
                //override status in the
                //event an error occurs
                *data_output = 0xFFFF;
                rval = OS_ERROR;
            }
        }
    }

    return rval;
}

//TODO this commands does not return bytes, but it WILL 
//return 0xFFFF if an error occurs. See manual pg 52.
int32 EPS_set_PDM_timer_limit(uint16 data)
{
    uint8 message[3];
    int32 rval = OS_SUCCESS;
    uint8 pdm_channel = data >> 8;
    uint8 limit = data & 0xFF;

    if (pdm_channel < 1 || pdm_channel > 10)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - PDM channel out of range");
        rval = OS_ERROR;
    }
    else if (limit != 0x00 && limit < (uint8)0x0A) //multiples of 30sec from 0x0A to 0xFE, 0xFF enables PDM indefintely; 0x00 disables PDM indefinitely
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: EPS_set_PDM_timer_limit: Timer limit out of range");
        rval = OS_ERROR;
    }
    else
    {
        message[0] = EPS_COMMANDS[EPS_CMD_SET_PDM_N_TIMER_LIMIT][0];
        message[1] = pdm_channel;
        message[2] = limit;

        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 3, NULL,
                                 EPS_COMMANDS[EPS_CMD_SET_PDM_N_TIMER_LIMIT][1], EPS_I2C_TIMEOUT, 150)) != OS_SUCCESS)
        {
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 EPS_get_PDM_timer_limit(uint8 pdm_channel, uint16 *data_output)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    if (pdm_channel < 1 || pdm_channel > 10)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - PDM channel out of range");
        rval = OS_ERROR;
    }
    else
    {
        message[0] = EPS_COMMANDS[EPS_CMD_GET_PDM_N_TIMER_LIMIT][0];
        message[1] = pdm_channel;

        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response,
                                 EPS_COMMANDS[EPS_CMD_GET_PDM_N_TIMER_LIMIT][1], EPS_I2C_TIMEOUT, 5)) != OS_SUCCESS)
        {
            //override limit in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
        else
        {
            if (response != 0xFFFF)
            {
                *data_output = response;
            }
            else
            {
                EPS_get_last_error(EPS_CMD_GET_PDM_N_TIMER_LIMIT);
                //override limit in the
                //event an error occurs
                *data_output = 0xFFFF;
                rval = OS_ERROR;
            }
        }
    }

    return rval;
}

int32 EPS_get_PDM_current_timer_val(uint8 pdm_channel, uint16 *data_output)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    if (pdm_channel < 1 || pdm_channel > 10)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - PDM channel out of range");
        rval = OS_ERROR;
    }
    else
    {
        message[0] = EPS_COMMANDS[EPS_CMD_GET_PDM_N_CURRENT_TIMER_VAL][0];
        message[1] = pdm_channel;

        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, &response,
                                 EPS_COMMANDS[EPS_CMD_GET_PDM_N_CURRENT_TIMER_VAL][1], EPS_I2C_TIMEOUT, 1)) != OS_SUCCESS)
        {
            //override value in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
        else
        {
            if (response != 0xFFFF)
            {
                *data_output = response;
            }
            else
            {
                EPS_get_last_error(EPS_CMD_GET_PDM_N_CURRENT_TIMER_VAL);
                //override limit in the
                //event an error occurs
                *data_output = 0xFFFF;
                rval = OS_ERROR;
            }
        }
    }

    return rval;
}

int32 EPS_reset_PCM(uint8 pcm_channel)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    if (pcm_channel < (uint8)0x01 || pcm_channel > (uint8)0x0F)
    {
        CFE_EVS_SendEvent(EPSLIB_I2C_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - PCM channel(s) out of range");
        rval = OS_ERROR;
    }
    else
    {
        message[0] = EPS_COMMANDS[EPS_CMD_PCM_RESET][0];
        message[1] = pcm_channel;

        if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL,
                                 EPS_COMMANDS[EPS_CMD_PCM_RESET][1], EPS_I2C_TIMEOUT, 1)) != OS_SUCCESS)
        {
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 EPS_manual_reset(void)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    message[0] = EPS_COMMANDS[EPS_CMD_MANUAL_RESET][0];
    message[1] = 0x00;

    if ((EPS_i2c_transaction(EPS_I2C_HANDLE, EPS_ADDRESS, message, 2, NULL,
                             EPS_COMMANDS[EPS_CMD_MANUAL_RESET][1], EPS_I2C_TIMEOUT, 0)) != OS_SUCCESS)
    {
        rval = OS_ERROR;
    }

    return rval;
}


int EPS_i2c_transaction(int handle, uint8_t addr, void *txbuf, uint8_t txlen, void *rxbuf, uint8_t rxlen, uint16_t timeout, uint8 delay)
{

    OS_MutSemTake(eps_i2c_transaction_mutex);

    int status;

    status = i2c_master_transaction(handle, addr, txbuf, txlen, NULL, 0, timeout);

    if (status != OS_SUCCESS)
    {
        OS_MutSemGive(eps_i2c_transaction_mutex);
        return status;
    }

    if (delay > 0)
        OS_TaskDelay(delay);

    if (rxbuf)
    {
        status = i2c_master_transaction(handle, addr, NULL, 0, rxbuf, rxlen, timeout);
    }

    OS_MutSemGive(eps_i2c_transaction_mutex);

    return status;
}


/*! 
 * \brief Convert EPS counts to value.
 *  
 * Values for flight and test EPS based on CoC-1381 and CoC-1382 respectively. 
 * - units are V, A, and degrees celsius
 * - discrepancy between table and section 8.3 w.r.t. temperature conversion factor
 */
float EPS_telemetry_conversion(uint16 chan, uint16 read_value )
{
    float multiplier = 0.0;
    float offset = 0.0;

    //TODO add this as a #def in cmake or somewhere else
    uint8 flight = 1; //flight EPS SN: CS005495; test EPS SN: CS005496

    switch(chan)
    {
    /*VBCR1*/
    case CHANNEL_VBCR1:
        if (flight)
        {
            multiplier = 0.0250;
            offset = -0.054031764;
        }
        else
        {
            multiplier = 0.0250;
            offset = -0.069905098;
        }
        break; 

    case CHANNEL_IBCR1A:
        if (flight)
        {
            multiplier = 0.9874;
            offset = -4.174173455;
        }
        else
        {
            multiplier = 0.9785;
            offset = 1.77603005;
        }
        break;

    case CHANNEL_IBCR1B:
        if (flight)
        {
            multiplier = 0.9789;
            offset = 7.008291672;
        }
        else
        {
            multiplier = 0.9807;
            offset = 3.218265628;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR1A:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;  
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;  
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR1B:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;  
        }
        break;
      
    /*VBCR2*/
    case CHANNEL_VBCR2:
        if (flight)
        {
            multiplier = 0.0249;
            offset = 0.017106143;
        }
        else
        {
            multiplier = 0.0242;
            offset = 0.335439976;
        }
        break;

    case CHANNEL_IBCR2A:
        if (flight)
        {
            multiplier = 0.9750;
            offset = -3.503655217;
        }
        else
        {
            multiplier = 1.0419;
            offset = -18.14088122;
        }
        break;

    case CHANNEL_IBCR2B:
        if (flight)
        {
            multiplier = 0.9757;
            offset = 7.049298113;
        }
        else
        {
            multiplier = 0.9384;
            offset = 8.821566144;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR2A:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR2B:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        break;

    /*VBCR3*/
    case CHANNEL_VBCR3:
        if (flight)
        {
            multiplier = 0.0100;
            offset = -0.002789656;
        }
        else
        {
            multiplier = 0.0099;
            offset = 0.03745770;
        }
        break;

    case CHANNEL_IBCR3A:
        if (flight)
        {
            multiplier = 0.9909;
            offset = -3.835232987;
        }
        else
        {
            multiplier = 0.9805;
            offset = -5.390965407;
        }
        break;

    case CHANNEL_IBCR3B:
        if (flight)
        {
            multiplier = 0.9742;
            offset = -3.524680552;
        }
        else
        {
            multiplier = 0.9761;
            offset = -0.997164069;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR3A:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR3B:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        break;

    /*VBCR4*/
    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_VBCR4:
        if (flight)
        {
            multiplier = 0.0249;
            offset = 0.0;
        }
        else
        {
            multiplier = 0.0249;
            offset = 0.0;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_IBCR4A:
        if (flight)
        {
            multiplier = 0.0009775;
            offset = 0;
        }
        else
        {
            multiplier = 0.0009775;
            offset = 0;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_IBCR4B:
        if (flight)
        {
            multiplier = 0.0009775;
            offset = 0;
        }
        else
        {
            multiplier = 0.0009775;
            offset = 0;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR4A:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR4B:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        break;


    /*VBCR5*/
    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_VBCR5:
        if (flight)
        {
            multiplier = 0.0249;
            offset = 0.0;
        }
        else
        {
            multiplier = 0.0249;
            offset = 0.0;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_IBCR5A:
        if (flight)
        {
            multiplier = 0.0009775;
            offset = 0.0;
        }
        else
        {
            multiplier = 0.0009775;
            offset = 0.0;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_IBCR5B:
        if (flight)
        {
            multiplier = 0.0009775;
            offset = 0.0;
        }
        else
        {
            multiplier = 0.0009775;
            offset = 0.0;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR5A:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        break;

    //TODO Calibration sheet did not have
    //a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_TBCR5B:
        if (flight)
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        else
        {
            multiplier = 0.4963;
            offset = -273.15;
        }
        break;

    /*BCR Outputs*/
    case CHANNEL_IIDIODE_OUT:
        if (flight)
        {
            multiplier = 14.177;
            offset = -6.257116521;
        }
        else
        {
            multiplier = 14.36982316;
            offset = -18.81296305;
        }
        break;

    case CHANNEL_VIDIODE_OUT:
        if (flight)
        {
            multiplier = 0.0090;
            offset = 0.01881831;
        }
        else
        {
            multiplier = 0.009;
            offset = 0.001666667;
        }
        break;

//TODO Calibration sheet did not have
//a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_I3V3_DRW:
        if (flight)
        {
            multiplier = 0.001327547;
        }
        else
        {
            multiplier = 0.001327547;
        }
        break;

//TODO Calibration sheet did not have
//a value for this case. Using USM-01-02668 rev A.
    case CHANNEL_I5V_DRW:
        if (flight)
        {
            multiplier = 0.001327547;
        }
        else
        {
            multiplier = 0.001327547;
        }
        break;

        /*PCM*/
    case CHANNEL_IPCM12V:
        if (flight)
        {
            multiplier = 2.06;
            offset = 3.262117965;
        }
        else
        {
            multiplier = 2.06;
            offset = -6.078449002;
        }

        break;

    case CHANNEL_VPCM12V:
        if (flight)
        {
            multiplier = 0.005;
            offset = 7.57;
        }
        else
        {
            multiplier = 0.009;
            offset = 4.013;
        }

        break;

    case CHANNEL_IPCMBATV:
        if (flight)
        {
            multiplier = 5.284;
            offset = -16.01660841;
        }
        else
        {
            multiplier = 5.297;
            offset = -15.14973264;
        }

        break;

    case CHANNEL_VPCMBATV:
        if (flight)
        {
            multiplier = 0.009412;
            offset = -0.378276679;
        }
        else
        {
            multiplier = 0.009535;
            offset = -0.447889995;
        }

        break;

    case CHANNEL_IPCM5V:
        if (flight)
        {
            multiplier = 5.250;
            offset = -9.605679262;
        }
        else
        {
            multiplier = 5.268;
            offset = 17.02871336;
        }

        break;

    case CHANNEL_VPCM5V:
        if (flight)
        {
            multiplier = 0.007486;
            offset = -1.398997817;
        }
        else
        {
            multiplier = 0.007205;
            offset = -1.162948718;
        }

        break;

    case CHANNEL_IPCM3V3:
        if (flight)
        {
            multiplier = 5.291;
            offset = -25.11133945;
        }
        else
        {
            multiplier = 5.247;
            offset = -12.46619029;
        }

        break;

    case CHANNEL_VPCM3V3:
        if (flight)
        {
            multiplier = 0.005288627;
            offset = -0.768041845;
        }
        else
        {
            multiplier = 0.00549;
            offset = -0.923534014;
        }

        break;
    
        /*Switches*/
        //TODO: Changing flight Currents to uncalibrated. Possibly get true calibrated values
    case CHANNEL_VSW1: //12V
        if (flight)
        {
            multiplier = 0.015;
            offset = -1.362;
        }
        else
        {
            multiplier = 0.0250;
            offset = -10.242;
        }

        break;

    case CHANNEL_ISW1:
        if (flight)
        {
            multiplier = 0.001328;
            offset = 0;
        }
        else
        {
            multiplier = 0.0013;
            offset = -7.744670749;
        }

        break;

    case CHANNEL_VSW2: //12V
        if (flight)
        {
            multiplier = 0.01297;
            offset = 0.454444444;
        }
        else
        {
            multiplier = 0.02296;
            offset = -8.539259259;
        }

        break;

    case CHANNEL_ISW2: 
        if (flight)
        {
            multiplier = 0.001328;
            offset = 0;
        }
        else
        {
            multiplier = 0.00133;
            offset = -5.843992008;
        }

        break;

    case CHANNEL_VSW3: //5V
        if (flight)
        {
            multiplier = 0.00723;
            offset = -1.184866667;
        }
        else
        {
            multiplier = 0.00692;
            offset = -0.917487603;
        }

        break;

    case CHANNEL_ISW3:
        if (flight)
        {
            multiplier = 0.00138;
            offset = 0;
        }
        else
        {
            multiplier = 0.00132;
            offset = -3.690421648;
        }

        break;

    case CHANNEL_VSW4: //3.3V
        if (flight)
        {
            multiplier = 0.00505;
            offset = -0.59025;
        }
        else
        {
            multiplier = 0.005112;
            offset = -0.624585313;
        }

        break;

    case CHANNEL_ISW4:
        if (flight)
        {
            multiplier = 0.001328;
            offset = 0;
        }
        else
        {
            multiplier = 0.001324;
            offset = -0.146562849;
        }

        break;

    case CHANNEL_VSW5: //5V
        if (flight)
        {
            multiplier = 0.00793;
            offset = -1.808213115;
        }
        else
        {
            multiplier = 0.007441624;
            offset = -1.369142132;
        }

        break;

    case CHANNEL_ISW5:
        if (flight)
        {
            multiplier = 0.001328;
            offset = 0;
        }
        else
        {
            multiplier = 0.001331833;
            offset = -6.096487992;
        }

        break;

    case CHANNEL_VSW6: //5V
        if (flight)
        {
            multiplier = 0.00713;
            offset = -1.098266667;
        }
        else
        {
            multiplier = 0.007418782;
            offset = -1.345142132;
        }

        break;

    case CHANNEL_ISW6:
        if (flight)
        {
            multiplier = 0.001328;
            offset = 0;
        }
        else
        {
            multiplier = 0.001335975;
            offset = -7.937734278;
        }

        break;

    case CHANNEL_VSW7: //5V
        if (flight)
        {
            multiplier = 0.007812;
            offset = -1.685502538;
        }
        else
        {
            multiplier = 0.007928;
            offset = -1.784819672;
        }

        break;

    case CHANNEL_ISW7:
        if (flight)
        {
            multiplier = 0.001328;
            offset = 0;
        }
        else
        {
            multiplier = 0.001344;
            offset = -2.138472187;
        }

        break;

    case CHANNEL_VSW8: //3.3V
        if (flight)
        {
            multiplier = 0.004881;
            offset = -0.453816415;
        }
        else
        {
            multiplier = 0.005372;
            offset = -0.831581448;
        }

        break;

    case CHANNEL_ISW8:
        if (flight)
        {
            multiplier = 0.001328;
            offset = 0;
        }
        else
        {
            multiplier = 0.001326;
            offset = -1.586138893;
        }

        break;

    case CHANNEL_VSW9: //3.3V
        if (flight)
        {
            multiplier = 0.005472;
            offset = -0.90680663;
        }
        else
        {
            multiplier = 0.005501;
            offset = -0.92850591;
        }

        break;

    case CHANNEL_ISW9:
        if (flight)
        {
            multiplier = 0.001328;
            offset = 0;
        }
        else
        {
            multiplier = 0.001337;
            offset = -1.868780037;
        }

        break;

    case CHANNEL_VSW10: //3.3V
        if (flight)
        {
            multiplier = 0.005082;
            offset = -0.608583153;
        }
        else
        {
            multiplier = 0.006311;
            offset = -1.55274344;
        }

        break;

    case CHANNEL_ISW10:
        if (flight)
        {
            multiplier = 0.001328;
            offset = 0;
        }
        else
        {
            multiplier = 0.001320;
            offset = -4.740413691;
        }

        break;

        /*Board Temp*/
    case CHANNEL_TBRD:
        if (flight)
        {
            multiplier = 0.3721;
            offset = -274.3;
        }
        else
        {
            multiplier = 0.3716;
            offset = -273.4;
        }

        break;
 
        /*default:
          TODO: Error-Channel does not exist?*/
    default:
        CFE_EVS_SendEvent(EPSLIB_TLM_ERR_EID, CFE_EVS_ERROR, "EPS LIB: Error - channel 0x%04X does not exist", chan);
        break;

    }

    return (multiplier * (read_value & 0x07ff)) + offset;
  
}








