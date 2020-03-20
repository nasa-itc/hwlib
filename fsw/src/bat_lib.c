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

#include "cfe_evs.h"
#include "osapi.h"
#include "common_types.h"
#include "eps/eps_bat.h"
#include "libi2c.h"


#define BATLIB_I2C_ERR_EID 1002
#define BATLIB_TLM_ERR_EID 1003

#define BAT_I2C_HANDLE 0
#define BAT_I2C_TIMEOUT 100 //ms

static const uint8 BAT_I2C_ADDRESSES[] = {0x2D, 0x2A};

static const uint8 BAT_COMMANDS[BAT_CMD_COUNT][2] = {
    {0x01, 2},  // BAT_CMD_GET_BOARD_STATUS
    {0x03, 2},  // BAT_CMD_GET_LAST_ERROR
    {0x04, 2},  // BAT_CMD_GET_VERSION
    {0x05, 2},  // BAT_CMD_GET_CHECKSUM
    {0x10, 2},  // BAT_CMD_GET_TELEMETRY
    {0x31, 2},  // BAT_CMD_GET_NUMBER_OF_BOR
    {0x32, 2},  // BAT_CMD_GET_NUMBER_OF_ASR
    {0x33, 2},  // BAT_CMD_GET_NUMBER_OF_MR
    {0x90, 2},  // BAT_CMD_GET_HEATER_CONTROL_STATUS
    {0x91, 2},  // BAT_CMD_SET_HEATER_CONTROL_STATUS
    {0x80, 0}   // BAT_CMD_MANUAL_RESET
};

static uint16 BAT_CHANNEL_ADDRESS [BAT_ANALOG_CHANNELS] = 
{
    0xE280, // TLM_VBAT
    0xE284, // TLM_IBAT
    0xE28E, // TLM_IDIRBAT
    0xE308, // TLM_TBRD
    0xE214, // TLM_IPCM5V
    0xE210, // TLM_VPCM5V
    0xE204, // TLM_IPCM3V3
    0xE200, // TLM_VPCM3V3
    0xE398, // TLM_TBAT1
    0xE39F, // TLM_HBAT1
    0xE3A8, // TLM_TBAT2
    0xE3AF, // TLM_HBAT2
    0xE3B8, // TLM_TBAT3
    0xE3BF, // TLM_HBAT3
    0xE3C8, // TLM_TBAT4
    0xE3CF  // TLM_HBAT4
};

static float BAT_TELEM_CONVERSIONS [BAT_COUNT][BAT_ANALOG_CHANNELS][2] = 
{
    { // CS05602 - CoC-1326
        {0.01154113, -2.165166006},   // TLM_VBAT
        {14.12626972, -21.29715505},  // TLM_IBAT
        {0.0, 0.0},                   // TLM_IDIRBAT (not used)  
        {0.373270499, -274.3071741},  // TLM_TBRD
        {1.326049, -4.971009049},     // TLM_IPCM5V
        {0.005875, -0.019700461},     // TLM_VPCM5V
        {1.332333, -4.006814185},     // TLM_IPCM3V3
        {0.004378, -0.062901554},     // TLM_VPCM3V3
        {0.356420, -211.8428277},     // TLM_TBAT1
        {0.0, 0.0},                   // TLM_HBAT1 (not used)
        {0.358785, -213.8201893},     // TLM_TBAT2
        {0.0, 0.0},                   // TLM_HBAT2 (not used)
        {0.357263, -212.2530611},     // TLM_TBAT3
        {0.0, 0.0},                   // TLM_HBAT3 (not used)
        {0.359659, -214.94825},       // TLM_TBAT4
        {0.0, 0.0}                    // TLM_HBAT4 (not used)
    },
    { // CS05876 - CoC-1434-01-04974
        {0.013799092, -4.077361669},  // TLM_VBAT
        {14.2197363, -22.09711562},   // TLM_IBAT
        {0.0, 0.0},                   // TLM_IDIRBAT (not used)  
        {0.376119835, -276.5698132},  // TLM_TBRD
        {1.325265973, -0.77177011},   // TLM_IPCM5V
        {0.00625, -0.33125},          // TLM_VPCM5V
        {1.317095431, 3.117553712},   // TLM_IPCM3V3
        {0.003893443, 0.324672131},   // TLM_VPCM3V3
        {0.378749772, -227.1297211},  // TLM_TBAT1
        {0.0, 0.0},                   // TLM_HBAT1 (not used)
        {0.375908852, -225.6774861},  // TLM_TBAT2
        {0.0, 0.0},                   // TLM_HBAT2 (not used)
        {0.379299669, -227.9787862},  // TLM_TBAT3
        {0.0, 0.0},                   // TLM_HBAT3 (not used)
        {0.375748837, -224.3624786},  // TLM_TBAT4
        {0.0, 0.0}                    // TLM_HBAT4 (not used)
    }
};

static int BAT_i2c_transaction(int handle, uint8_t addr, void *txbuf, uint8_t txlen, void *rxbuf, uint8_t rxlen, uint16_t timeout, uint8 delay);

static float BAT_telemetry_conversion(uint8 channel, uint16 read_value, uint8 battery);

