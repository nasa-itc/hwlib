/* Copyright (C) 2009 - 2019 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including, but not
limited to, any warranty that the software will conform to, specifications any implied warranties of merchantability, fitness
for a particular purpose, and freedom from infringement, and any warranty that the documentation will conform to the program, or
any warranty that the software will be error free.

In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or consequential damages,
arising out of, resulting from, or in any way connected with the software or its documentation.  Whether or not based upon warranty,
contract, tort or otherwise, and whether or not loss was sustained from, or arose out of the results of, or use of, the software,
documentation or services provided hereunder

** History:
**   2020/06/05  T.Jacobs/582    | Initial version,
*/

/* The RTEMS GPIO drivers uses virtual pin numbers, because the GPIO signal are not assigned
 * unique GPIO pin numbers. The virtual pin number is the array index number of the associated 
 * FPGA signal name. 
 *
 * example pin #4 has an array index of 4 and  asscoiated with the "RSTO" FPGA signal name.
*/

#include "libgpio.h"
#include "mares_gpio_defs.h"

#define GPIO_HWLIB_0_VALUE   0xD0
#define GPIO_HWLIB_1_VALUE   0xB1

#define GPIO_HWLIB_MAX_PINS 33

typedef struct
{
  char signalName[64];  
  uint32_t controlReg;
  uint32_t statusReg;
  uint32_t  controlMask;
  uint16_t  controlShift;
  uint32_t  statusMask;
  uint16_t  statusShift;
} gpio_virtual_info_t;


gpio_info_t gpio_device;

gpio_virtual_info_t gpio_vitrual_info[GPIO_HWLIB_MAX_PINS] =
{ 
         /* FPGA signal name,  control reg, status reg,  control reg mask, control reg shift, status reg mask status reg shift */ 

         {"LX7730_ACK_IN_N", 0x805002E8, 0x805002E8, 0X000000FF, 0, 0x00000010, 0},
         {"LX7730_ASPI_ENABLE_N", 0x80500274, 0x80500274, 0xFF000000, 24, 0x00010000, 16},
         {"LX7730_ADC_CLK_OUT", 0x80500274, 0x80500274, 0x0000FFFF, 0, 0x0000FFFF, 0},
         {"LX7730_RESET_OUT_N", 0x80500230, 0, 0x00FF0000, 16, 0, 0},
         {"RSTO", 0x8050028C, 0, 0x000000FF, 0, 0, 0},
         {"GPIO_B0_DIR",  0x80500260, 0x80500270, 0x000000FF, 0, 0x00000001, 0},
         {"GPIO_B0_OE_N",  0x80500260, 0x80500270, 0x00FF0000, 16, 0x00000100, 8},
         {"GPIO_B1_DIR",  0x80500260, 0x80500270, 0x0000FF00, 8, 0x00000010, 4},
         {"GPIO_B1_OE_N",  0x80500260, 0x80500270, 0xFF000000, 24, 0x00001000, 12},
         {"TLM_V_3P3SW",  0,0x800007C0, 0, 0, 0x00000100, 8},
         {"PPS_SPARE_T",  0x800007C0, 0x800007C0, 0x000000FF, 0, 0x000000FF, 0},
         {"CDH_RS422_TX5",  0x80500288, 0, 0x000000FF, 0, 0, 0},
         {"GPIO5_B1",  0x80500278, 0x8050027C, 0x000000FF, 0, 0x00000001, 0},
         {"GPIO6_B1",  0x80500278, 0x8050027C, 0x0000FF00, 8, 0x00000010, 4},
         {"GPIO7_B1",  0x80500278, 0x8050027C, 0x00FF0000, 16, 0x00000100, 8},
         {"GPIO8_B1",  0x80500220, 0x80500220, 0x00010000, 16, 0x00010000, 16},
         {"GPIO9_B1",  0x80500220, 0x80500220, 0x00020000, 17, 0x00020000, 17},
         {"GPIO10_B1", 0x80500220, 0x80500220, 0x00040000, 18, 0x00040000, 18},
         {"GPIO8_B1_DIR",  0x80500220, 0x80500220, 0x00000001, 0, 0x00000001, 0},
         {"GPIO9_B1_DIR",  0x80500220, 0x80500220, 0x00000002, 1, 0x00000002, 1},
         {"GPIO10_B1_DIR", 0x80500220, 0x80500220, 0x00000004, 2, 0x00000004, 2},
         {"GPIO11",    0x80500264, 0x8050026C, 0x000000FF, 0, 00010000, 16},
         {"GPIO12",    0x80500264, 0x8050026C, 0x0000FF00, 8, 0x00100000, 20 },
         {"GPIO13",    0x80500264, 0x8050026C, 0x00FF0000, 16, 0x01000000, 24},
         {"GPIO14",    0x80500264, 0x8050026C, 0xFF000000, 24, 0x10000000, 28},
         {"GPIO15",    0x80500268, 0x8050026C, 0x000000FF, 0, 0x00000001, 0},
         {"SMAP_BUS0_CSI_ADV_B_OUT_N",  0x80500268, 0x8050026C, 0x0000FF00, 8, 0x00000010, 4 },
         {"SMAP_BUS0_CSI_PROGRAM_B_OUT",  0x80500268, 0x8050026C, 0x00FF0000, 16, 0x00000100, 8},
         {"SMAP_RDWR_B_OUT",    0x80500268, 0x8050026C, 0x0FF000000, 24, 0x00001000, 12},
         {"HEATER_SERVICE_1",    0x80500298, 0x8050029C, 0x000000FF, 0, 0x00000001, 0},
         {"HEATER_SERVICE_2",   0x80500298, 0x8050029C, 0x0000FF00, 8, 0x00000010, 4},
         {"SPARE",    0, 0, 0, 0, 0, 0},
         {"SPARE",    0, 0, 0, 0, 0, 0},

};




