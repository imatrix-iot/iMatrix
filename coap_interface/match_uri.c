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
 * match_uri.c
 *
 */
#include <stdbool.h>

#include "wiced.h"

#include "../CoAP/coap.h"
#include "../cli/messages.h"
#include "../cli/messages.h"
#include "../device/icb_def.h"
#include "coap_def.h" // coap_def.c creates the global array CoAP_entries[]
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../storage.h"
/*
 * match_uri.c
 *
 *  Created on: Apr 10, 2015
 *      Author: eric thelin
 *
 *      The match_uri() function searches the array of possible CoAP entries and
 *      returns the one that matches the passed in uri. NULL is returned if
 *      the uri does not match anything in the array.
 */

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_RECV
    #undef PRINTF
	#define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_RECV ) != 0x00 ) imx_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif
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
extern IOT_Device_Config_t device_config;   // Defined in storage.h

/******************************************************
 *               Function Definitions
 ******************************************************/
/* TRUE == 1 FALSE == 0 see ../defines.h */

/* I'm only interested in string equality so create a boolean function for string equality
 * Eric Thelin 13 April 2015
 */
int str_equals(char* s1, char* s2) { // Returns TRUE or FALSE
    return ( strcmp( s1, s2 ) == 0 );
}

/* return NULL if uri not found, otherwise return pointer to entry that matches
 * Eric Thelin 13 April 2015
 */
CoAP_entry_t* match_uri(char* uri, CoAP_entry_t* all_entries, int arSize)
{
    if ( (arSize <= 0) || (all_entries == NULL ) ) {//invalid array size or empty array (
        return NULL;
    }
    else {// there is at least one element in the all_entries[] array

        int i=0;

        int found = str_equals( uri, all_entries[ i ].node.uri );

        // find index i for CoAP_entry that matches uri and set notFound = FALSE)
        while ( ( i+1 < arSize ) && ! found ) {
            i++;
            found = str_equals( uri, all_entries[ i ].node.uri );
        }
        PRINTF( "%s %s.\r\n", found ? "Found" : "Did not find", uri );

        if ( ! found ) {
            return NULL;
        }
        else {// return pointer to CoAP_entry identified by this uri
            return &( all_entries[ i ] );
        }
    }
}
