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

#ifndef _BAT_LIB_H_
#define _BAT_LIB_H_

extern int32 BAT_LibInit(void);
extern int32 BAT_get_last_error(uint8 channel, uint8 battery);
extern int32 BAT_get_board_status(uint16 *status, uint8 battery);
extern int32 BAT_get_version(uint16 *version, uint8 battery);
extern int32 BAT_get_checksum(uint16 *checksum, uint8 battery);
extern int32 BAT_manual_reset(uint8 battery);
extern int32 BAT_get_telemetry(uint8 channel, float *data_output, uint8 battery);
extern int32 BAT_get_num_brownout_resets(uint16 *data_output, uint8 battery);
extern int32 BAT_get_num_auto_software_resets(uint16 *data_output, uint8 battery);
extern int32 BAT_get_num_manual_resets(uint16 *data_output, uint8 battery);
extern int32 BAT_get_heater_controller_status(uint16 *status, uint8 battery);
extern int32 BAT_set_heater_controller_status(uint8 heater_code, uint8 battery);

#endif
