/*
  Copyright (C) 2009 - 2016 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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

#include "hwlib_eps_test.h"
#include "hwlib_test_utils.h"

#include <eps_lib.h>

#include <uttest.h>
#include <utassert.h>

/* eps double test tolerance */
#define EPS_TOLERANCE 0.001

/* eps params */
#define EPS_BCR_COUNT 3 
#define EPS_PCM_COUNT 4
#define EPS_SWITCH_COUNT 10

/* eps i2c params */
#define EPS_I2C_ADDRESS 0x2b
#define EPS_I2C_HANDLE 0
#define EPS_I2C_BUF_MAX 16
#define EPS_I2C_ERROR 0xffffffff

/* eps i2c cmds */
#define TEST_EPS_CMD_GET_BOARD_STATUS                0x01
#define TEST_EPS_CMD_GET_LAST_ERROR                  0x03
#define TEST_EPS_CMD_GET_VERSION                     0x04
#define TEST_EPS_CMD_GET_CHECKSUM                    0x05
#define TEST_EPS_CMD_GET_TELEMETRY                   0x10
#define TEST_EPS_CMD_GET_COM_WATCHDOG_PERIOD         0x20
#define TEST_EPS_CMD_SET_COM_WATCHDOG_PERIOD         0x21
#define TEST_EPS_CMD_RESET_COM_WATCHDOG              0x22
#define TEST_EPS_CMD_GET_NUMBER_OF_BOR               0x31
#define TEST_EPS_CMD_GET_NUMBER_OF_ASR               0x32
#define TEST_EPS_CMD_GET_NUMBER_OF_MR                0x33
#define TEST_EPS_CMD_GET_NUMBER_OF_CWDR              0x34
#define TEST_EPS_CMD_SWITCH_ON_ALL_PDM               0x40
#define TEST_EPS_CMD_SWITCH_OFF_ALL_PDM              0x41
#define TEST_EPS_CMD_GET_ACTUAL_STATE_ALL_PDM        0x42
#define TEST_EPS_CMD_GET_EXPECTED_STATE_ALL_PDM      0x43
#define TEST_EPS_CMD_GET_INIT_STATE_ALL_PDM          0x44
#define TEST_EPS_CMD_SET_ALL_PDM_TO_INIT_STATE       0x45
#define TEST_EPS_CMD_SWITCH_PDM_N_ON                 0x50 
#define TEST_EPS_CMD_SWITCH_PDM_N_OFF                0x51
#define TEST_EPS_CMD_SET_PDM_N_INIT_STATE_ON         0x52
#define TEST_EPS_CMD_SET_PDM_N_INIT_STATE_OFF        0x53
#define TEST_EPS_CMD_GET_PDM_N_ACTUAL_STATUS         0x54
#define TEST_EPS_CMD_SET_PDM_N_TIMER_LIMIT           0x60
#define TEST_EPS_CMD_GET_PDM_N_TIMER_LIMIT           0x61
#define TEST_EPS_CMD_GET_PDM_N_CURRENT_TIMER_VAL     0x62
#define TEST_EPS_CMD_PCM_RESET                       0x70 
#define TEST_EPS_CMD_MANUAL_RESET                    0x80

/* eps telemetry channels */
#define TEST_CHANNEL_BCR_SHIFT   0x0004
#define TEST_CHANNEL_VBCR        0xE110
#define TEST_CHANNEL_IBCRA       0xE114
#define TEST_CHANNEL_IBCRB       0xE115
#define TEST_CHANNEL_TBCRA       0xE118
#define TEST_CHANNEL_TBCRB       0xE119
#define TEST_CHANNEL_PCM_SHIFT   0x0004
#define TEST_CHANNEL_IPCM        0xE204
#define TEST_CHANNEL_VPCM        0xE200
#define TEST_CHANNEL_PDM_SHIFT   0x0004
#define TEST_CHANNEL_VSW         0xE410
#define TEST_CHANNEL_ISW         0xE414
#define TEST_CHANNEL_IIDIODE_OUT 0xE284
#define TEST_CHANNEL_VIDIODE_OUT 0xE280
#define TEST_CHANNEL_I3V3_DRW    0xE205
#define TEST_CHANNEL_I5V_DRW     0xE215
#define TEST_CHANNEL_TBRD        0xE308

/* set (no response) commands */
static uint8 SET_CMDS[] = {
    TEST_EPS_CMD_SET_COM_WATCHDOG_PERIOD,
    TEST_EPS_CMD_RESET_COM_WATCHDOG,
    TEST_EPS_CMD_SWITCH_ON_ALL_PDM,
    TEST_EPS_CMD_SWITCH_OFF_ALL_PDM,
    TEST_EPS_CMD_SET_ALL_PDM_TO_INIT_STATE,
    TEST_EPS_CMD_SWITCH_PDM_N_ON,
    TEST_EPS_CMD_SWITCH_PDM_N_OFF,
    TEST_EPS_CMD_SET_PDM_N_INIT_STATE_ON,
    TEST_EPS_CMD_SET_PDM_N_INIT_STATE_OFF,
    TEST_EPS_CMD_SET_PDM_N_TIMER_LIMIT,
    TEST_EPS_CMD_PCM_RESET,
    TEST_EPS_CMD_MANUAL_RESET
};

#define NUM_SET_COMMANDS sizeof(SET_CMDS)/sizeof(SET_CMDS[0])

