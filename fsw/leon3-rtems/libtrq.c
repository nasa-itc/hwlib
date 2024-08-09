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

#include "libtrq.h"

#define TRQ_FNAME_SIZE 20

#define TRQ_READ_REG(offset)              *((volatile uint32_t *)(offset))
#define TRQ_WRITE_REG(offset,data)        *((volatile uint32_t *)(offset)) = (data)


/* Get dword from reg, then read bits specified by 'mask' from 'dword' and shift down by 'shift'. */
#define TRQ_FPGA_RD_REG_BITS(reg,mask,shift) \
   (((TRQ_READ_REG(reg)) & (mask)) >> (shift))

/* Shift 'value' up by 'shift' and write to those bits in 'dword' that
** are specified by 'mask'.  Other bits in 'dword' are unchanged. Then 
   write dword to reg.  */
#define TRQ_FPGA_WR_REG_BITS(reg,mask,shift,value)  \
TRQ_WRITE_REG(reg, (uint32_t)((TRQ_READ_REG(reg) & ~mask) | (((value) & (mask >> shift)) << shift)) )


#define TRQ_FPGA_PROC_SHARED_REG_FILE3	                  0x80500200

/*
** PWM Bridge Register
*/
#define TRQ_FPGA_PWM_HBRIDGE_REGISTER                     (TRQ_FPGA_PROC_SHARED_REG_FILE3 + 0x20)  /* PWM HBridge Register */
#define TRQ_FPGA_PWM_HBRIDGE_ENABLE                       1
#define TRQ_FPGA_PWM_HBRIDGE_DISABLE                      0
#define TRQ_FPGA_PWM_HBRIDGE_REVERSE_DIR                  1
#define TRQ_FPGA_PWM_HBRIDGE_FORWARD_DIR                  0
#define TRQ_FPGA_PWM_HBRIDGE_1_DIR_MASK                   0x00000001
#define TRQ_FPGA_PWM_HBRIDGE_2_DIR_MASK                   0x00000002
#define TRQ_FPGA_PWM_HBRIDGE_3_DIR_MASK                   0x00000004
#define TRQ_FPGA_PWM_HBRIDGE_1_ENA_MASK                   0x00010000
#define TRQ_FPGA_PWM_HBRIDGE_2_ENA_MASK                   0x00020000
#define TRQ_FPGA_PWM_HBRIDGE_3_ENA_MASK                   0x00040000

#define TRQ_PWM_HBRIDGE_GPIO8_B1_ENABLE(port, value)           TRQ_FPGA_WR_REG_BITS(TRQ_FPGA_PWM_HBRIDGE_REGISTER, TRQ_FPGA_PWM_HBRIDGE_1_ENA_MASK, 16, value)
#define TRQ_PWM_HBRIDGE_GPIO9_B1_ENABLE(port, value)           TRQ_FPGA_WR_REG_BITS(TRQ_FPGA_PWM_HBRIDGE_REGISTER, TRQ_FPGA_PWM_HBRIDGE_2_ENA_MASK, 17, value)
#define TRQ_PWM_HBRIDGE_GPIO10_B1_ENABLE(port, value)          TRQ_FPGA_WR_REG_BITS(TRQ_FPGA_PWM_HBRIDGE_REGISTER, TRQ_FPGA_PWM_HBRIDGE_3_ENA_MASK, 18, value)
#define TRQ_PWM_HBRIDGE_GPIO8_B1_DIR(port, value)              TRQ_FPGA_WR_REG_BITS(TRQ_FPGA_PWM_HBRIDGE_REGISTER, TRQ_FPGA_PWM_HBRIDGE_1_DIR_MASK, 0, value)
#define TRQ_PWM_HBRIDGE_GPIO9_B1_DIR(port, value)              TRQ_FPGA_WR_REG_BITS(TRQ_FPGA_PWM_HBRIDGE_REGISTER, TRQ_FPGA_PWM_HBRIDGE_2_DIR_MASK, 1, value)
#define TRQ_PWM_HBRIDGE_GPIO10_B1_DIR(port, value)             TRQ_FPGA_WR_REG_BITS(TRQ_FPGA_PWM_HBRIDGE_REGISTER, TRQ_FPGA_PWM_HBRIDGE_3_DIR_MASK, 2, value)

#define TRQ_PWM_NANOSECONDS_TO_MILLISECONDS       1000000

uint32_t trq_hwlib_grpwm_driver_init = FALSE;
uint32_t trq_hwlib_nscalers;
struct grpwm_ioctl_cap cap;

#define FREQ_HZ     48000000
#define SCALER_FREQ 48000