int32_t gpio_init(gpio_info_t* device) 
{    
   
    if (device->pin < GPIO_HWLIB_MAX_PINS)
    {
         device->isOpen = GPIO_OPEN;
    }  
    return GPIO_SUCCESS;
}


int32_t gpio_read(gpio_info_t* device, uint8_t* value)
{

    if (device->pin < GPIO_HWLIB_MAX_PINS)
    {
         if (gpio_vitrual_info[device->pin].statusReg)
         {
              *value = FPGA_RD_REG_BITS(gpio_vitrual_info[device->pin].statusReg,  gpio_vitrual_info[device->pin].statusMask, gpio_vitrual_info[device->pin].statusShift);
         }
         else
         {
              return GPIO_READ_ERR;
         }

    }

    else
    {
         return GPIO_READ_ERR;
    }

    return GPIO_SUCCESS;
}

int32_t gpio_write(gpio_info_t* device, uint8_t value)
{
   uint8_t  numBits;

   if (device->pin < GPIO_HWLIB_MAX_PINS)
    {
         if (gpio_vitrual_info[device->pin].controlReg)
         {
              numBits = (0xFFFFFFFF & gpio_vitrual_info[device->pin].controlMask) >> gpio_vitrual_info[device->pin].controlShift;
              
             if (numBits > 1 && device->pin != LX7730_ADC_CLK_OUT_PIN  && device->pin != LX7730_RESET_OUT_N_PIN)
              {
                   if (value == 0)
                   {
                        value = GPIO_HWLIB_0_VALUE;
                   }
                   else
                   {
                        value = GPIO_HWLIB_1_VALUE;
                   }
              }

              FPGA_WR_REG_BITS(gpio_vitrual_info[device->pin].controlReg,  gpio_vitrual_info[device->pin].controlMask, gpio_vitrual_info[device->pin].controlShift, value);
         }
         else
         {
              return GPIO_WRITE_ERR;
         }

    }
    else
    {
         return GPIO_WRITE_ERR;
    }

      return GPIO_SUCCESS;
}

int32_t gpio_close(gpio_info_t* device)
{
    
    if (device->pin < GPIO_HWLIB_MAX_PINS)
    {
         device->isOpen = GPIO_CLOSED;
    }
    else
    {        
         return GPIO_FD_OPEN_ERR;
    }
    
    return GPIO_SUCCESS;
}





