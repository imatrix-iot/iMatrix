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

/** @file .c
 *
 *  Created on: November 11, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"
#include "../storage.h"
#include "../cli/interface.h"
#include "../device/var_data.h"

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
extern IOT_Device_Config_t device_config;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Log a message on and iMatrix Event
  * @param  String
  * @retval : None
  */
void log_iMatrix( char *msg )
{
    uint16_t log_msg_length;
    var_data_entry_t *var_data_ptr;
    /*
     * Get the index for the logging entry if enabled
     */
    /*
     * Get a variable length packet to store our message in
     */

    if( device_config.send_logs_to_imatrix == true ) {
        log_msg_length = strlen( msg );
        var_data_ptr = imx_get_var_data( log_msg_length );
        if( var_data_ptr != NULL ) {
            /*
             * Add message
             */
            strcpy( (char *) var_data_ptr->data, msg );
            var_data_ptr->length = strlen( msg ) + 1;
            /*
             * Add to queue to send
             */
            imx_printf( "Adding log Message: %s\r\n", msg );
            // Need to do this for now just free
            imx_add_var_free_pool( var_data_ptr );
        }
    }
}
