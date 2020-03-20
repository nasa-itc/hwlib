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

/* EPS_LIB Event IDs */
#define EPSLIB_I2C_ERR_EID   1000
#define EPSLIB_TLM_ERR_EID   1001
#define EPSLIB_MUTEX_ERR_EID 1002 

/*************************************************************************
** Data 
*************************************************************************/

#ifndef _eps_lib_h_
#define _eps_lib_h_

#include "cfe.h"

extern int32 EPS_LibInit(void);
extern int32 EPS_get_last_error(uint8 channel);
extern int32 EPS_get_board_status(uint8 *status);
extern int32 EPS_get_version(uint16 *data_output);
extern int32 EPS_get_checksum(uint16 *data_output);
extern int32 EPS_get_telemetry(uint16 channel, float *data_output);
extern int32 EPS_get_comms_watchdog_period(uint16 *data_output);
extern int32 EPS_set_comms_watchdog_period(uint8 period);
extern int32 EPS_reset_comms_watchdog(void);
extern int32 EPS_get_num_brownout_resets(uint16 *data_output);
extern int32 EPS_get_num_auto_software_resets(uint16 *data_output);
extern int32 EPS_get_num_manual_resets(uint16 *data_output);
extern int32 EPS_get_num_comms_watchdog_resets(uint16 *data_output);
extern int32 EPS_switch_all_PDM_on(void);
extern int32 EPS_switch_all_PDM_off(void);
extern int32 EPS_get_all_PDM_actual_state(uint32 *data_output);
extern int32 EPS_get_all_PDM_expected_state(uint32 *data_output);
extern int32 EPS_get_PDM_init_state(uint32 *data_output);
extern int32 EPS_set_all_PDM_init_state(void);
extern int32 EPS_switch_PDM_on(uint8 pdm_channel);
extern int32 EPS_switch_PDM_off(uint8 pdm_channel);
extern int32 EPS_set_PDM_init_state_on(uint8 pdm_channel);
extern int32 EPS_set_PDM_init_state_off(uint8 pdm_channel);
extern int32 EPS_get_PDM_actual_status(uint8 pdm_channel, uint16 *data_output);
extern int32 EPS_set_PDM_timer_limit(uint16 data);
extern int32 EPS_get_PDM_timer_limit(uint8 pdm_channel, uint16 *data_output);
extern int32 EPS_get_PDM_current_timer_val(uint8 pdm_channel, uint16 *data_output);
extern int32 EPS_reset_PCM(uint8 pcm_channel);
extern int32 EPS_manual_reset(void);

#endif 