static float CONV_TABLE_FLIGHT[][2] = {
    {0.0250, -0.054031764},      /* vbcr1 */
    {0.9874, -4.174173455},      /* ibcr1a */
    {0.9789, 7.008291672},       /* ibcr1b */
    {0.4963, -273.15},           /* tbcr1a */
    {0.4963, -273.15},           /* tbcr1b */
    {0.0249, 0.017106143},       /* vbcr2 */
    {0.9750, -3.503655217},      /* ibcr2a */
    {0.9757, 7.049298113},       /* ibcr2b */
    {0.4963, -273.15},           /* tbcr2a */
    {0.4963, -273.15},           /* tbcr2b */
    {0.0100, -0.002789656},      /* vbcr3 */
    {0.9909, -3.835232987},      /* ibcr3a */
    {0.9742, -3.524680552},      /* ibcr3b */
    {0.4963, -273.15},           /* tbcr3a */
    {0.4963, -273.15},           /* tbcr3b */
    {14.177, -6.257116521},      /* iidiode */
    {0.0090, 0.01881831},        /* vidiode */
    {0.001327547, 0.0},          /* i3v3_drw */
    {0.001327547, 0.0},          /* i5v_drw */
    {0.005288627, -0.768041845}, /* vpcm3v3 */
    {5.291, -25.11133945},       /* ipcm3v3 */
    {0.007486, -1.398997817},    /* vpcm5v */
    {5.250, -9.605679262},       /* ipcm5v */
    {0.009412, -0.378276679},    /* vpcmbatv */
    {5.284, -16.01660841},       /* ipcmbatv */
    {0.005, 7.57},               /* vpcm12v */
    {2.06, 3.262117965},         /* ipcm12v */
    {0.015, -1.362},             /* vsw1 */
    {0.001329, -1.576325527},    /* isw1 */
    {0.01297, 0.454444444},      /* vsw2 */
    {0.001323, -3.976893911},    /* isw2 */
    {0.00723, -1.184866667},     /* vsw3 */
    {0.00132, -5.569464866},     /* isw3 */
    {0.00505, -0.59025},         /* vsw4 */
    {0.00132, -6.333068415},     /* isw4 */
    {0.00793, -1.808213115},     /* vsw5 */
    {0.00132, -6.087316858},     /* isw5 */
    {0.00713, -1.098266667},     /* vsw6 */
    {0.00133, -2.813299957},     /* isw6 */
    {0.007812, -1.685502538},    /* vsw7 */
    {0.001329, -5.313430133},    /* isw7 */
    {0.004881, -0.453816415},    /* vsw8 */
    {0.001327, -5.28174575},     /* isw8 */
    {0.005472, -0.90680663},     /* vsw9 */
    {0.001324, -6.084854687},    /* isw9 */
    {0.005082, -0.608583153},    /* vsw10 */
    {0.001317, -5.259827737},    /* isw10 */
    {0.3721, -274.3},            /* tbrd */
};

static float CONV_TABLE[][2] = {
    {0.0250, -0.069905098},      /* vbcr1 */
    {0.9785, 1.77603005},        /* ibcr1a */
    {0.9807, 3.218265628},       /* ibcr1b */
    {0.4963, -273.15},           /* tbcr1a */
    {0.4963, -273.15},           /* tbcr1b */
    {0.0242, 0.335439976},       /* vbcr2 */
    {1.0419, -18.14088122},      /* ibcr2a */
    {0.9384, 8.821566144},       /* ibcr2b */
    {0.4963, -273.15},           /* tbcr2a */
    {0.4963, -273.15},           /* tbcr2b */
    {0.0099, 0.03745770},        /* vbcr3 */
    {0.9805, -5.390965407},      /* ibcr3a */
    {0.9761, -0.997164069},      /* ibcr3b */
    {0.4963, -273.15},           /* tbcr3a */
    {0.4963, -273.15},           /* tbcr3b */
    {14.36982316, -18.81296305}, /* iidiode */
    {0.009, 0.001666667},        /* vidiode */
    {0.001327547, 0.0},          /* i3v3_drw */
    {0.001327547, 0.0},          /* i5v_drw */
    {0.00549, -0.923534014},     /* vpcm3v3 */
    {5.247, -12.46619029},       /* ipcm3v3 */
    {0.007205, -1.162948718},    /* vpcm5v */
    {5.268, 17.02871336},        /* ipcm5v */
    {0.009535, -0.447889995},    /* vpcmbat */
    {5.297, -15.14973264},       /* ipcmbat */
    {0.009, 4.013},              /* vpcm12v */
    {2.06, -6.078449002},        /* ipcm12v */
    {0.0250, -10.242},           /* vsw1 */
    {0.0013, -7.744670749},      /* isw1 */
    {0.02296, -8.539259259},     /* vsw2 */
    {0.00133, -5.843992008},     /* isw2 */
    {0.00692, -0.917487603},     /* vsw3 */
    {0.00132, -3.690421648},     /* isw3 */
    {0.005112, -0.624585313},    /* vsw4 */
    {0.001324, -0.146562849},    /* isw4 */
    {0.007441624, -1.369142132}, /* vsw5 */
    {0.001331833, -6.096487992}, /* isw5 */
    {0.007418782, -1.345142132}, /* vsw6 */
    {0.001335975, -7.937734278}, /* isw6 */
    {0.007928, -1.784819672},    /* vsw7 */
    {0.001344, -2.138472187},    /* isw7 */
    {0.005372, -0.831581448},    /* vsw8 */
    {0.001326, -1.586138893},    /* isw8 */
    {0.005501, -0.92850591},     /* vsw9 */
    {0.001337, -1.868780037},    /* isw9 */
    {0.006311, -1.55274344},     /* vsw10 */
    {0.001320, -4.740413691},    /* isw10 */
    {0.3716, -273.4},            /* tbrd */
};

/* test eps i2c cmd */
static void test_eps_i2c(uint8 cmd, uint8 data1, uint8 data2, uint8 is_error, const char* cmd_name)
{
    /* verify basic eps i2c params */
    UtAssert_True(i2c_data.handle == EPS_I2C_HANDLE, "eps i2c handle");
    UtAssert_True(i2c_data.address == EPS_I2C_ADDRESS, "eps i2c address");

    /* test description */
    char desc[128];
    strcpy(desc, "eps i2c cmd: ");
    strcat(desc, cmd_name);

    /* is this a set (no response) cmd */
    int cnt = 2;
    unsigned int i;
    for(i = 0; i < NUM_SET_COMMANDS; i++)
    {
        if(cmd == SET_CMDS[i])
        {
            cnt = 1;
            break;
        }
    }
    
    /* cmd cnt - 2 transactions per cmd, additional cmd to get error */
    cnt = (is_error * 2) + cnt;
    if(i2c_data.retcode != E_NO_ERR) cnt = 1;

    /* test i2c cmd */
    UtAssert_True(i2c_data.cmd_cnt == cnt, desc);
    UtAssert_True(i2c_data.txbuf[0] == cmd, desc);
    UtAssert_True(i2c_data.txbuf[1] == data1, desc);
    if((is_error == 0) && (i2c_data.txlen > 1))
    {
        UtAssert_True(i2c_data.txbuf[2] == data2, desc);
    }
}

/* test telemetry */
static void test_tlm(uint16_t channel, uint16_t count, float conv[], const char* tlm_name)
{
    /* tlm description */
    char desc[128];
    strcpy(desc, "eps tlm: ");
    strcat(desc, tlm_name);

    /* reset i2c data */
    HWLib_Reset_I2C_Data();
    i2c_data.rxbuf = (const uint8_t*)&count;
    i2c_data.rxlen = sizeof(count);

    /* test tlm conversion */
    float tlm;
    int result = EPS_get_telemetry(channel, &tlm);
    float exp_tlm = (count * conv[0]) + conv[1];
    UtAssert_True(result == OS_SUCCESS, "eps get tlm success");
    test_eps_i2c(TEST_EPS_CMD_GET_TELEMETRY, channel >> 8, channel & 0xff, 0, "eps tlm");
    UtAssert_DoubleCmpAbs(tlm, exp_tlm, EPS_TOLERANCE, desc);
}

