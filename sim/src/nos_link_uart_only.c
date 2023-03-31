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

#include "nos_link.h"
#include <stdlib.h>
#include <stdio.h>

/* nos usart connection table */
nos_connection_t nos_usart_connection[NUM_USARTS] = {
    {"tcp://127.0.0.1:12000", "usart_0"},
    {"tcp://127.0.0.1:12000", "usart_1"},
    {"tcp://127.0.0.1:12000", "usart_2"},
    {"tcp://127.0.0.1:12000", "usart_3"},
    {"tcp://127.0.0.1:12000", "usart_4"},
    {"tcp://127.0.0.1:12000", "usart_5"},
    {"tcp://127.0.0.1:12000", "usart_6"},
    {"tcp://127.0.0.1:12000", "usart_7"},
    {"tcp://127.0.0.1:12000", "usart_8"},
    {"tcp://127.0.0.1:12000", "usart_9"},
    {"tcp://127.0.0.1:12000", "usart_10"},
    {"tcp://127.0.0.1:12000", "usart_11"},
    {"tcp://127.0.0.1:12000", "usart_12"},
    {"tcp://127.0.0.1:12000", "usart_13"},
    {"tcp://127.0.0.1:12000", "usart_14"},
    {"tcp://127.0.0.1:12000", "usart_15"},
    {"tcp://127.0.0.1:12000", "usart_16"},
    {"tcp://127.0.0.1:12000", "usart_17"},
    {"tcp://127.0.0.1:12000", "usart_18"},
    {"tcp://127.0.0.1:12000", "usart_19"},
    {"tcp://127.0.0.1:12000", "usart_20"},
    {"tcp://127.0.0.1:12000", "usart_21"},
    {"tcp://127.0.0.1:12000", "usart_22"},
    {"tcp://127.0.0.1:12000", "usart_23"},
    {"tcp://127.0.0.1:12000", "usart_24"},
    {"tcp://127.0.0.1:12000", "usart_25"},
    {"tcp://127.0.0.1:12000", "usart_26"},
    {"tcp://127.0.0.1:12000", "usart_27"},
    {"tcp://127.0.0.1:12000", "usart_28"},
    {"tcp://127.0.0.1:12000", "usart_29"}
};

/* common transport hub */
NE_TransportHub *hub = NULL;

/* internal hardware bus init/destroy */
extern void nos_init_usart_link(void);
extern void nos_destroy_usart_link(void);

/* initialize nos engine link */
void nos_init_link(void)
{
    printf("initializing nos engine link...\n");

    /* create transport hub */
    hub = NE_create_transport_hub(0);

    /* initialize buses */
    nos_init_usart_link();
}

/* destroy nos engine link */
void nos_destroy_link(void)
{
    printf("destroying nos engine link...\n");

    /* destroy buses */
    nos_destroy_usart_link();

    /* destroy transport hub */
    NE_destroy_transport_hub(&hub);
}

