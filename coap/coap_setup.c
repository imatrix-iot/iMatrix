/*
 * Copyright 2017, Sierra Telecom. All Rights Reserved.
 *
 * This software, associated documentation and materials ("Software"),
 * is owned by Sierra Telecom ("Sierra") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Sierra hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Sierra's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Sierra.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Sierra
 * reserves the right to make changes to the Software without notice. Sierra
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Sierra does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Sierra product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Sierra's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Sierra against all liability.
 */

/** @file
 *
 * coap.c
 * Functions used to process the CoAP protocol
 *
 */


#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


#include "wiced.h"

#include "coap_udp_recv.h"
#include "coap_setup.h"
#include "coap.h"
#include "que_manager.h"
#include "coap_token.h"
#include "wifi/wifi.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
extern message_list_t list_free;

static message_t message_pool[ NO_FREE_MSG_BUFFERS ];

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  coap_init_pool
  * @param  None
  * @retval : None
  */
void coap_init_pool(void)
{
    uint16_t i;
/*
 * Initialize the pool of free messages
 */
    list_init( &list_free );
    for ( i = 0; i < NO_FREE_MSG_BUFFERS; i++ ) {
        memset( &message_pool[ i ], 0x00, sizeof( message_t) );
        list_add( &list_free, &message_pool[ i ] );
    }
}
