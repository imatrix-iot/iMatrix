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
 *  Created on: October 1, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"

#include "storage.h"
#include "cli/interface.h"
#include "device/icb_def.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
//#define PRINT_ALLOCATION_MSGS
#define CCM_POOL_LENGTH ( 40 * 1024 )       // Variable data allocated to this space
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
control_sensor_data_t *cd;
control_sensor_data_t *sd;
uint8_t *var_pool_data;
uint8_t ccm_pool_area[ CCM_POOL_LENGTH ] CCMSRAM;
iMatrix_Control_Block_t icb CCMSRAM;
IOT_Device_Config_t device_config CCMSRAM;
imx_imatrix_init_config_t imatrix_init_config CCMSRAM;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief Initialize all the storage - CCMSRAM is uninitialized
  * Initialize based on settings defined in initial config.
  * @param  None
  * @retval : true - memory allocated and initialized / false out of memory.
  */

imx_status_t init_storage(void)
{
    uint16_t i;
    uint16_t memory_requested;

#ifdef USE_CCMRAM
    imx_printf( "\r\n*** Using CCMRAM ***\r\n" );
    icb.using_ccmsram = true;
    /*
     * CCM Memory pool is a simple space that blocks are allocated out of until empty
     */
    icb.ccm_heap_start = &ccm_pool_area[ 0 ];
    icb.ccm_heap_end = icb.ccm_heap_start + CCM_POOL_LENGTH;
    icb.ccm_next_entry = icb.ccm_heap_start;
    imx_printf( "CCM Heap Start: 0x%8lxu, End: 0x%l08x, Length %lu\r\n", (uint32_t) icb.ccm_heap_start, (uint32_t) icb.ccm_heap_end, (uint32_t) CCM_POOL_LENGTH );
#endif
    /*
     * Allocate space based on settings in device_config structure
     */
    /*
     * Allocate buffer space for CoAP - When this is made programmable - current initialized in CoAP setup.
     */
    // if( allocate_coap_space() == false )
    //    return IMX_OUT_OF_MEMORY;
    /*
     * Allocate blocks for the Controls / Sensors and the associated history data.
     *
     * Note: At a later date make the history size entry dependent
     */
    /*
     * Allocate block for Controls Time Series Data storage
     */

    if( device_config.no_controls > IMX_MAX_NO_CONTROLS )
        return IMX_MAXIMUM_CONTROLS_EXCEEDED;
#ifdef PRINT_ALLOCATION_MSGS
    imx_printf( "Allocating space for: %u Controls\r\n", device_config.no_controls );
#endif
    if( device_config.no_controls != 0 ) {
        memory_requested = device_config.no_controls * ( sizeof( control_sensor_data_t ) );
        cd = (control_sensor_data_t *) imx_allocate_storage( memory_requested );
        if( cd == NULL )
            return IMX_OUT_OF_MEMORY;
        for( i = 0; i < device_config.no_controls; i++ ) {
            memory_requested = ( device_config.history_size * SAMPLE_LENGTH  );
            cd[ i ].data = (data_32_t*) imx_allocate_storage( memory_requested );
            if( cd[ i ].data == NULL )
                return IMX_OUT_OF_MEMORY;
        }
#ifdef PRINT_ALLOCATION_MSGS
        imx_printf( "Controls - cd @ 0x%08lx - ", (uint32_t) cd );
        for( i = 0; i < device_config.no_controls; i++ ) {
            imx_printf( "[ %u ] @ 0x%08lx ", i, (uint32_t) cd[ i ].data );
        }
        imx_printf( "\r\n" );
#endif
    }
    /*
     * Allocate block for Sensor Time Series Data storage
     */
    if( device_config.no_sensors > IMX_MAX_NO_SENSORS )
        return IMX_MAXIMUM_SENSORS_EXCEEDED;
#ifdef PRINT_ALLOCATION_MSGS
    imx_printf( "Allocating space for: %u Sensors\r\n", device_config.no_sensors );
#endif
    if( device_config.no_sensors != 0 ) {
        memory_requested = device_config.no_sensors * ( sizeof( control_sensor_data_t ) );
        sd = (control_sensor_data_t *) imx_allocate_storage( memory_requested );
        if( sd == NULL )
            return IMX_OUT_OF_MEMORY;
        for( i = 0; i < device_config.no_sensors; i++ ) {
            memory_requested = ( device_config.history_size * SAMPLE_LENGTH  );
            sd[ i ].data = (data_32_t *) imx_allocate_storage( memory_requested );
            if( sd[ i ].data == NULL )
                return IMX_OUT_OF_MEMORY;
        }
#ifdef PRINT_ALLOCATION_MSGS
        imx_printf( "Sensors - sd @ 0x%08lx - ", (uint32_t) sd );
        for( i = 0; i < device_config.no_sensors; i++ ) {
            imx_printf( "[ %u ] @ 0x%08lx", i, (uint32_t) sd[ i ].data );
        }
    imx_printf( "\r\n" );
#endif
    }
    /*
     * Allocate space for variable length data storage
     */
#ifdef PRINT_ALLOCATION_MSGS
    imx_printf( "Allocating space for Variable length Data\r\n" );
#endif
    memory_requested = 0;
    for( i = 0; i < device_config.no_variable_length_pools; i++ ) {
#ifdef PRINT_ALLOCATION_MSGS
        if( ( device_config.var_data_config[ i ].size * device_config.var_data_config[ i ].no_entries ) > 0 )
            imx_printf( "  Pool size: %u, %u entries\r\n", device_config.var_data_config[ i ].size, device_config.var_data_config[ i ].no_entries );
#endif
        memory_requested += sizeof( var_data_header_t ) + ( device_config.var_data_config[ i ].size * device_config.var_data_config[ i ].no_entries );
    }

    if( memory_requested != 0 ) {
        var_pool_data = ( uint8_t * ) imx_allocate_storage( memory_requested );
        if( var_pool_data == NULL )
            return IMX_OUT_OF_MEMORY;
    }
    icb.var_pool_size = memory_requested;

#ifdef USE_CCMRAM
    imx_printf( "Initial iMatrix Memory allocation complete CCMSRAM used: %lu, free: %lu\r\n", icb.ccm_next_entry - icb.ccm_heap_start, icb.ccm_heap_end - icb.ccm_next_entry );
#endif
    return IMX_SUCCESS;
}
/**
  * @brief simple routine to take a chunk of data out of regular heap or the CCM heap
  * memset to 0
  * @param  sized required
  * @retval : pointer to block - memory allocated / NULL out of memory.
  */

void *imx_allocate_storage( uint16_t size )
{
    void *ccm_entry;
    uint8_t *malloc_entry;

    // imx_printf( "iMatrix memory allocation request: %u Bytes\r\n", size );
    if( size == 0 )
        return NULL;    // Can not allocate no space...
    if( icb.using_ccmsram == true ) {
        if( ( icb.ccm_next_entry + size ) > icb.ccm_heap_end )
            return NULL;
        ccm_entry = icb.ccm_next_entry;
        icb.ccm_next_entry += size;
#ifdef PRINT_ALLOCATION_MSGS
        imx_printf( "  Allocated %u from CCMSRAM Space next entry: @ 0x%8lx, %lu Bytes Left\r\n", size, (uint32_t) icb.ccm_next_entry, (uint32_t) ( icb.ccm_heap_end - icb.ccm_next_entry ) );
#endif
        memset( ccm_entry, 0x00, size );
        return ccm_entry;
    } else {
        malloc_entry = malloc( size );
        if( malloc_entry == NULL )
            return NULL;
        memset( malloc_entry, 0x00, size );
        return (void *) malloc_entry;
    }
}
