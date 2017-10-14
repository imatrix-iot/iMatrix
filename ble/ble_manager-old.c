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
 * ble_manager.c
 *
 */


#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../cli/interface.h"
#include "wiced.h"
#define BLE_ENABLED
#ifdef BLE_ENABLED
#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_cfg.h"
#include "bt_target.h"
#include "wiced_bt_stack.h"
#include "gattdefs.h"
#include "sdpdefs.h"

#include "../defines.h"
#include "../hal.h"
#include "../device/dcb_def.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MAX_NO_BLE_DEVICES	100
#define MAC_ADDRESS_LENGTH	6
#define NO_RF_SCAN_RECORDS	100
/******************************************************
 *                   Enumerations
 ******************************************************/
enum {
	BLE_SCAN = 0,
	WIFI_2_4,
	WIFI_5,
	RF_ID,
};
/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct {
	uint16_t device_type;
	uint16_t no_scan_records;
	uint16_t channel;
	uint8_t mac_address[ MAC_ADDRESS_LENGTH ];
	struct {
		union {
			int32_t rssi;
			uint32_t uint32_data;	// Reserved for future options
		} value;
		wiced_utc_time_t seen_at;
	} data[ NO_RF_SCAN_RECORDS ];
} rf_scan_result_t;

typedef struct {
	 wiced_bt_device_address_t		remote_bd_addr;                         /**< Device address */
	 uint8_t                      	ble_addr_type;                          /**< LE Address type */
	 wiced_bt_dev_ble_evt_type_t    ble_evt_type;                           /**< Scan result event type */
	 int8_t                         rssi;                                   /**< Set to #BTM_INQ_RES_IGNORE_RSSI, if not valid */
	 uint8_t                        flag;
	 wiced_utc_time_t				last_seen;
} ble_results_t;

/******************************************************
 *                    Structures
 ******************************************************/
extern dcb_t dcb;
/******************************************************
 *               Variable Definitions
 ******************************************************/
static ble_results_t ble_scan_results[ MAX_NO_BLE_DEVICES ];
static uint16_t no_ble_devices;
static uint16_t no_updates;
static wiced_mutex_t ble_scan_mutex;
extern const wiced_bt_cfg_settings_t wiced_bt_cfg_settings;
extern const wiced_bt_cfg_buf_pool_t wiced_bt_cfg_buf_pools[];

/******************************************************
 *               Function Definitions
 ******************************************************/
static wiced_result_t ble_sensor_management_callback( wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data );
/**
  * @brief
  * @param  None
  * @retval : None
  */
void ble_scan( uint16_t arg )
{
    wiced_result_t result;

	if( arg == true ) {
		if( dcb.ble_scan_active == true ) {
			cli_print( "BLE Scan Already Active - Stop before restart\r\n" );
			return;
		}
		cli_print( "BLE Discovery Starting\n\r" );

		no_ble_devices = 0;
		no_updates = 0;
		memset( &ble_scan_results, 0x00, sizeof( ble_results_t ) );
		/*
		 * Set up the Mutex so we don't access the data at the same time (Backgroud/Foreground tasks)
		 */
	    result = wiced_rtos_init_mutex( &ble_scan_mutex );
	    if( result != WICED_SUCCESS ) {
	        cli_print( "Unable to setup BLE Mutex...\r\n" );
	        return;
	    }
		/*
		 * Register call back and configuration with stack
		 */
		result = wiced_bt_stack_init( ble_sensor_management_callback,  &wiced_bt_cfg_settings, wiced_bt_cfg_buf_pools );
	    if( result != WICED_SUCCESS ) {
	        cli_print( "Unable to setup BLE Callback...\r\n" );
	        return;
	    }
	    dcb.ble_scan_active = true;
	} else {
		cli_print( "BLE Sensor Discovery terminated\r\n" );
		wiced_bt_stack_deinit();
	    result = wiced_rtos_deinit_mutex( &ble_scan_mutex );
	    if( result != WICED_SUCCESS ) {
	        cli_print( "Unable to deinit BLE Mutex...\r\n" );
	        return;
	    }
	    dcb.ble_scan_active = FALSE;
	}
}

