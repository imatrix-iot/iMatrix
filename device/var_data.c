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
 *	config.c
 *	
 *	Read / Write the configuration
 *
 */


#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "var_data.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define VAR_POOL_SIZE   ( ( ( POOL_0_SIZE + sizeof( var_data_header_t ) ) * DEFAULT_NO_POOL_0 ) + \
                          ( ( POOL_1_SIZE + sizeof( var_data_header_t ) ) * DEFAULT_NO_POOL_1 ) + \
                          ( ( POOL_2_SIZE + sizeof( var_data_header_t ) ) * DEFAULT_NO_POOL_2 ) + \
                          ( ( POOL_3_SIZE + sizeof( var_data_header_t ) ) * DEFAULT_NO_POOL_3 ) + \
                          ( ( POOL_4_SIZE + sizeof( var_data_header_t ) ) * DEFAULT_NO_POOL_4 ) + \
                          ( ( POOL_5_SIZE + sizeof( var_data_header_t ) ) * DEFAULT_NO_POOL_5 ) + \
                          ( ( POOL_6_SIZE + sizeof( var_data_header_t ) ) * DEFAULT_NO_POOL_6 ) )

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
static uint8_t var_pool_data[ VAR_POOL_SIZE ] CCMSRAM;
static var_data_block_t var_data_block[ NO_VAR_POOLS ] CCMSRAM;

extern IOT_Device_Config_t device_config;

/******************************************************
 *               Function Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	initialize the variable length data pools
  * @param  None
  * @retval : None
  */

void init_var_pool(void)
{
    uint16_t i, j, pool_index;
    var_data_entry_t *var_data_ptr;

    /*
     * Assign space for the variable length data pools from the space defined in the above storage for them. Structure as per configuration.
     * Note: Configuration can be changed and pools reinitialized.
     */
    imx_printf( "Initializing Variable length data pool, pool size: %u Bytes\r\n", VAR_POOL_SIZE );
    memset( var_pool_data, 0x00, sizeof( var_pool_data ) );
    memset( var_data_block, 0x00, sizeof( var_data_block ) );

    pool_index = 0;
    for( i = 0; i < NO_VAR_POOLS; i++ ) {
        for( j = 0; j < device_config.var_data_config[ i ].no_entries; j++ ) {
            if( ( pool_index + sizeof( var_data_header_t )  + device_config.var_data_config[ i ].size ) > VAR_POOL_SIZE ) {
                printf( "Out of Variable length Free space, terminating initialization early\r\n" );
                return;
            }
            /*
             * Set up an entry
             */
            var_data_ptr = ( var_data_entry_t *) &var_pool_data[ pool_index ];
            var_data_ptr->header.pool_id = i;
            var_data_ptr->header.length = 0;
            var_data_ptr->header.next = NULL;
            add_var_free_pool( var_data_ptr );
            pool_index += sizeof( var_data_header_t )  + device_config.var_data_config[ i ].size;
        }

    }

}
/**
  * @brief  return / add this var data to the free lists
  * @param  None
  * @retval : None
  */

void add_var_free_pool( var_data_entry_t *var_data_ptr )
{
    var_data_entry_t *temp_ptr;

    imx_printf( "Adding entry to Pool: %u (%u Byte pool)\r\n", var_data_ptr->header.pool_id, device_config.var_data_config[ var_data_ptr->header.pool_id ].size );
    /*
     * Clear all data from entry
     */
    memset( var_data_ptr->data, 0x00, device_config.var_data_config[ var_data_ptr->header.pool_id ].size );

    if( var_data_block[ var_data_ptr->header.pool_id ].head == NULL || var_data_block[ var_data_ptr->header.pool_id ].tail == NULL ) {
        /*
         * This is first entry - set this as the head and tail.
         */
        var_data_block[ var_data_ptr->header.pool_id ].head = var_data_ptr;
        var_data_block[ var_data_ptr->header.pool_id ].tail = var_data_ptr;
    } else {
        /*
         * Extend the list
         */
        temp_ptr = var_data_block[ var_data_ptr->header.pool_id ].tail;
        temp_ptr->header.next = var_data_ptr;
        var_data_ptr->header.next = NULL;
        var_data_ptr->header.length = 0;
        var_data_block[ var_data_ptr->header.pool_id ].tail = var_data_ptr;
    }
}
/**
  * @brief  get var data entry
  * @param  length required
  * @retval : pointer to structure - NULL if not available.
  */
var_data_entry_t *get_var_data( uint16_t length )
{
    uint16_t i;
    var_data_entry_t *var_data_ptr;

    /*
     * Check to find a free buffer capable of handling the requirement.
     */
    for( i = 0; i < NO_VAR_POOLS; i++ ) {
        if( device_config.var_data_config[ i ].size >= length ) {   // This one will do
            if( var_data_block[ i ].head != NULL ) {
                /*
                 * Take top element
                 */
                var_data_ptr = var_data_block[ i ].head;
                /*
                 * Set Head to next element down
                 */
                var_data_block[ i ].head = var_data_ptr->header.next;
                return( var_data_ptr );
            }
        }
    }
    /*
     * None found.
     */
    imx_printf( "No free variable length data available, (requesting: %u Bytes)\r\n", length );
    return NULL;
}