/* test lib init */
static void HWLib_EPS_Test_LibInit(void)
{
    int32 result = EPS_LibInit();
    UtAssert_True(result == OS_SUCCESS, "eps lib init");
}

/* test get board status */
static void HWLib_EPS_Test_Get_Board_Status(void)
{
    /* test board status */
    uint32_t exp_data[] = {0xdeadbeef, 0x20};
    uint8 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_board_status(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get board status success");
    UtAssert_True(data == ((exp_data[0] & 0x7f0000) >> 16), "eps get board status value");
    test_eps_i2c(TEST_EPS_CMD_GET_BOARD_STATUS, 0, 0, 0, "get board status");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test board status error */
    exp_data[0] = EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_board_status(&data);
    UtAssert_True(result == OS_ERROR, "eps get board status error");
    UtAssert_True((int8)data == -1, "eps get board status value");
    test_eps_i2c(TEST_EPS_CMD_GET_BOARD_STATUS, 0, 0, 1, "get board status error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_board_status(&data);
    UtAssert_True(result == OS_ERROR, "eps get board status i2c error");
    UtAssert_True((int8)data == -1, "eps get board status value");
    test_eps_i2c(TEST_EPS_CMD_GET_BOARD_STATUS, 0, 0, 1, "get board status i2c error");
}

/* test get version */
static void HWLib_EPS_Test_Get_Version(void)
{
    /* test board version */
    uint32_t exp_data[] = {0xdeadbeef, 0x20};
    uint16 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_version(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get version success");
    UtAssert_True(data == (exp_data[0] >> 16), "eps get version value");
    test_eps_i2c(TEST_EPS_CMD_GET_VERSION, 0, 0, 0, "get version");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test board version error */
    exp_data[0] = EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_version(&data);
    UtAssert_True(result == OS_ERROR, "eps get version error");
    UtAssert_True((int16)data == -1, "eps get version value");
    test_eps_i2c(TEST_EPS_CMD_GET_VERSION, 0, 0, 1, "get version error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_version(&data);
    UtAssert_True(result == OS_ERROR, "eps get version i2c error");
    UtAssert_True((int16)data == -1, "eps get version value");
    test_eps_i2c(TEST_EPS_CMD_GET_VERSION, 0, 0, 1, "get version i2c error");
}

/* test get checksum */
static void HWLib_EPS_Test_Get_Checksum(void)
{
    /* test checksum */
    uint32_t exp_data[] = {0xdeadbeef, 0x20};
    uint16 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_checksum(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get checksum success");
    UtAssert_True(data == (exp_data[0] >> 16), "eps get checksum value");
    test_eps_i2c(TEST_EPS_CMD_GET_CHECKSUM, 0, 0, 0, "get checksum");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test checksum error */
    exp_data[0] = EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_checksum(&data);
    UtAssert_True(result == OS_ERROR, "eps get checksum error");
    UtAssert_True((int16)data == -1, "eps get checksum value");
    test_eps_i2c(TEST_EPS_CMD_GET_CHECKSUM, 0, 0, 1, "get checksum error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_checksum(&data);
    UtAssert_True(result == OS_ERROR, "eps get checksum i2c error");
    UtAssert_True((int16)data == -1, "eps get checksum value");
    test_eps_i2c(TEST_EPS_CMD_GET_CHECKSUM, 0, 0, 1, "get checksum i2c error");
}

/* test get telemetry */
static void HWLib_EPS_Test_Get_Telemetry(void)
{
    /* init i2c rx buf */
    uint16_t data[] = {0x032d, 0x00fd, 0x02fd, 0x0230, 0x023a,  /* bcr1 */
                       0x032d, 0x00fd, 0x02fd, 0x0230, 0x023a,  /* bcr2 */
                       0x032d, 0x00fd, 0x02fd, 0x0230, 0x023a,  /* bcr3 */
                       0x03ad, 0x02ad,                          /* idiode */
                       0x01ad, 0x00ad,                          /* 3v3,5v drw */
                       0x02fd, 0x02e8,                          /* pcm 3v3 */
                       0x0351, 0x030e,                          /* pcm 5v */
                       0x0304, 0x0321,                          /* pcm bat */
                       0x0380, 0x0243,                          /* pcm 12v */
                       0x0372, 0x03ad,                          /* pdm 1 */
                       0x0372, 0x03ad,                          /* pdm 2 */
                       0x0332, 0x0125,                          /* pdm 3 */
                       0x02fd, 0x018f,                          /* pdm 4 */
                       0x0332, 0x0125,                          /* pdm 5 */
                       0x0332, 0x0125,                          /* pdm 6 */
                       0x0332, 0x0125,                          /* pdm 7 */
                       0x02fd, 0x018f,                          /* pdm 8 */
                       0x02fd, 0x018f,                          /* pdm 9 */
                       0x02fd, 0x018f,                          /* pdm 10 */
                       0x0313};                                 /* tbrd */

    i2c_data.rxbuf = (const uint8_t*)data;
    i2c_data.rxlen = sizeof(data);
    uint16_t channel;
    int index;

    /* iterate bcrs */
    int i;
    for(i = 0; i < EPS_BCR_COUNT; i++)
    {
        index = (5 * i);
        /* test vbcr tlm */
        channel = TEST_CHANNEL_VBCR + (i << TEST_CHANNEL_BCR_SHIFT);
        test_tlm(channel, data[index], CONV_TABLE[index], "vbcr");

        /* test ibcra tlm */
        channel = TEST_CHANNEL_IBCRA + (i << TEST_CHANNEL_BCR_SHIFT);
        test_tlm(channel, data[index+1], CONV_TABLE[index+1], "ibcra");

        /* test ibcrb tlm */
        channel = TEST_CHANNEL_IBCRB + (i << TEST_CHANNEL_BCR_SHIFT);
        test_tlm(channel, data[index+2], CONV_TABLE[index+2], "ibcrb");

        /* test tbcra tlm */
        channel = TEST_CHANNEL_TBCRA + (i << TEST_CHANNEL_BCR_SHIFT);
        test_tlm(channel, data[index+3], CONV_TABLE[index+3], "tbcra");

        /* test tbcrb tlm */
        channel = TEST_CHANNEL_TBCRB + (i << TEST_CHANNEL_BCR_SHIFT);
        test_tlm(channel, data[index+4], CONV_TABLE[index+4], "tbcrb");
    }

    /* test idiode tlm */
    test_tlm(TEST_CHANNEL_IIDIODE_OUT, data[15], CONV_TABLE[15], "iidiode out");
    test_tlm(TEST_CHANNEL_VIDIODE_OUT, data[16], CONV_TABLE[16], "vidiode out");
    
    /* test current drw tlm */
    test_tlm(TEST_CHANNEL_I3V3_DRW, data[17], CONV_TABLE[17], "i3v3 drw");
    test_tlm(TEST_CHANNEL_I5V_DRW, data[18], CONV_TABLE[18], "i5v drw");

    /* iterate pcm buses */
    for(i = 0; i < 4; i++)
    {
        index = (2*i) + 19;
        /* test vpcm bus tlm */
        channel = TEST_CHANNEL_VPCM + (i << TEST_CHANNEL_PCM_SHIFT);
        test_tlm(channel, data[index], CONV_TABLE[index], "vpcm");

        /* test ipcm bus tlm */
        channel = TEST_CHANNEL_IPCM + (i << TEST_CHANNEL_PCM_SHIFT);
        test_tlm(channel, data[index+1], CONV_TABLE[index+1], "ipcm");
    }

    /* iterate pdm switches */
    for(i = 0; i < 10; i++)
    {
        index = (2*i) + 27;
        /* test vpdm */
        channel = TEST_CHANNEL_VSW + (i << TEST_CHANNEL_PDM_SHIFT);
        test_tlm(channel, data[index], CONV_TABLE[index], "vpdm");

        /* test ipdm */
        channel = TEST_CHANNEL_ISW + (i << TEST_CHANNEL_PDM_SHIFT);
        test_tlm(channel, data[index+1], CONV_TABLE[index+1], "ipdm");
    }

    /* test board tmp tlm */
    test_tlm(TEST_CHANNEL_TBRD, data[47], CONV_TABLE[47], "tbrd");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test tlm error */
    uint16_t exp_data[] = {(uint16_t)EPS_I2C_ERROR, 0x0, 0x03};
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    float tlm;
    int result = EPS_get_telemetry(TEST_CHANNEL_TBRD, &tlm);
    UtAssert_True(result == OS_ERROR, "eps get tlm error");
    UtAssert_DoubleCmpAbs(tlm, 0.0f, EPS_TOLERANCE, "eps get tlm value");
    test_eps_i2c(TEST_EPS_CMD_GET_TELEMETRY, TEST_CHANNEL_TBRD >> 8, TEST_CHANNEL_TBRD & 0xff, 1, "get tlm error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_telemetry(TEST_CHANNEL_TBRD, &tlm);
    UtAssert_True(result == OS_ERROR, "eps get tlm error");
    UtAssert_DoubleCmpAbs(tlm, 0.0f, EPS_TOLERANCE, "eps get tlm value");
    test_eps_i2c(TEST_EPS_CMD_GET_TELEMETRY, TEST_CHANNEL_TBRD >> 8, TEST_CHANNEL_TBRD & 0xff, 1, "get tlm error");
}

/* test get wdt period */
static void HWLib_EPS_Test_Get_Watchdog_Period(void)
{
    /* test wdt period */
    uint16 exp_data[] = {0xaced, 0x00, 0x20};
    uint16 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_comms_watchdog_period(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get wdt period success");
    UtAssert_True(data == exp_data[0], "eps get wdt period value");
    test_eps_i2c(TEST_EPS_CMD_GET_COM_WATCHDOG_PERIOD, 0, 0, 0, "get wdt period");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test wdt period error */
    exp_data[0] = (uint16)EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_comms_watchdog_period(&data);
    UtAssert_True(result == OS_ERROR, "eps get wdt period error");
    UtAssert_True((int16)data == -1, "eps get wdt period value");
    test_eps_i2c(TEST_EPS_CMD_GET_COM_WATCHDOG_PERIOD, 0, 0, 1, "get wdt period error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_comms_watchdog_period(&data);
    UtAssert_True(result == OS_ERROR, "eps get wdt period i2c error");
    UtAssert_True((int16)data == -1, "eps get wdt period value");
    test_eps_i2c(TEST_EPS_CMD_GET_COM_WATCHDOG_PERIOD, 0, 0, 1, "get wdt period i2c error");
}

/* test set wdt period */
static void HWLib_EPS_Test_Set_Watchdog_Period(void)
{
    /* valid timeouts */
    uint8_t timeout[] = {1, 10, 90};
    int i;
    for(i = 0; i < 3; i++)
    {
        /* test wdt period */
        int result = EPS_set_comms_watchdog_period(timeout[i]);
        UtAssert_True(result == OS_SUCCESS, "eps set wdt period success");
        test_eps_i2c(TEST_EPS_CMD_SET_COM_WATCHDOG_PERIOD, timeout[i], 0, 0, "set wdt period");

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* invalid timeouts */
    uint8_t invalid_timeout[] = {0, 91};
    for(i = 0; i < 2; i++)
    {
        /* test invalid wdt period */
        int result = EPS_set_comms_watchdog_period(invalid_timeout[i]);
        UtAssert_True(result == OS_ERROR, "eps set wdt period error");
        UtAssert_True(i2c_data.cmd_cnt == 0, "eps no set wdt period i2c cmd");

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    int result = EPS_set_comms_watchdog_period(10);
    UtAssert_True(result == OS_ERROR, "eps set wdt period i2c error");
    test_eps_i2c(TEST_EPS_CMD_SET_COM_WATCHDOG_PERIOD, 10, 0, 1, "set wdt period i2c error");
}

/* test reset wdt */
static void HWLib_EPS_Test_Reset_Watchdog(void)
{
    /* test reset wdt */
    int result = EPS_reset_comms_watchdog();
    UtAssert_True(result == OS_SUCCESS, "eps reset wdt success");
    test_eps_i2c(TEST_EPS_CMD_RESET_COM_WATCHDOG, 0, 0, 0, "reset wdt");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_reset_comms_watchdog();
    UtAssert_True(result == OS_ERROR, "eps reset wdt i2c error");
    test_eps_i2c(TEST_EPS_CMD_RESET_COM_WATCHDOG, 0, 0, 1, "reset wdt i2c error");
}

/* test get num brown out resets */
static void HWLib_EPS_Test_Get_Num_BOR(void)
{
    /* test num brown out resets */
    uint32 exp_data[] = {0xdeadbeef, 0x0};
    uint16 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_num_brownout_resets(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get num brown out resets success");
    UtAssert_True(data == (exp_data[0] >> 16), "eps get num brown out resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_BOR, 0, 0, 0, "get num brown out resets");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test num brown out resets error */
    exp_data[0] = EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_num_brownout_resets(&data);
    UtAssert_True(result == OS_ERROR, "eps get num brown out resets error");
    UtAssert_True((int16)data == -1, "eps get num brown out resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_BOR, 0, 0, 1, "get num brown out resets error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_num_brownout_resets(&data);
    UtAssert_True(result == OS_ERROR, "eps get num brown out resets i2c error");
    UtAssert_True((int16)data == -1, "eps get num brown out resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_BOR, 0, 0, 1, "get num brown out resets i2c error");
}

/* test get num auto sw resets */
static void HWLib_EPS_Test_Get_Num_ASR(void)
{
    /* test num sw resets */
    uint32 exp_data[] = {0xdeadbeef, 0x0};
    uint16 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_num_auto_software_resets(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get num sw resets success");
    UtAssert_True(data == (exp_data[0] >> 16), "eps get num sw resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_ASR, 0, 0, 0, "get num sw resets");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test num sw resets error */
    exp_data[0] = EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_num_auto_software_resets(&data);
    UtAssert_True(result == OS_ERROR, "eps get num sw resets error");
    UtAssert_True((int16)data == -1, "eps get num sw resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_ASR, 0, 0, 1, "get num sw resets error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_num_auto_software_resets(&data);
    UtAssert_True(result == OS_ERROR, "eps get num sw resets i2c error");
    UtAssert_True((int16)data == -1, "eps get num sw resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_ASR, 0, 0, 1, "get num sw resets i2c error");
}

/* test get num manual resets */
static void HWLib_EPS_Test_Get_Num_MR(void)
{
    /* test num manual resets */
    uint32 exp_data[] = {0xdeadbeef, 0x0};
    uint16 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_num_manual_resets(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get num manual resets success");
    UtAssert_True(data == (exp_data[0] >> 16), "eps get num manual resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_MR, 0, 0, 0, "get num manual resets");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test num manual resets error */
    exp_data[0] = EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_num_manual_resets(&data);
    UtAssert_True(result == OS_ERROR, "eps get num manual resets error");
    UtAssert_True((int16)data == -1, "eps get num manual resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_MR, 0, 0, 1, "get num manual resets error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_num_manual_resets(&data);
    UtAssert_True(result == OS_ERROR, "eps get num manual resets i2c error");
    UtAssert_True((int16)data == -1, "eps get num manual resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_MR, 0, 0, 1, "get num manual resets i2c error");
}

/* test get num wdt resets */
static void HWLib_EPS_Test_Get_Num_WDTR(void)
{
    /* test num wdt resets */
    uint16 exp_data[] = {0xdead, 0x0, 0x0};
    uint16 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_num_comms_watchdog_resets(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get num wdt resets success");
    UtAssert_True(data == exp_data[0], "eps get num wdt resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_CWDR, 0, 0, 0, "get num wdt resets");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test num wdt resets error */
    exp_data[0] = (uint16)(EPS_I2C_ERROR >> 16);
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_num_comms_watchdog_resets(&data);
    UtAssert_True(result == OS_ERROR, "eps get num wdt resets error");
    UtAssert_True((int16)data == -1, "eps get num wdt resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_CWDR, 0, 0, 1, "get num wdt resets error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_num_comms_watchdog_resets(&data);
    UtAssert_True(result == OS_ERROR, "eps get num wdt resets i2c error");
    UtAssert_True((int16)data == -1, "eps get num wdt resets value");
    test_eps_i2c(TEST_EPS_CMD_GET_NUMBER_OF_CWDR, 0, 0, 1, "get num wdt resets i2c error");
}

/* test all pdm on */
static void HWLib_EPS_Test_PDM_All_On(void)
{
    /* test pdm all on */
    int result = EPS_switch_all_PDM_on();
    UtAssert_True(result == OS_SUCCESS, "eps pdm all on success");
    test_eps_i2c(TEST_EPS_CMD_SWITCH_ON_ALL_PDM, 0, 0, 0, "pdm all on");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_switch_all_PDM_on();
    UtAssert_True(result == OS_ERROR, "eps pdm all on i2c error");
    test_eps_i2c(TEST_EPS_CMD_SWITCH_ON_ALL_PDM, 0, 0, 1, "pdm all on error");
}

/* test all pdm off */
static void HWLib_EPS_Test_PDM_All_Off(void)
{
    /* test pdm all off */
    int result = EPS_switch_all_PDM_off();
    UtAssert_True(result == OS_SUCCESS, "eps pdm all off success");
    test_eps_i2c(TEST_EPS_CMD_SWITCH_OFF_ALL_PDM, 0, 0, 0, "pdm all off");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_switch_all_PDM_off();
    UtAssert_True(result == OS_ERROR, "eps pdm all off i2c error");
    test_eps_i2c(TEST_EPS_CMD_SWITCH_OFF_ALL_PDM, 0, 0, 1, "pdm all off i2c error");
}

/* test get pdm all actual state */
static void HWLib_EPS_Test_Get_PDM_All_Actual_State(void)
{
    /* test get pdm all actual state */
    uint32 exp_data[] = {0xdeadbeef, 0x0};
    uint32 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_all_PDM_actual_state(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get pdm all actual state success");
    UtAssert_True(data == (exp_data[0] & 0x7fe), "eps get pdm all actual state value");
    test_eps_i2c(TEST_EPS_CMD_GET_ACTUAL_STATE_ALL_PDM, 0, 0, 0, "get pdm all actual state");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test get pdm all actual state error */
    exp_data[0] = EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_all_PDM_actual_state(&data);
    UtAssert_True(result == OS_ERROR, "eps get pdm all actual state error");
    UtAssert_True((int32)data == -1, "eps get pdm all actual state value");
    test_eps_i2c(TEST_EPS_CMD_GET_ACTUAL_STATE_ALL_PDM, 0, 0, 1, "get pdm all actual state error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_all_PDM_actual_state(&data);
    UtAssert_True(result == OS_ERROR, "eps get pdm all actual state i2c error");
    UtAssert_True((int32)data == -1, "eps get pdm all actual state value");
    test_eps_i2c(TEST_EPS_CMD_GET_ACTUAL_STATE_ALL_PDM, 0, 0, 1, "get pdm all actual state i2c error");
}

/* test get pdm all expected state */
static void HWLib_EPS_Test_Get_PDM_All_Expected_State(void)
{
    /* test get pdm all expected state */
    uint32 exp_data[] = {0xdeadbeef, 0x0};
    uint32 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_all_PDM_expected_state(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get pdm all expected state success");
    UtAssert_True(data == (exp_data[0] & 0x7fe), "eps get pdm all expected state value");
    test_eps_i2c(TEST_EPS_CMD_GET_EXPECTED_STATE_ALL_PDM, 0, 0, 0, "get pdm all expected state");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test get pdm all expected state error */
    exp_data[0] = EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_all_PDM_expected_state(&data);
    UtAssert_True(result == OS_ERROR, "eps get pdm all expected state error");
    UtAssert_True((int32)data == -1, "eps get pdm all expected state value");
    test_eps_i2c(TEST_EPS_CMD_GET_EXPECTED_STATE_ALL_PDM, 0, 0, 1, "get pdm all expected state error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_all_PDM_expected_state(&data);
    UtAssert_True(result == OS_ERROR, "eps get pdm all expected state i2c error");
    UtAssert_True((int32)data == -1, "eps get pdm all expected state value");
    test_eps_i2c(TEST_EPS_CMD_GET_EXPECTED_STATE_ALL_PDM, 0, 0, 1, "get pdm all expected state i2c error");
}

/* test get pdm all init state */
static void HWLib_EPS_Test_Get_PDM_All_Initial_State(void)
{
    /* test get pdm all init state */
    uint32 exp_data[] = {0xdeadbeef, 0x0};
    uint32 data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    int result = EPS_get_PDM_init_state(&data);
    UtAssert_True(result == OS_SUCCESS, "eps get pdm all init state success");
    UtAssert_True(data == (exp_data[0] & 0x7fe), "eps get pdm all init state value");
    test_eps_i2c(TEST_EPS_CMD_GET_INIT_STATE_ALL_PDM, 0, 0, 0, "get pdm all init state");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test get pdm all init state error */
    exp_data[0] = EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_PDM_init_state(&data);
    UtAssert_True(result == OS_ERROR, "eps get pdm all init state error");
    UtAssert_True((int32)data == -1, "eps get pdm all init state value");
    test_eps_i2c(TEST_EPS_CMD_GET_INIT_STATE_ALL_PDM, 0, 0, 1, "get pdm all init state error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_PDM_init_state(&data);
    UtAssert_True(result == OS_ERROR, "eps get pdm all init state i2c error");
    UtAssert_True((int32)data == -1, "eps get pdm all init state value");
    test_eps_i2c(TEST_EPS_CMD_GET_INIT_STATE_ALL_PDM, 0, 0, 1, "get pdm all init state i2c error");
}

/* test set pdm all init state */
static void HWLib_EPS_Test_Set_PDM_All_Initial_State(void)
{
    /* test set pdm all init state */
    int result = EPS_set_all_PDM_init_state();
    UtAssert_True(result == OS_SUCCESS, "eps set pdm all init state success");
    test_eps_i2c(TEST_EPS_CMD_SET_ALL_PDM_TO_INIT_STATE, 0, 0, 0, "set pdm all init state");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_set_all_PDM_init_state();
    UtAssert_True(result == OS_ERROR, "eps set pdm all init state i2c error");
    test_eps_i2c(TEST_EPS_CMD_SET_ALL_PDM_TO_INIT_STATE, 0, 0, 1, "set pdm all init state i2c error");
}

/* test pdm on */
static void HWLib_EPS_Test_PDM_On(void)
{
    /* test valid/invalid pdm channels */
    int i;
    for(i = 0; i < 15; i++)
    {
        /* test pdm on */
        int result = EPS_switch_PDM_on((uint8)i);

        /* valid channels */
        if((i >= 1) && (i <= 10))
        {
            UtAssert_True(result == OS_SUCCESS, "eps pdm on success");
            test_eps_i2c(TEST_EPS_CMD_SWITCH_PDM_N_ON, i, 0, 0, "pdm on");
        }
        else
        {
            UtAssert_True(result == OS_ERROR, "eps pdm on error");
        }

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    int result = EPS_switch_PDM_on(5);
    UtAssert_True(result == OS_ERROR, "eps pdm on i2c error");
    test_eps_i2c(TEST_EPS_CMD_SWITCH_PDM_N_ON, 5, 0, 1, "pdm on i2c error");
}

/* test pdm off */
static void HWLib_EPS_Test_PDM_Off(void)
{
    /* test valid/invalid pdm channels */
    int i;
    for(i = 0; i < 15; i++)
    {
        /* test pdm off */
        int result = EPS_switch_PDM_off((uint8)i);

        /* valid channels */
        if((i >= 1) && (i <= 10))
        {
            UtAssert_True(result == OS_SUCCESS, "eps pdm off success");
            test_eps_i2c(TEST_EPS_CMD_SWITCH_PDM_N_OFF, i, 0, 0, "pdm off");
        }
        else
        {
            UtAssert_True(result == OS_ERROR, "eps pdm off error");
        }

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    int result = EPS_switch_PDM_off(5);
    UtAssert_True(result == OS_ERROR, "eps pdm off i2c error");
    test_eps_i2c(TEST_EPS_CMD_SWITCH_PDM_N_OFF, 5, 0, 1, "pdm off i2c error");
}

/* test set pdm init state on */
static void HWLib_EPS_Test_Set_PDM_Init_State_On(void)
{
    /* test valid/invalid pdm channels */
    int i;
    for(i = 0; i < 15; i++)
    {
        /* test set pdm init state on */
        int result = EPS_set_PDM_init_state_on((uint8)i);

        /* valid channels */
        if((i >= 1) && (i <= 10))
        {
            UtAssert_True(result == OS_SUCCESS, "eps set pdm init state on success");
            test_eps_i2c(TEST_EPS_CMD_SET_PDM_N_INIT_STATE_ON, i, 0, 0, "set pdm init state on");
        }
        else
        {
            UtAssert_True(result == OS_ERROR, "eps set pdm init state on error");
        }

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    int result = EPS_set_PDM_init_state_on(5);
    UtAssert_True(result == OS_ERROR, "eps set pdm init state on i2c error");
    test_eps_i2c(TEST_EPS_CMD_SET_PDM_N_INIT_STATE_ON, 5, 0, 1, "set pdm init state on i2c error");
}

/* test set pdm init state off */
static void HWLib_EPS_Test_Set_PDM_Init_State_Off(void)
{
    /* test valid/invalid pdm channels */
    int i;
    for(i = 0; i < 15; i++)
    {
        /* test set pdm init state off */
        int result = EPS_set_PDM_init_state_off((uint8)i);

        /* valid channels */
        if((i >= 1) && (i <= 10))
        {
            UtAssert_True(result == OS_SUCCESS, "eps set pdm init state off success");
            test_eps_i2c(TEST_EPS_CMD_SET_PDM_N_INIT_STATE_OFF, i, 0, 0, "set pdm init state off");
        }
        else
        {
            UtAssert_True(result == OS_ERROR, "eps set pdm init state off error");
        }

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    int result = EPS_set_PDM_init_state_off(5);
    UtAssert_True(result == OS_ERROR, "eps set pdm init state off i2c error");
    test_eps_i2c(TEST_EPS_CMD_SET_PDM_N_INIT_STATE_OFF, 5, 0, 1, "set pdm init state off i2c error");
}

/* test get pdm actual status */
static void HWLib_EPS_Test_Get_PDM_Actual_Status(void)
{
    /* test data */
    uint16 exp_data[] = {0xdead, 0x0, 0x0};
    uint16 data;
    int result;

    /* test valid/invalid pdm channels */
    int i;
    for(i = 0; i < 15; i++)
    {
        /* test get pdm actual status */
        data = 0;
        i2c_data.rxbuf = (uint8_t*)exp_data;
        i2c_data.rxlen = sizeof(exp_data);
        result = EPS_get_PDM_actual_status((uint8)i, &data);

        /* valid channels */
        if((i >= 1) && (i <= 10))
        {
            UtAssert_True(result == OS_SUCCESS, "eps get pdm actual status success");
            UtAssert_True(data == exp_data[0], "eps get pdm actual status value");
            test_eps_i2c(TEST_EPS_CMD_GET_PDM_N_ACTUAL_STATUS, i, 0, 0, "get pdm actual status");
        }
        else
        {
            UtAssert_True(result == OS_ERROR, "eps get pdm actual status error");
        }

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* test get pdm actual status error */
    exp_data[0] = (uint16)EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_PDM_actual_status(5, &data);
    UtAssert_True(result == OS_ERROR, "eps get pdm actual status error");
    UtAssert_True((int16)data == -1, "eps get pdm actual status value");
    test_eps_i2c(TEST_EPS_CMD_GET_PDM_N_ACTUAL_STATUS, 5, 0, 1, "get pdm actual status error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_PDM_actual_status(5, &data);
    UtAssert_True(result == OS_ERROR, "eps get pdm actual status i2c error");
    UtAssert_True((int16)data == -1, "eps get pdm actual status value");
    test_eps_i2c(TEST_EPS_CMD_GET_PDM_N_ACTUAL_STATUS, 5, 0, 1, "get pdm actual status i2c error");
}

/* test set pdm timer limit */
static void HWLib_EPS_Test_Set_PDM_Timer_Limit(void)
{
    /* test valid/invalid pdm channels */
    int i;
    for(i = 0; i < 15; i++)
    {
        /* test valid/invalid timer limits */
        int j;
        for(j = 0; j < 0xff; j++)
        {
            /* test set pdm timer limit */
            uint16 data = (i << 8) | j;
            int result = EPS_set_PDM_timer_limit(data);

            /* valid channels and limits */
            if((i < 1) || (i > 10))
            {
                UtAssert_True(result == OS_ERROR, "eps set pdm timer limit error");
            }
            else if((j != 0) && (j < 0x0a))
            {
                UtAssert_True(result == OS_ERROR, "eps set pdm timer limit error");
            }
            else
            {
                UtAssert_True(result == OS_SUCCESS, "eps set pdm timer limit success");
                test_eps_i2c(TEST_EPS_CMD_SET_PDM_N_TIMER_LIMIT, i, j, 0, "set pdm timer limit");
            }

            /* reset i2c data */
            HWLib_Reset_I2C_Data();
        }
    }

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    uint16 data = (5 << 8) | 0xa0;
    int result = EPS_set_PDM_timer_limit(data);
    UtAssert_True(result == OS_ERROR, "eps set pdm timer limit i2c error");
    test_eps_i2c(TEST_EPS_CMD_SET_PDM_N_TIMER_LIMIT, 5, 0xa0, 1, "set pdm timer limit i2c error");
}

/* test get pdm timer limit */
static void HWLib_EPS_Test_Get_PDM_Timer_Limit(void)
{
    /* test data */
    uint16 exp_data[] = {0xdead, 0x0, 0x0};
    uint16 data;
    int result;

    /* test valid/invalid pdm channels */
    int i;
    for(i = 0; i < 15; i++)
    {
        /* test get pdm timer limit */
        data = 0;
        i2c_data.rxbuf = (uint8_t*)exp_data;
        i2c_data.rxlen = sizeof(exp_data);
        result = EPS_get_PDM_timer_limit(i, &data);

        /* valid channels */
        if((i >= 1) && (i <= 10))
        {
            UtAssert_True(result == OS_SUCCESS, "eps get pdm timer limit success");
            UtAssert_True(data == exp_data[0], "eps get pdm timer limit value");
            test_eps_i2c(TEST_EPS_CMD_GET_PDM_N_TIMER_LIMIT, i, 0, 0, "get pdm timer limit");
        }
        else
        {
            UtAssert_True(result == OS_ERROR, "eps get pdm timer limit error");
        }

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* test get pdm timer limit error */
    exp_data[0] = (uint16)EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_PDM_timer_limit(5, &data);
    UtAssert_True(result == OS_ERROR, "eps get pdm timer limit error");
    UtAssert_True((int16)data == -1, "eps get pdm timer limit value");
    test_eps_i2c(TEST_EPS_CMD_GET_PDM_N_TIMER_LIMIT, 5, 0, 1, "get pdm timer limit error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_PDM_timer_limit(5, &data);
    UtAssert_True(result == OS_ERROR, "eps get pdm timer limit i2c error");
    UtAssert_True((int16)data == -1, "eps get pdm timer limit value");
    test_eps_i2c(TEST_EPS_CMD_GET_PDM_N_TIMER_LIMIT, 5, 0, 1, "get pdm timer limit i2c error");
}

/* test get pdm timer value */
static void HWLib_EPS_Test_Get_PDM_Timer_Value(void)
{
    /* test data */
    uint16 exp_data[] = {0xdead, 0x0, 0x0};
    uint16 data;
    int result;

    /* test valid/invalid pdm channels */
    int i;
    for(i = 0; i < 15; i++)
    {
        /* test get pdm timer current value */
        data = 0;
        i2c_data.rxbuf = (uint8_t*)exp_data;
        i2c_data.rxlen = sizeof(exp_data);
        result = EPS_get_PDM_current_timer_val(i, &data);

        /* valid channels */
        if((i >= 1) && (i <= 10))
        {
            UtAssert_True(result == OS_SUCCESS, "eps get pdm timer value success");
            UtAssert_True(data == exp_data[0], "eps get pdm timer value value");
            test_eps_i2c(TEST_EPS_CMD_GET_PDM_N_CURRENT_TIMER_VAL, i, 0, 0, "get pdm timer value");
        }
        else
        {
            UtAssert_True(result == OS_ERROR, "eps get pdm timer value error");
        }

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* test get pdm timer current value error */
    exp_data[0] = (uint16)EPS_I2C_ERROR;
    data = 0;
    i2c_data.rxbuf = (uint8_t*)exp_data;
    i2c_data.rxlen = sizeof(exp_data);

    result = EPS_get_PDM_current_timer_val(5, &data);
    UtAssert_True(result == OS_ERROR, "eps get pdm timer value error");
    UtAssert_True((int16)data == -1, "eps get pdm timer value value");
    test_eps_i2c(TEST_EPS_CMD_GET_PDM_N_CURRENT_TIMER_VAL, 5, 0, 1, "get pdm timer value error");

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_get_PDM_current_timer_val(5, &data);
    UtAssert_True(result == OS_ERROR, "eps get pdm timer value i2c error");
    UtAssert_True((int16)data == -1, "eps get pdm timer value value");
    test_eps_i2c(TEST_EPS_CMD_GET_PDM_N_CURRENT_TIMER_VAL, 5, 0, 1, "get pdm timer value i2c error");
}

/* test reset pcm */
static void HWLib_EPS_Test_Reset_PCM(void)
{
    /* test valid/invalid pcm channels */
    int i;
    for(i = 0; i < 20; i++)
    {
        /* test pdm off */
        int result = EPS_reset_PCM((uint8)i);

        /* valid channels */
        if((i >= 0x01) && (i <= 0x0f))
        {
            UtAssert_True(result == OS_SUCCESS, "eps reset pcm success");
            test_eps_i2c(TEST_EPS_CMD_PCM_RESET, i, 0, 0, "reset pcm");
        }
        else
        {
            UtAssert_True(result == OS_ERROR, "eps reset pcm error");
        }

        /* reset i2c data */
        HWLib_Reset_I2C_Data();
    }

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    int result = EPS_reset_PCM(5);
    UtAssert_True(result == OS_ERROR, "eps reset pcm i2c error");
    test_eps_i2c(TEST_EPS_CMD_PCM_RESET, 5, 0, 1, "reset pcm i2c error");
}

/* test manual reset */
static void HWLib_EPS_Test_Manual_Reset(void)
{
    /* test manual reset */
    int result = EPS_manual_reset();
    UtAssert_True(result == OS_SUCCESS, "eps manual reset success");
    test_eps_i2c(TEST_EPS_CMD_MANUAL_RESET, 0, 0, 0, "manual reset");

    /* reset i2c data */
    HWLib_Reset_I2C_Data();

    /* test i2c error */
    i2c_data.retcode = E_FAIL;
    i2c_data.cmd_cnt = 0;
    result = EPS_manual_reset();
    UtAssert_True(result == OS_ERROR, "eps manual reset i2c error");
    test_eps_i2c(TEST_EPS_CMD_MANUAL_RESET, 0, 0, 1, "manual reset i2c error");
}

void HWLib_EPS_Test_AddTestCases(void)
{
    UtTest_Add(HWLib_EPS_Test_LibInit, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: lib init");
    UtTest_Add(HWLib_EPS_Test_Get_Board_Status, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get board status");
    UtTest_Add(HWLib_EPS_Test_Get_Version, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get version");
    UtTest_Add(HWLib_EPS_Test_Get_Checksum, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get checksum");
    UtTest_Add(HWLib_EPS_Test_Get_Telemetry, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get telemetry");
    UtTest_Add(HWLib_EPS_Test_Get_Watchdog_Period, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get wdt period");
    UtTest_Add(HWLib_EPS_Test_Set_Watchdog_Period, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: set wdt period");
    UtTest_Add(HWLib_EPS_Test_Reset_Watchdog, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: reset wdt");
    UtTest_Add(HWLib_EPS_Test_Get_Num_BOR, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get number brown out resets");
    UtTest_Add(HWLib_EPS_Test_Get_Num_ASR, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get number auto sw resets");
    UtTest_Add(HWLib_EPS_Test_Get_Num_MR, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get number manual resets");
    UtTest_Add(HWLib_EPS_Test_Get_Num_WDTR, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get number wdt resets");
    UtTest_Add(HWLib_EPS_Test_PDM_All_On, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: pdm all on");
    UtTest_Add(HWLib_EPS_Test_PDM_All_Off, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: pdm all off");
    UtTest_Add(HWLib_EPS_Test_Get_PDM_All_Actual_State, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get pdm all actual state");
    UtTest_Add(HWLib_EPS_Test_Get_PDM_All_Expected_State, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get pdm all expected state");
    UtTest_Add(HWLib_EPS_Test_Get_PDM_All_Initial_State, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get pdm all initial state");
    UtTest_Add(HWLib_EPS_Test_Set_PDM_All_Initial_State, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: set pdm all initial state");
    UtTest_Add(HWLib_EPS_Test_PDM_On, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: pdm on");
    UtTest_Add(HWLib_EPS_Test_PDM_Off, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: pdm off");
    UtTest_Add(HWLib_EPS_Test_Set_PDM_Init_State_On, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: set pdm init state on");
    UtTest_Add(HWLib_EPS_Test_Set_PDM_Init_State_Off, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: set pdm init state off");
    UtTest_Add(HWLib_EPS_Test_Get_PDM_Actual_Status, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get pdm actual status");
    UtTest_Add(HWLib_EPS_Test_Set_PDM_Timer_Limit, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: set pdm timer limit");
    UtTest_Add(HWLib_EPS_Test_Get_PDM_Timer_Limit, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get pdm timer limit");
    UtTest_Add(HWLib_EPS_Test_Get_PDM_Timer_Value, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: get pdm timer current value");
    UtTest_Add(HWLib_EPS_Test_Reset_PCM, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: reset pcm");
    UtTest_Add(HWLib_EPS_Test_Manual_Reset, HWLib_Test_Setup, HWLib_Test_TearDown,
               "hwlib eps: manual reset");
}