static void ble_scan_results_cb( wiced_bt_ble_scan_results_t* p_scan_result, uint8_t* p_adv_data ) {

	uint16_t count, found, compare_result, oldest, i;
	wiced_result_t result;
	wiced_utc_time_t now, oldest_time;

    if ( p_scan_result != NULL )
    {
    	no_updates += 1;
    	wiced_time_get_utc_time( &now );
        /*
         * See if this device is in our list, if so update, otherwise add it in MAC address order
         */
        count = 0;
        found = false;
        while( ( count < no_ble_devices ) && ( found == false ) ) {
        	compare_result = memcmp( &ble_scan_results[ count ].ble_addr_type, &p_scan_result->remote_bd_addr, BD_ADDR_LEN );
        	if(  compare_result == 0x00 ) {
        		found = true;
        	} else if( compare_result == 0x01 ) {
        		/*
        		 * The current item is bigger than the one we have - so insert it in at this point - push all others down, if no space discard oldest entry first
        		 */
        		if( no_ble_devices == MAX_NO_BLE_DEVICES ) {
        			oldest_time = 0xFFFFFFFF; // Must be less than this - at least we hope
        			oldest = 0;
        			for( i = 0; i < no_ble_devices; i++ ) {
        				if( ble_scan_results[ i ].last_seen < oldest_time ) {
        					oldest_time = ble_scan_results[ i ].last_seen;
        					oldest = i;
        				}
        			}
        			/*
        			 * Move the list down from this point
        			 */
                    result = wiced_rtos_lock_mutex( &ble_scan_mutex );
                    if( result != WICED_SUCCESS ) {
                        print_status( "Unable to lock BLE Mutex...\r\n" );
                        return;
                    }
        			memmove( &ble_scan_results[ oldest ], &ble_scan_results[ oldest + 1 ], MAX_NO_BLE_DEVICES - ( oldest + 1 ) );
        			no_ble_devices -= 1;
        		}
        		/*
        		 * Now insert item by moving everything down - will always be less than MAX_NO_BLE_DEVICES
        		 */
                result = wiced_rtos_lock_mutex( &ble_scan_mutex );
                if( result != WICED_SUCCESS ) {
                    print_status( "Unable to lock BLE Mutex...\r\n" );
                    return;
                }
        		memmove( &ble_scan_results[ count + 1 ], &ble_scan_results[ count ], MAX_NO_BLE_DEVICES - ( count ) );
        		no_ble_devices += 1;
        		found = true;
        	}
        	if( found == false )
        		count += 1;
        };
		/*
		 * When we get here we are either at the end of the list or have found the matching entry,
		 *
		 * Either way Update the record "count" time and results
		 */
        if( found == false ) {	// List has not been accessed so lock it first
            result = wiced_rtos_lock_mutex( &ble_scan_mutex );
            if( result != WICED_SUCCESS ) {
                print_status( "Unable to lock BLE Mutex...\r\n" );
                return;
            }
            /*
             * We are adding a device to the end
             */
    		no_ble_devices += 1;
        }
        ble_scan_results[ count ].last_seen = now;
        ble_scan_results[ count ].rssi = p_scan_result->rssi;
        ble_scan_results[ count ].ble_evt_type = p_scan_result->ble_evt_type;
        ble_scan_results[ count ].flag = p_scan_result->flag;
        if( result != WICED_SUCCESS ) {
            print_status( "Unable to unlock BLE Mutex...\r\n" );
            return;
        }
    }

}
/*
 * Print BLE Scan results
 */
void print_ble_scan_results( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

	wiced_result_t result;
	uint16_t i;

	cli_print( "BLE Scan Results (Scan - %s):\r\n", dcb.ble_scan_active == true ? "Active" : "Inactive" );
	cli_print( "MAC Address RSSI (dB) Event Flag - New Updates: %u\r\n", no_updates );
	cli_print( "--------------------------------\r\n" );
	if( no_ble_devices > 0 ) {
	    result = wiced_rtos_lock_mutex( &ble_scan_mutex );
	    if( result != WICED_SUCCESS ) {
	        print_status( "Unable to lock BLE Mutex...\r\n" );
	        return;
	    }
	    for( i = 0; i < no_ble_devices; i++ )
	    	cli_print( "02x:02x:02x:02x:02x:02x:      %04d      %4d  %d\r\n",
	    			ble_scan_results[ i ].remote_bd_addr[ 0 ],
	    			ble_scan_results[ i ].remote_bd_addr[ 1 ],
	    			ble_scan_results[ i ].remote_bd_addr[ 2 ],
	    			ble_scan_results[ i ].remote_bd_addr[ 3 ],
	    			ble_scan_results[ i ].remote_bd_addr[ 4 ],
	    			ble_scan_results[ i ].remote_bd_addr[ 5 ],
	    			ble_scan_results[ i ].rssi,
	    			ble_scan_results[ i ].ble_evt_type,
	    			ble_scan_results[ i ].flag );
	    no_updates = 0;
	    result = wiced_rtos_unlock_mutex( &ble_scan_mutex );
	    if( result != WICED_SUCCESS ) {
	        print_status( "Unable to unlock BLE Mutex...\r\n" );
	    }
	} else
		cli_print( "None Detected\r\n" );
}

/*
 * ble_sensor bt/ble link management callback
 */

static wiced_result_t ble_sensor_management_callback( wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data )
{
    wiced_result_t result = WICED_BT_SUCCESS;

    WPRINT_BT_APP_INFO(("hello_sensor_management_callback: %x\n", event ));

    switch( event )
    {
    /* Bluetooth  stack enabled */
    case BTM_ENABLED_EVT:
    	wiced_bt_ble_observe (WICED_TRUE, 250, ble_scan_results_cb);
        break;

    case BTM_DISABLED_EVT:
        break;

    case BTM_BLE_SCAN_STATE_CHANGED_EVT:
    	print_status("wiced_bt_ble_get_current_scan_state = %u\n", wiced_bt_ble_get_current_scan_state());
    	break;

    default:
        break;
    }

    return result;
}
#else
/*
 * Dummy routines
 */
void ble_scan( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

	cli_print( "BLE Not enabled on this platform\r\n" );

}
/*
 * Print BLE Scan results
 */
void print_ble_scan_results( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

	cli_print( "BLE Not enabled on this platform\r\n" );
}
#endif
/*
 * Print BLE Scan Status
 */
void print_ble_scan_status(void)
{
#ifdef BLE_ENABLED
	if( dcb.ble_scan_active == true ) {
		cli_print( "BLE Scan Active, %u New results\r\n", no_updates );
	}
#endif
}