int trq_device; 


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *
 * trq_update_channel():         Update and enable pwm channel. 
 * 
 * Inputs:              int fd                        - channel file descriptor  
 *                      int channel                   -  pwm channel
 *                      grpwm_ioctl_update_chan *chan -  pwm update pointer
 * Outputs:             
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static int trq_update_channel(int fd, int channel, struct grpwm_ioctl_update_chan *chan)
{
	int status;
	struct grpwm_ioctl_update up;

	up.chanmask = 1<<channel;
	up.channels[channel] = *chan;

	status = ioctl(fd, GRPWM_IOCTL_UPDATE, &up);
	if ( status < 0 ) {
		printf("Failed to update channel %d: %d (%s)\n", channel, errno, strerror(errno));
		return -1;
	}
	return 0;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *
 * trq_enablePwmChannel():         configure and enable PWM channel. 
 * 
 * Inputs:              trq_info_t *device - TRQ device info structure for PWM device 
 * Outputs:             
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static void trq_enablePwmChannel(trq_info_t* device)
{
    struct grpwm_ioctl_update_chan val;
    uint32_t period;
    uint32_t timeHigh;
    uint32_t ms;

    if(!device->enabled)
    {
        return; 
    }

    /*
     * convert time period nanoseconds to milliseconds and calculate period register value
    */
    ms = device->timer_period_ns / TRQ_PWM_NANOSECONDS_TO_MILLISECONDS; 
    period = (ms * 100);

    /*
     * convert time high nanoseconds to milliseconds and calculate compare register value
    */
    ms = ms - (device->timer_high_ns / TRQ_PWM_NANOSECONDS_TO_MILLISECONDS); 
    timeHigh= (ms * 100);

    /* Enable channel */
    val.options =
		GRPWM_UPDATE_OPTION_ENABLE | 
		GRPWM_UPDATE_OPTION_PERIOD | 
		GRPWM_UPDATE_OPTION_COMP |
		GRPWM_UPDATE_OPTION_FIX;
	val.period = period;
	val.compare = timeHigh; /* duty cycle  */
	val.dbcomp = 0;
	val.fix = GRPWM_UPDATE_FIX_DISABLE;
	
	trq_update_channel(trq_device, device->trq_num, &val);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *
 * trq_hbride_enable():         Enable or disable and set specific hbridge direction. 
 * 
 * Inputs:              trq_info_t *device - TRQ device info structure for PWM device 
 *                      uint8_t ena_dis          - hbridge state (0 - disable, 1 - enable)
 * Outputs:             
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static void trq_hbride_enable(trq_info_t* device, uint8_t ena_dis, uint8_t dir)
{

     switch (device->trq_num)
     {
         case 0:
              TRQ_PWM_HBRIDGE_GPIO8_B1_ENABLE(device->trq_num, ena_dis);
              if (ena_dis)
              {
                   TRQ_PWM_HBRIDGE_GPIO8_B1_DIR(device->trq_num, dir);
              }
             
         break;
         
         case 1:
              TRQ_PWM_HBRIDGE_GPIO9_B1_ENABLE(device->trq_num, ena_dis);
              if (ena_dis)
              {
                   TRQ_PWM_HBRIDGE_GPIO9_B1_DIR(device->trq_num, dir);
              }
         break;

         case 2:
              TRQ_PWM_HBRIDGE_GPIO10_B1_ENABLE(device->trq_num, ena_dis);
              if (ena_dis)
              {
                   TRQ_PWM_HBRIDGE_GPIO10_B1_DIR(device->trq_num, dir);
              }
         break;

        default:
        break;

     }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *
 * trq_config_channel():         Configure pwm channel. 
 * 
 * Inputs:              int fd                  - channel file descriptor  
 *                      int channel             -  pwm channel
 *                      unsigned int            - irq config 
 *                      grpwm_ioctl_config *cfg -  pwm config pointer
 * Outputs:             
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static int trq_config_channel(int fd, int channel, unsigned int cfg_irq, struct grpwm_ioctl_config *cfg)
{
	unsigned int irq = (channel<<8) | GRPWM_IRQ_DISABLE;
	int status;

	cfg->channel = channel;
	
	status = ioctl(fd, GRPWM_IOCTL_IRQ, irq);
	if ( status < 0 ) {
		printf("Failed to Disable IRQ on channel %d: %d (%s)\n", channel, errno, strerror(errno));
		return -1;
	}

	status = ioctl(fd, GRPWM_IOCTL_SET_CONFIG, cfg);
	if ( status < 0 ) {
		printf("Failed to Configure channel %d: %d (%s)\n", channel, errno, strerror(errno));
		return -1;
	}

	return 0;
}


static void trq_isr(int channel, void *arg)
{
	printk("PWM_ISR: channel %d, arg: %x\n", channel, (int)arg);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_set_time_high(): Configure the time high per period in nanoseconds for a TRQ device. Time high lengths
 *                      may not exceed a device's period length. 
 *
 * Inputs:              trq_info_t *device      -   TRQ device info structure 
 *                      uint32_t    new_time    -   New time high length for the device period in nanoseconds
 *
 * Outputs:             trq_info_t *device      -   High time set to new_time if successful
 *                      returns int32_t         -   TRQ_ERROR_* type on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_set_time_high(trq_info_t* device, uint32_t new_time) 
{
    if(!device->enabled)
    {
        printf("trq_set_time_high: Error setting trq %d timer period because it's disabled! \n", device->trq_num); 
        return TRQ_ERROR; 
    }

	// Make sure the time high isn't greater than the period
	if(new_time > device->timer_period_ns) 
    {
		printf("trq_set_time_high: Error setting trq %d time high, must not exceed the period! \n", device->trq_num);
		return TRQ_TIME_HIGH_VAL_ERR;
	}

 	device->timer_high_ns = new_time;

    if(device->timer_period_ns && device->timer_high_ns)
    {
        // Enable PWM channel
        trq_enablePwmChannel(device);
    }

     return TRQ_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_set_period():    Configure the period length for a TRQ device in nanoseconds. Note that timer time high 
 *                      is set to zero before this function is called. 
 *
 * Inputs:              trq_info_t *device      -   TRQ device info structure 
 *
 * Outputs:             trq_info_t *device      -   Period set to new_period if successful
 *                      returns int32_t         -   TRQ_ERROR on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_set_period(trq_info_t* device) 
{
    if(!device->enabled)
    {
        return TRQ_ERROR; 
    }

    // Enable PWM channel
    trq_enablePwmChannel(device);

    return TRQ_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_set_direction():    Configure the direction of the TRQ device. 
 *
 * Inputs:              trq_info_t *device      -   TRQ device info structure 
 *                      bool        direction   -   New direction desired for device
 *
 * Outputs:             trq_info_t *device      -   positive_direction set to direction if successful
 *                      returns int32_t         -   TRQ_ERROR on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_set_direction(trq_info_t* device, bool direction) 
{
    /* enable/disable hbridge and change direction */
    if (direction == TRQ_DIR_POSITIVE) 
    {
        trq_hbride_enable(device, TRQ_FPGA_PWM_HBRIDGE_ENABLE, TRQ_FPGA_PWM_HBRIDGE_REVERSE_DIR);
    }
    else 
    {
        trq_hbride_enable(device, TRQ_FPGA_PWM_HBRIDGE_ENABLE, TRQ_FPGA_PWM_HBRIDGE_FORWARD_DIR);
    }
    device->positive_direction = direction;

    return TRQ_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_init():          Opens a TRQ device and initializes it. This function can also be
 *                      used on an already opened TRQ number to reset it. 
 *
 * Inputs:              trq_info_t *device  -   TRQ Device info structure to initialize   
 * 
 * Outputs:             trq_info_t *device  -   Info structure contains AXI timer device file descriptor. 
 *                      returns int32_t     -   <0 on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_init(trq_info_t* device) 
{
    int32_t status = TRQ_SUCCESS;
    int chan;
    char devname[TRQ_FNAME_SIZE];
    char pinname[TRQ_FNAME_SIZE]; 
    struct grpwm_ioctl_cap cap;
    struct grpwm_ioctl_scaler scalers;
    struct grpwm_ioctl_config cfg;
    uint32_t nscalers, i;
   
    if(!device->enabled)
    {              
        printf("trq_init: !device->enabled \n");
        snprintf(devname, TRQ_FNAME_SIZE, "/dev/grpwm%d", 0); 

        if (trq_hwlib_grpwm_driver_init == FALSE)
        {
            printf("trq_init: trq_hwlib_grpwm_driver_init == FALSE \n");
       
            /* Open the grpwm driver */
            trq_device = open("/dev/grpwm0", O_RDWR, 0);
            if(trq_device < 0)
            {
                printf("COULDN'T OPEN GRPWM DRIVER: %s\n", devname); 
                status = TRQ_ERROR;
                return status; 
            }

            /* Get capability register info */
            status = ioctl(trq_device, GRPWM_IOCTL_GET_CAP, &cap);
            if ( status < 0 ) 
            {
                printf("Failed to get capabilities: %d (%s)\n", errno, strerror(errno));
                return TRQ_INIT_ERR;
            }

            trq_hwlib_nscalers = ((cap.pwm >> 13) & 0x7) + 1;
            //        trq_hwlib_nscalers =  1;
                    scalers.index_mask = 0;
            for (i=0; i<trq_hwlib_nscalers; i++) 
            {
                scalers.index_mask |= 1<<i;
                //scalers.values[i] = 500;
                scalers.values[i] = (FREQ_HZ / SCALER_FREQ) - 1;
                printf("Initing scaler %d to %dHz: %d\n", i, SCALER_FREQ, scalers.values[i]);
            }
        
            status = ioctl(trq_device, GRPWM_IOCTL_SET_SCALER, &scalers);
            if ( status < 0 ) 
            {
                printf("Failed to set scalers: %d (%s)\n", errno, strerror(errno));
                return TRQ_INIT_ERR;
            }

            trq_hwlib_grpwm_driver_init = TRUE;
        }
        
        printf("trq_init: trq_hwlib_grpwm_driver_init == TRUE \n");

        /*** Configure channel ***/
        cfg.options = GRPWM_CONFIG_OPTION_ASYMMERTIC | GRPWM_CONFIG_OPTION_POLARITY_LOW | GRPWM_CONFIG_OPTION_SINGLE;
        cfg.dbscaler = 0;
        cfg.scaler_index = 0;
        cfg.irqscaler = 0;
        cfg.isr = trq_isr;
        cfg.isr_arg = 0;
        cfg.wave_activate = 0;
        cfg.wave_synccfg = 0;
        cfg.wave_sync = 0;
        cfg.wave_data_length = 0;
        cfg.wave_data = NULL;

        /* Disable IRQ and set config */
        if ( trq_config_channel(trq_device, device->trq_num, GRPWM_IRQ_DISABLE, &cfg) ) 
        {
            printf("Failed to configure Channel \n");
            return TRQ_INIT_ERR;
        }

        printf("Initialized PWM channel: %s for device->trq_num %d\n", devname, device->trq_num); 
        device->enabled = true;
    }

    // Set timer time high to zero
    status = trq_set_time_high(device, 0);
    if(status != TRQ_SUCCESS)
    {
        printf("Error setting trq %d time high to zero\n", device->trq_num);
        status = TRQ_INIT_ERR;
        return status;
    }
    device->timer_high_ns = 0;  // no pulse

    // Set timer period. Expects timer_period_ns to be set by app. 
    status = trq_set_period(device);
    if(status != TRQ_SUCCESS)
    {
        printf("Error setting trq %d period\n", device->trq_num);
        status = TRQ_INIT_ERR;
        return status;
    }

    return status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * trq_command():   Change TQR period, time high, or direction.
 *
 * Inputs:              trq_info_t* device      -   TQR device info structure to modify
 *                      uint32_t time_period    -   The length of a single pwm cycle (nanoseconds)
 *                      float percent_high      -   Percent of the period to be high
 *                      bool pos_dir            -   Direction - True for positive, False for negative
 *
 * Outputs:             trq_info_t *device      -   Parameters set to new values if successful
 *                      returns int32_t         -   TRQ_ERROR_* type on failure, TRQ_SUCCESS on success
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t trq_command(trq_info_t *device, uint8_t percent_high, bool pos_dir)
{
    int32_t status = TRQ_SUCCESS;
    uint32_t TimeHigh;
    int low_timer = 0;
    char charVal;

    if(!device->enabled)
    {
        status=TRQ_ERROR; 
        printf("trq_command: Error device not enabled! \n");
        return status;
    }        
    
    // Calculate time high
    if (percent_high > 100)
    {
        printf("trq_command: Error setting percent high greater than 100! \n");
        return TRQ_ERROR;
    }

    // Modified the following to avoid use of floats
    // TimeHigh = device->timer_period_ns * (percent_high / 100.00);
    TimeHigh = (device->timer_period_ns / 100) * percent_high;
    
    // Change time high? 
    if(device->timer_high_ns != TimeHigh)
    {
        status=trq_set_time_high(device, TimeHigh);
        if(status!=TRQ_SUCCESS)
        {
            status=TRQ_ERROR; 
            return status;
        }
        device->timer_high_ns = TimeHigh;
    }

    // Change direction true (1) is forward 
    if(device->positive_direction!=pos_dir)
    {
        // Set direction
        status = trq_set_direction(device, pos_dir);
        if(status != TRQ_SUCCESS)
        {
            printf("trq_command: Error setting trq %d direction! \n", device->trq_num);
            status = TRQ_INIT_ERR;
            return status;
        }
    }

    return status;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *
 * trq_close():         Disables and closes an active PWM device. 
 * 
 * Inputs:              trq_info_t *device - TRQ device info structure for PWM device to disable
 * Outputs:             trq_info_t *device - Enabled flag set to false, PWM disabled 
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void trq_close(trq_info_t* device)
{
   struct grpwm_ioctl_update_chan val;

    if(!device->enabled)
    {
        return; 
    }   
    
    val.options = GRPWM_UPDATE_OPTION_DISABLE;
    val.period = 0;
    val.compare = 0;
    val.dbcomp = 0;
    val.fix = GRPWM_UPDATE_FIX_DISABLE;
    trq_update_channel(trq_device, device->trq_num, &val);

   
    trq_hbride_enable(device, TRQ_FPGA_PWM_HBRIDGE_DISABLE, device->positive_direction);
    device->enabled = false; 
}