int32 BAT_LibInit(void)
{
    return OS_SUCCESS;
}

int32 BAT_get_last_error(uint8 channel, uint8 battery)
{
    uint8 message[2];
    uint16 error;
    int32 rval = OS_SUCCESS;

    message[0] = BAT_COMMANDS[BAT_CMD_GET_LAST_ERROR][0];
    message[1] = 0x00;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, &error,
                             BAT_COMMANDS[BAT_CMD_GET_LAST_ERROR][1], BAT_I2C_TIMEOUT, 1)) != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Error - BAT_get_last_error: I2C error");
        rval = OS_ERROR;
    }
    else
    {
        switch(error)
        {
        case 0x10:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: CRC code does not match data", battery, channel);
            break;
        case 0x01:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: Unknown command received", battery, channel);
            break;
        case 0x02:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: Supplied data incorrect when processing command", battery, channel);
            break;
        case 0x03:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: Selected channel does not exist", battery, channel);
            break;
        case 0x04:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: Selected channel is currently inactive", battery, channel); 
            break;
        case 0x13:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: A reset had to occur", battery, channel);
            break;
        case 0x14:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: There was an error with the ADC acquisition", battery, channel);
            break;
        case 0x20:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: Reading from EEPROM generated an error", battery, channel);
            break;
        case 0x30:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: Error on the internal SPI bus", battery, channel);
            break;
        default:
            CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u command 0x%02X error: Undefined error occured", battery, channel);
        }
    }
        return rval;
}

int32 BAT_get_board_status(uint16 *status, uint8 battery)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    message[0] = BAT_COMMANDS[BAT_CMD_GET_BOARD_STATUS][0];
    message[1] = 0x00;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, &response,
                             BAT_COMMANDS[BAT_CMD_GET_BOARD_STATUS][1], BAT_I2C_TIMEOUT, 1)) != OS_SUCCESS)
    {
        //override status in the
        //event an error occurs
        *status = 0xFFFF;
        rval = OS_ERROR; 
    }
    else
    {
        if (response != 0xFFFF)
        {
            *status = (response & 0xFF00) >> 8;
        }
        else
        {
            BAT_get_last_error(BAT_CMD_GET_BOARD_STATUS, battery);
            //override status in the
            //event an error occurs
            *status = 0xFFFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 BAT_get_version(uint16 *version, uint8 battery)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    message[0] = BAT_COMMANDS[BAT_CMD_GET_VERSION][0];
    message[1] = 0x00;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, &response,
                             BAT_COMMANDS[BAT_CMD_GET_VERSION][1], BAT_I2C_TIMEOUT, 1)) != OS_SUCCESS)
    {
        //override version in the 
        //event an error occurs
        *version = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    {
        if (response != 0xFFFF)
        {
            *version = response;
        }
        else
        {
            BAT_get_last_error(BAT_CMD_GET_VERSION, battery);
            //override version in the 
            //event an error occurs
            *version = 0xFFFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 BAT_get_checksum(uint16 *checksum, uint8 battery)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    message[0] = BAT_COMMANDS[BAT_CMD_GET_CHECKSUM][0];
    message[1] = 0x00;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, &response,
                             BAT_COMMANDS[BAT_CMD_GET_CHECKSUM][1], BAT_I2C_TIMEOUT, 35)) != OS_SUCCESS)
    {
        //override checksum in the
        //event an error occurs
        *checksum = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    {
        if (response != 0xFFFF)
        {
            *checksum = response;
        }
        else
        {
            BAT_get_last_error(BAT_CMD_GET_CHECKSUM, battery);
            //override checksum in the
            //event an error occurs
            *checksum = 0xFFFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 BAT_manual_reset(uint8 battery)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    message[0] = BAT_COMMANDS[BAT_CMD_MANUAL_RESET][0];
    message[1] = 0x00;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, NULL,
                             BAT_COMMANDS[BAT_CMD_MANUAL_RESET][1], BAT_I2C_TIMEOUT, 0)) != OS_SUCCESS)
    {
        rval = OS_ERROR;
    }

    return rval;
}

int32 BAT_get_telemetry(uint8 channel, float *data_output, uint8 battery)
{

    uint16 response;
    uint8 message[3];
    int32 rval = OS_SUCCESS;
    uint16 channel_address = BAT_CHANNEL_ADDRESS[channel];

    message[0] = BAT_COMMANDS[BAT_CMD_GET_TELEMETRY][0];
    message[1] = channel_address >> 8;
    message[2] = channel_address & 0xFF;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 3, &response, 
                             BAT_COMMANDS[BAT_CMD_GET_TELEMETRY][1], BAT_I2C_TIMEOUT, 5)) != OS_SUCCESS)
    {
        *data_output = 0.0;
        rval = OS_ERROR;
    }
    else
    {
        if (response != 0xFFFF)
        {
            *data_output = BAT_telemetry_conversion(channel, response, battery);
        }
        else
        {
            BAT_get_last_error(BAT_CMD_GET_TELEMETRY, battery);
            *data_output = 0.0;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 BAT_get_num_brownout_resets(uint16 *data_output, uint8 battery)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    message[0] = BAT_COMMANDS[BAT_CMD_GET_NUMBER_OF_BOR][0];
    message[1] = 0x00;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, &response, 
                             BAT_COMMANDS[BAT_CMD_GET_NUMBER_OF_BOR][1], BAT_I2C_TIMEOUT, 1)) != OS_SUCCESS)
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
            BAT_get_last_error(BAT_CMD_GET_NUMBER_OF_BOR, battery);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 BAT_get_num_auto_software_resets(uint16 *data_output, uint8 battery)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    message[0] = BAT_COMMANDS[BAT_CMD_GET_NUMBER_OF_ASR][0];
    message[1] = 0x00;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, &response, 
                             BAT_COMMANDS[BAT_CMD_GET_NUMBER_OF_ASR][1], BAT_I2C_TIMEOUT, 1)) != OS_SUCCESS)
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
            BAT_get_last_error(BAT_CMD_GET_NUMBER_OF_ASR, battery);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
    }

    return rval;
}

