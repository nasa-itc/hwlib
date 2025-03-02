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

#include "libgpio.h"

int32_t gpio_init(gpio_info_t* device) 
{    
    return GPIO_SUCCESS;
}

int32_t gpio_read(gpio_info_t* device, uint8_t* value)
{
    return GPIO_SUCCESS;
}

int32_t gpio_write(gpio_info_t* device, uint8_t value)
{
    return GPIO_SUCCESS;
}

int32_t gpio_close(gpio_info_t* device)
{
    return GPIO_SUCCESS;
}