int32 BAT_get_num_manual_resets(uint16 *data_output, uint8 battery)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    message[0] = BAT_COMMANDS[BAT_CMD_GET_NUMBER_OF_MR][0];
    message[1] = 0x00;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, &response, 
                             BAT_COMMANDS[BAT_CMD_GET_NUMBER_OF_MR][1], BAT_I2C_TIMEOUT, 1)) != OS_SUCCESS)
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
            BAT_get_last_error(BAT_CMD_GET_NUMBER_OF_MR, battery);
            //override resets in the
            //event an error occurs
            *data_output = 0xFFFF;
            rval = OS_ERROR;
        }
    }
 
    return rval;
}

int32 BAT_get_heater_controller_status(uint16 *status, uint8 battery)
{
    uint8 message[2];
    uint16 response;
    int32 rval = OS_SUCCESS;

    message[0] = BAT_COMMANDS[BAT_CMD_GET_HEATER_CONTROLLER_STATUS][0];
    message[1] = 0x00;

    if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, &response, 
                             BAT_COMMANDS[BAT_CMD_GET_HEATER_CONTROLLER_STATUS][1], BAT_I2C_TIMEOUT, 1)) != OS_SUCCESS)
    {
        //override resets in the
        //event an error occurs
        *status = 0xFFFF;
        rval = OS_ERROR;
    }
    else
    {
        if (response != 0xFFFF)
        {
            *status = response & 0x0001;
        }
        else
        {
            BAT_get_last_error(BAT_CMD_GET_HEATER_CONTROLLER_STATUS, battery);
            //override resets in the
            //event an error occurs
            *status = 0xFFFF;
            rval = OS_ERROR;
        }
    }
 
    return rval;
}

int32 BAT_set_heater_controller_status(uint8 heater_code, uint8 battery)
{
    uint8 message[2];
    int32 rval = OS_SUCCESS;

    if (heater_code > 1)
    {
        CFE_EVS_SendEvent(BATLIB_I2C_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Battery %u: Error - Heater code out of range", battery);
        rval = OS_ERROR;
    }
    else
    {
        message[0] = BAT_COMMANDS[BAT_CMD_SET_HEATER_CONTROLLER_STATUS][0];
        message[1] = heater_code;

        if ((BAT_i2c_transaction(BAT_I2C_HANDLE, BAT_I2C_ADDRESSES[battery], message, 2, NULL,
                                 BAT_COMMANDS[BAT_CMD_SET_HEATER_CONTROLLER_STATUS][1], BAT_I2C_TIMEOUT, 1)) != OS_SUCCESS)
        {
            rval = OS_ERROR;
        }
    }

    return rval;
}

int BAT_i2c_transaction(int handle, uint8_t addr, void *txbuf, uint8_t txlen, void *rxbuf, uint8_t rxlen, uint16_t timeout, uint8 delay)
{
    int status;

    status = i2c_master_transaction(handle, addr, txbuf, txlen, NULL, 0, timeout);

    if (status != OS_SUCCESS)
        return status;

    if (delay > 0)
        OS_TaskDelay(delay);

    if (rxbuf)
    {
        status = i2c_master_transaction(handle, addr, NULL, 0, rxbuf, rxlen, timeout);
    }

    return status;
}

float BAT_telemetry_conversion(uint8 channel, uint16 read_value, uint8 battery)
{
    float rval = 0.0;

    if (channel == TLM_IDIRBAT || channel == TLM_HBAT1 || channel == TLM_HBAT2 || channel == TLM_HBAT3 || channel == TLM_HBAT4)
    {
        if ((read_value & 0x07ff) < 512)
            rval = 1.0;
    }
    else if (channel < BAT_ANALOG_CHANNELS)
    {
        rval = (BAT_TELEM_CONVERSIONS[battery][channel][0] * (read_value & 0x07ff)) + BAT_TELEM_CONVERSIONS[battery][channel][1];
    }
    else
    {
        CFE_EVS_SendEvent(BATLIB_TLM_ERR_EID, CFE_EVS_ERROR, "BAT LIB: Error, invalid battery channel number %d\n", channel);
    }

    return rval;
}
