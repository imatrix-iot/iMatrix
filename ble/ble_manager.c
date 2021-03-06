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

/** @file
 *
 * Bluetooth SmartBridge Demo Application
 *
 * This application demonstrates how to use the Bluetooth SmartBridge API. The app
 * scans for remote Bluetooth Smart devices and allows users to select which device
 * to connect with. Users interact with the app via a webpage running on a local
 * webserver. Once a connection to a remote device is established, services and
 * characteristic values for the device are reported in real-time on the webpage
 *
 * The application demonstrates the following features ...
 *  - Bluetooth SmartBridge (with Security, Attribute Cache, and multiple concurrent
 *    connections support enabled)
 *  - DNS redirect
 *  - Webserver
 *  - Gedday mDNS / DNS-SD Network Discovery
 *
 * Device Configuration
 *    The application is configured to use the Wi-Fi configuration
 *    from the local wifi_config_dct.h file. Change CLIENT_AP_SSID and
 *    CLIENT_AP_PASSPHRASE to your wireless network settings.
 *
 * Application Operation
 * The app runs in a thread, the entry point is application_start()
 *
 *    Startup
 *      - Initialise the device
 *      - Initialise WICED Bluetooth Framework
 *      - Initialise WICED Bluetooth SmartBridge
 *      - Set the maximum number of concurrent connections supported by WICED Bluetooth
 *        SmartBridge
 *      - Configure WICED Bluetooth SmartBridge to enable Attribute Cache
 *      - Start the network interface to connect the device to the network
 *      - Set the local time from a time server on the internet
 *      - Start a webserver to report information about Bluetooth Smart devices
 *      - Start Gedday to advertise the webserver on the network
 *      - Start scanning for remote Bluetooth Smart devices
 *
 *    Usage
 *        Information and debug traces are printed to the UART console.
 *
 *        An mDNS (or Bonjour) browser may be used to find the webpage, or alternately,
 *        the IP address of the device (which is printed to the UART) may be entered
 *        into a web browser.
 *
 *        The webpage displays the Bluetooth devices found.
 *        - If none found, click 'Rescan' to restart the scanning process
 *        - Click 'Connect' on the desired Bluetooth device to initiate a connection.
 *          The application initiates pairing with the Bluetooth device if not paired
 *          before. If successful, pairing information is stored in the DCT
 *        - Connections with remote Bluetooth devices are displayed on the webpage.
 *        - Click 'Details' on the desired Bluetooth device to view a list of services
 *          and characteristic values supported by the device
 *        - Click 'Return' to return to the main page
 *        - Click 'Disconnect' to disconnect
 *        - Click 'Clear Pairing Info' to delete pairing information from the DCT
 *
 *    Notes
 *        If you don't have a Bluetooth Smart device handy, an app called
 *        LightBlue (available from the Apple App store) may be used to
 *        simulate a Bluetooth Smart device on an iPad or iPhone
 *
 */



#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <math.h>
#include "wiced.h"

#include "../cli/interface.h"

#if defined(BLE_ENABLED)
#pragma message "BLE: Enabled in system"
#include "http_server.h"
#include "gedday.h"
#include "resources.h"

#include "wiced_bt.h"
#include "wiced_bt_dev.h"
#include "wiced_bt_smart_interface.h"
#include "wiced_bt_smartbridge.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_smartbridge_gatt.h"

#include "../defines.h"
#include "../hal.h"
#include "../system.h"
#include "../device/dcb_def.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define WEB_PAGE_TIME_LENGTH                8
#define WEB_PAGE_DATE_LENGTH                10
#define MAX_CONCURRENT_CONNECTIONS          5
#define DEFAULT_PASSKEY                     "012345"
#define MAX_BOND_INFO (5)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
#pragma pack(1)

typedef struct
{
    wiced_bt_smart_bond_info_t bond_info[MAX_BOND_INFO]; /* Bond information                      */
    uint32_t                   current_index;            /* Index of the bond info array to store */
} bt_smartbridge_bond_info_dct_t;

#pragma pack()

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

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static wiced_result_t start_scan                      ( void );
static wiced_result_t scan_complete_handler           ( void );
static wiced_result_t scan_advertising_report_handler ( const wiced_bt_smart_advertising_report_t* advertising_report );
static wiced_result_t connect_handler                 ( void* arg );
static wiced_result_t disconnection_handler           ( wiced_bt_smartbridge_socket_t* socket );
static wiced_result_t pairing_handler                 ( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_bond_info_t* bond_info );
static wiced_result_t notification_handler            ( wiced_bt_smartbridge_socket_t* socket, uint16_t attribute_handle );
static int32_t        process_smartbridge_report      ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_rescan                  ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_return                  ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_details                 ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_connect                 ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_disconnect              ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_passkey                 ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_clear_bond_info         ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static wiced_result_t display_passkey                 ( wiced_http_response_stream_t* stream );
static wiced_result_t display_scan_result_list        ( wiced_http_response_stream_t* stream );
static wiced_result_t display_connection_list         ( wiced_http_response_stream_t* stream );
static wiced_result_t display_attributes_list         ( wiced_http_response_stream_t* stream, wiced_bt_smartbridge_socket_t* socket );
static wiced_result_t convert_address_string_to_type  ( const char* string, wiced_bt_device_address_t* address );
static wiced_result_t find_bond_info                  ( const wiced_bt_device_address_t* address, wiced_bt_smart_address_type_t type, wiced_bt_smart_bond_info_t* bond_info );
static wiced_result_t store_bond_info                 ( const wiced_bt_smart_bond_info_t* bond_info );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Local HTTP page database */
static START_OF_HTTP_PAGE_DATABASE(web_pages)
    ROOT_HTTP_PAGE_REDIRECT("/apps/bt_smartbridge/smartbridge_report.html"),
    { "/apps/bt_smartbridge/smartbridge_report.html",       "text/html",                WICED_RESOURCE_URL_CONTENT, .url_content.resource_data  = &resources_apps_DIR_bt_smartbridge_DIR_smartbridge_report_html, },
    { "/bluetooth_device_report.html",                      "text/html",                WICED_DYNAMIC_URL_CONTENT,  .url_content.dynamic_data   = {process_smartbridge_report, 0 }, },
    { "/rescan"    ,                                        "text/html",                WICED_DYNAMIC_URL_CONTENT,  .url_content.dynamic_data   = {process_rescan, 0 }, },
    { "/return"    ,                                        "text/html",                WICED_DYNAMIC_URL_CONTENT,  .url_content.dynamic_data   = {process_return, 0 }, },
    { "/details"   ,                                        "text/html",                WICED_DYNAMIC_URL_CONTENT,  .url_content.dynamic_data   = {process_details, 0 }, },
    { "/connect"   ,                                        "text/html",                WICED_DYNAMIC_URL_CONTENT,  .url_content.dynamic_data   = {process_connect, 0 }, },
    { "/disconnect",                                        "text/html",                WICED_DYNAMIC_URL_CONTENT,  .url_content.dynamic_data   = {process_disconnect, 0 }, },
    { "/set_passkey",                                       "text/html",                WICED_DYNAMIC_URL_CONTENT,  .url_content.dynamic_data   = {process_passkey, 0 }, },
    { "/clear_bond_info",                                   "text/html",                WICED_DYNAMIC_URL_CONTENT,  .url_content.dynamic_data   = {process_clear_bond_info, 0 }, },
    { "/images/favicon.ico",                                "image/vnd.microsoft.icon", WICED_RESOURCE_URL_CONTENT, .url_content.resource_data  = &resources_images_DIR_favicon_ico, },
    { "/scripts/general_ajax_script.js",                    "application/javascript",   WICED_RESOURCE_URL_CONTENT, .url_content.resource_data  = &resources_scripts_DIR_general_ajax_script_js, },
    { "/images/cypresslogo.png",                            "image/png",                WICED_RESOURCE_URL_CONTENT, .url_content.resource_data  = &resources_images_DIR_cypresslogo_png, },
    { "/images/cypresslogo_line.png",                       "image/png",                WICED_RESOURCE_URL_CONTENT, .url_content.resource_data  = &resources_images_DIR_cypresslogo_line_png, },
    { "/styles/buttons.css",                                "text/css",                 WICED_RESOURCE_URL_CONTENT, .url_content.resource_data  = &resources_styles_DIR_buttons_css, },
END_OF_HTTP_PAGE_DATABASE();

/* Scan settings */
static const wiced_bt_smart_scan_settings_t scan_settings =
{
    .type              = BT_SMART_ACTIVE_SCAN,
    .filter_policy     = FILTER_POLICY_NONE,
    .filter_duplicates = DUPLICATES_FILTER_ENABLED,
    .interval          = 96,
    .window            = 48,
    .duration_second   = 3,
};

/* SmartBridge connection settings */
static const wiced_bt_smart_connection_settings_t connection_settings =
{
    .timeout_second                = 3,
    .filter_policy                 = FILTER_POLICY_NONE,
    .interval_min                  = 40,
    .interval_max                  = 56,
    .latency                       = 0,
    .supervision_timeout           = 100,
    .ce_length_min                 = 0,
    .ce_length_max                 = 0,
    .attribute_protocol_timeout_ms = 1000,
};

/* SmartBridge security settings */
static const wiced_bt_smart_security_settings_t security_settings =
{
    .timeout_second              = 15,
    .io_capabilities             = BT_SMART_IO_KEYBOARD_DISPLAY,
    .authentication_requirements = BT_SMART_AUTH_REQ_BONDING_AND_PASSKEY_ENTRY,
    .oob_authentication          = BT_SMART_OOB_AUTH_NONE,
    .max_encryption_key_size     = 16,
    .master_key_distribution     = BT_SMART_DISTRIBUTE_ALL_KEYS,
    .slave_key_distribution      = BT_SMART_DISTRIBUTE_ALL_KEYS,
};

/* UUID constants */
static const wiced_bt_uuid_t uuid_list[] =
{
    [0] = { .len = UUID_16BIT, .uu.uuid16 = 0x2800 }, /* Primary Service */
    [1] = { .len = UUID_16BIT, .uu.uuid16 = 0x2803 }, /* Characteristic */
};

static wiced_http_server_t            http_server;
static wiced_bt_smartbridge_socket_t  smartbridge_socket[MAX_CONCURRENT_CONNECTIONS];
static wiced_bt_smartbridge_socket_t* socket_with_attributes_to_display = NULL;
static wiced_worker_thread_t          connect_worker_thread;
static wiced_mutex_t                  dct_mutex;
static char                           passkey[7] = DEFAULT_PASSKEY;

static const wiced_bt_device_address_t bt_smartbridge_custom_address    = {0x11, 0x22, 0x34, 0x45, 0x56, 0x67};
static const wiced_bt_device_address_t bt_smartbridge_address_mask      = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static ble_results_t ble_scan_results[ MAX_NO_BLE_DEVICES ];
static uint16_t max_sockets=10;

extern dcb_t dcb;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief initialize the BLE scan facility
  * @param  None
  * @retval : None
  */
void init_ble(void)
{
    uint32_t a;

    wiced_bt_init_address( (const wiced_bt_device_address_t*)&bt_smartbridge_custom_address, (const wiced_bt_device_address_t*)&bt_smartbridge_address_mask);

    /* Initialize WICED Bluetooth Framework */
    wiced_bt_init( WICED_BT_HCI_MODE, "IoT Device" );

    /* Initialize WICED SmartBridge */
    wiced_bt_smartbridge_init();

    /* Set maximum concurrent connections */
    //wiced_bt_smartbridge_set_max_concurrent_connections( MAX_CONCURRENT_CONNECTIONS );

    /* Enable Attribute Cache and set maximum number of caches */
    wiced_bt_smartbridge_enable_attribute_cache( MAX_CONCURRENT_CONNECTIONS );

    /* Create all sockets and make them ready to connect. A socket can connect and disconnect multiple times. */
    for ( a = 0; a < MAX_CONCURRENT_CONNECTIONS; a++ )
    {
        wiced_bt_smartbridge_create_socket( &smartbridge_socket[a] );
    }

    /* Create a worker thread for making a connection */
    wiced_rtos_create_worker_thread( &connect_worker_thread, WICED_NETWORK_WORKER_PRIORITY, 4096, 2 );

    /* Create a mutex for DCT access */
    wiced_rtos_init_mutex( &dct_mutex );

    imx_cli_print( "BT SmartBridge is running\r\n" );
}
void init_ble_scan(void)
{
	print_status( "Initializing BLE Scan system\r\n" );

    /* Start HTTP server */
    wiced_http_server_start( &http_server, 80, max_sockets, web_pages, WICED_STA_INTERFACE, DEFAULT_URL_PROCESSOR_STACK_SIZE );

    /* Advertise webpage services using Gedday */
    gedday_init( WICED_STA_INTERFACE, "WICED-BT-SmartBridge" );
    gedday_add_service( "WICED BT SmartBridge web server", "_http._tcp.local", 80, 300, "" );

    WPRINT_APP_INFO( ( "BT SmartBridge application is running. Please go to the webpage\n") );

	dcb.no_ble_devices = 0;
	dcb.no_ble_updates = 0;
	memset( &ble_scan_results, 0x00, sizeof( ble_results_t ) );
	/*
	 *  Start scanning for advertising remote Bluetooth devices
	 */
    start_scan();

}
/**
  * @brief
  * @param  None
  * @retval : None
  */
void ble_scan( uint16_t arg )
{
	if( arg == true ) {
		if( dcb.ble_scan_active == true ) {
			imx_cli_print( "BLE Scan Already Active - Stop before restart\r\n" );
			return;
		}
		imx_cli_print( "BLE Discovery Starting\n\r" );

		dcb.no_ble_devices = 0;
		dcb.no_ble_updates = 0;
		memset( &ble_scan_results, 0x00, sizeof( ble_results_t ) );
		/*
		 *  Start scanning for advertising remote Bluetooth devices
		 */
	    start_scan();
	    dcb.ble_scan_active = true;
	} else {
		imx_cli_print( "BLE Sensor Discovery terminated\r\n" );
	    dcb.ble_scan_active = FALSE;
	}
}

/* Start scan process
 */
static wiced_result_t start_scan( void )
{
    /* Stop ongoing scan */
    //wiced_bt_smartbridge_stop_scan( );

    WPRINT_APP_INFO(("[App] Start scanning\n"));

    /* Start scan */
    wiced_bt_smartbridge_start_scan( &scan_settings, scan_complete_handler, scan_advertising_report_handler );

    return WICED_SUCCESS;
}

/* Scan complete handler. Scan complete event reported via this callback.
 * It runs on the WICED_NETWORKING_WORKER_THREAD context.
 */
static wiced_result_t scan_complete_handler( void )
{
    WPRINT_APP_INFO(("[App] Scan complete\n"));

    /* Scan duration is complete */
    return WICED_SUCCESS;
}

/* Scan result handler. Scan result is reported via this callback.
 * It runs on the Bluetooth transport thread context.
 */
static wiced_result_t scan_advertising_report_handler( const wiced_bt_smart_advertising_report_t* advertising_report )
{
    WPRINT_APP_INFO(("[App] Scan result\n"));

    /* This is an intermediate scan report. Scan result is maintained internally.
     * Call wiced_bt_smartbridge_get_scan_results() to obtain the complete list
     * after the scan process is complete.
     */
    if ( advertising_report->event == BT_SMART_CONNECTABLE_DIRECTED_ADVERTISING_EVENT  )
    {
        /* When a Bluetooth Smart devices sends a directed advertisement, the device
         * expects a connection request before the advertising duration expires.
         * In here, quickly send a connection request.
         */
        wiced_rtos_send_asynchronous_event( &connect_worker_thread, connect_handler, (void*)advertising_report );
    }
    return WICED_SUCCESS;
}

/* Connect handler. Smartbridge connect is executed in this callback.
 * It runs on the connect_worker_thread context.
 * WARNING: Do not place UART print in this function. Only 600 bytes of stack space .
 * is reserved for connect_worker_thread
 */
static wiced_result_t connect_handler( void* arg )
{
    wiced_bt_smart_scan_result_t *scan_result =(wiced_bt_smart_scan_result_t*)arg;
    //wiced_bt_smart_advertising_report_t* scan_result = (wiced_bt_smart_advertising_report_t*)arg;
    wiced_bt_smart_bond_info_t bond_info;
    uint32_t i;
    /* Iterate all sockets and look for the first available socket */
    for ( i = 0; i < MAX_CONCURRENT_CONNECTIONS; i++ )
    {
        wiced_bt_smartbridge_socket_status_t status;

        wiced_bt_smartbridge_get_socket_status( &smartbridge_socket[i], &status );

        /* A free socket is found. Use it to connect */
        if ( status == SMARTBRIDGE_SOCKET_DISCONNECTED )
        {
            if ( find_bond_info( (const wiced_bt_device_address_t *)&scan_result->remote_device.address, scan_result->remote_device.address_type, &bond_info ) == WICED_SUCCESS )
            {
                /* Bond info found. Load bond info to socket */
                wiced_bt_smartbridge_set_bond_info( &smartbridge_socket[i], &security_settings, (const wiced_bt_smart_bond_info_t*)&bond_info );
            }
            else
            {
                /* Bond info not found. Initiate pairing request */
                wiced_bt_smartbridge_enable_pairing( &smartbridge_socket[i], &security_settings, passkey, pairing_handler );
            }
            /* Connect */
            if ( wiced_bt_smartbridge_connect( &smartbridge_socket[i], &scan_result->remote_device, &connection_settings, disconnection_handler, notification_handler ) == WICED_SUCCESS )
            {
                /* Enable Attribute Cache notification */
                WPRINT_APP_INFO(("[App] Smartbridge connection executing...\n"));
                wiced_bt_smartbridge_enable_attribute_cache_notification( &smartbridge_socket[i] );
                return WICED_SUCCESS;
            }
            else
            {
                return WICED_ERROR;
            }
        }
    }

    return WICED_ERROR;
}

/* Disconnection handler. Disconnection by remote device is reported via this callback.
 * It runs on the WICED_NETWORKING_WORKER_THREAD context.
 */
static wiced_result_t disconnection_handler( wiced_bt_smartbridge_socket_t* socket )
{
    return WICED_SUCCESS;
}

/* Notification handler. GATT notification by remote device is reported via this callback.
 * It runs on the WICED_NETWORKING_WORKER_THREAD context.
 */
static wiced_result_t notification_handler( wiced_bt_smartbridge_socket_t* socket, uint16_t attribute_handle )
{
    /* GATT value notification event. attribute_handle is the handle
     * which value of the attribute is updated by the remote device.
     */
    return WICED_SUCCESS;
}

/* Pairing handler. Successful pairing is reported via this callback.
 * It runs on the WICED_NETWORKING_WORKER_THREAD context.
 */
static wiced_result_t pairing_handler( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_bond_info_t* bond_info )
{
    /* Pairing successful. Store bond info */
    store_bond_info( bond_info );

    return WICED_SUCCESS;
}

/* Update webpage
 * It runs on the webserver thread context.
 */
static int32_t process_smartbridge_report( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    UNUSED_PARAMETER(http_message_body);
    UNUSED_PARAMETER(url_path);
    if ( socket_with_attributes_to_display != NULL )
    {
        /* Display attribute list page */
        display_attributes_list( stream, socket_with_attributes_to_display );
    }
    else
    {
        /* Display main page */
        display_passkey( stream );
        display_scan_result_list( stream );
        display_connection_list( stream );
    }

    return 0;
}

/* Process 'Connect' button click from the webpage
 * It runs on the webserver thread context.
 */
static int32_t process_connect( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    UNUSED_PARAMETER(http_message_body);
    UNUSED_PARAMETER(url_path);
    if ( wiced_bt_smartbridge_is_ready_to_connect() == WICED_TRUE )
    {
        wiced_bt_smart_scan_result_t* temp;
        wiced_bt_device_address_t address;
        uint32_t count;

        /* BD_ADDR is passed within the URL. Parse it here */
        convert_address_string_to_type( url_parameters, &address );

        wiced_bt_smartbridge_get_scan_result_list( &temp, &count );

        /* Search for BD_ADDR in the scan result list */
        while ( temp != NULL )
        {
            /* If found, stop scan and send a connect request. Connect attempt is executed in connect_handler callback */
            if ( memcmp( address, temp->remote_device.address, sizeof( address ) ) == 0 )
            {
                WPRINT_APP_INFO(("[App] Trying to Connect to %s ...\n", temp->remote_device.name));
                /* Post request to connect_worker_thread */
                wiced_rtos_send_asynchronous_event( &connect_worker_thread, connect_handler, (void*)temp );
                return 0;
            }

            temp = temp->next;
        }
    }
    return 0;
}

/* Process 'Disconnect' button click from the webpage
 * It runs on the webserver thread context.
 */
static int32_t process_disconnect( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    wiced_bt_device_address_t address;
    uint32_t i;
    UNUSED_PARAMETER(http_message_body);
    UNUSED_PARAMETER(url_path);

    /* BD_ADDR is passed within the URL. Parse it here */
    convert_address_string_to_type( url_parameters, &address );

    for ( i = 0; i < MAX_CONCURRENT_CONNECTIONS; i++ )
    {
        wiced_bt_smartbridge_socket_status_t status;

        wiced_bt_smartbridge_get_socket_status( &smartbridge_socket[i], &status );

        if ( status == SMARTBRIDGE_SOCKET_CONNECTED )
        {
            /* If found, Disconnect and restart scan */
            if ( memcmp( address, smartbridge_socket[i].remote_device.address, sizeof( address ) ) == 0 )
            {
                /* Disable notification */
                wiced_bt_smartbridge_disable_attribute_cache_notification( &smartbridge_socket[i] );

                /* Disconnect */
                wiced_bt_smartbridge_disconnect( &smartbridge_socket[i] );

                return 0;
            }
        }
    }

    return 0;
}

/* Process 'Details' button click from the webpage
 * It runs on the webserver thread context.
 */
static int32_t process_details( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg,  wiced_http_message_body_t* http_message_body )
{
    wiced_bt_device_address_t address;
    uint32_t i;
    UNUSED_PARAMETER(http_message_body);
    UNUSED_PARAMETER(url_path);

    /* BD_ADDR is passed within the URL. Parse it here */
    convert_address_string_to_type( url_parameters, &address );

    for ( i = 0; i < MAX_CONCURRENT_CONNECTIONS; i++ )
    {
        wiced_bt_smartbridge_socket_status_t status;

        wiced_bt_smartbridge_get_socket_status( &smartbridge_socket[i], &status );

        if ( status == SMARTBRIDGE_SOCKET_CONNECTED )
        {
            /* If found, Disconnect and restart scan */
            if ( memcmp( address, smartbridge_socket[i].remote_device.address, sizeof( address ) ) == 0 )
            {
                socket_with_attributes_to_display = &smartbridge_socket[i];

                return 0;
            }
        }
    }

    return 0;
}

/* Process 'Return' button click from the webpage
 * It runs on the webserver thread context.
 */
static int32_t process_return( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    /* Set socket_with_attributes_to_display to NULL to return to main page */
    socket_with_attributes_to_display = NULL;
    UNUSED_PARAMETER(http_message_body);
    UNUSED_PARAMETER(url_path);
    return 0;
}

/* Process 'Rescan' button click from the webpage
 * It runs on the webserver thread context.
 */
static int32_t process_rescan( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    UNUSED_PARAMETER(url_parameters);
    UNUSED_PARAMETER(arg);
    UNUSED_PARAMETER(http_message_body);
    UNUSED_PARAMETER(url_path);
    start_scan();
    return 0;
}

/* Process 'onblur' event from the passkey textbox
 */
static int32_t process_passkey( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg,  wiced_http_message_body_t* http_message_body )
{
    uint8_t length  = 0;
    char*   url_ptr = (char*)url_parameters;
    UNUSED_PARAMETER(http_message_body);
    UNUSED_PARAMETER(url_path);

    memset( passkey, 0, sizeof( passkey ) );
    while ( *url_ptr != '\n' )
    {
        length++;
        url_ptr++;
    }
    memcpy( passkey, url_parameters, MIN( length, sizeof( passkey ) - 1 ) );
    return 0;
}

/* Process 'Clear Pairing Info' button click from the webpage
 * It runs on the webserver thread context.
 */
static int32_t process_clear_bond_info( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    bt_smartbridge_bond_info_dct_t* bond_info_dct;
    UNUSED_PARAMETER(http_message_body);
    UNUSED_PARAMETER(url_path);

    /* DCT API isn't thread-safe. Lock mutex */
    wiced_rtos_lock_mutex( &dct_mutex );

    /* Read DCT to local copy so it can be modified */
    wiced_dct_read_lock( (void**) &bond_info_dct, WICED_TRUE, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    /* Reset DCT local copy to zeroes */
    memset( bond_info_dct, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    /* Write updated bond info to DCT */
    wiced_dct_write( (void*) bond_info_dct, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    wiced_dct_read_unlock( (void*)bond_info_dct, WICED_TRUE );

    wiced_rtos_unlock_mutex( &dct_mutex );

    return 0;
}

/* Display passkey to the webpage
 */
static wiced_result_t display_passkey( wiced_http_response_stream_t* stream )
{
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_passkey_start );
    wiced_http_response_stream_write( stream, (const void*) passkey, strnlen( passkey, sizeof( passkey ) - 1 ) );
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_passkey_end );

    return WICED_SUCCESS;
}

/* Display scan results to the webpage
 */
static wiced_result_t display_scan_result_list( wiced_http_response_stream_t* stream )
{
    char buffer[256];
    uint32_t buffer_length = 0;
    uint32_t count = 0;
    uint32_t i = 0;
    wiced_bt_smart_scan_result_t* temp = NULL;

    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_scan_title );

    if ( wiced_bt_smartbridge_is_scanning() == WICED_TRUE )
    {
        /* Display scanning message */
        wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_scanning_message );
    }
    else
    {
        wiced_bool_t no_scan_results = WICED_TRUE;

        /* List Scan Result */
        wiced_bt_smartbridge_get_scan_result_list( &temp, &count );

        /* Traverse scan results and print them on the webpage */
        while ( temp != NULL )
        {

            if ( temp->filter_display == WICED_FALSE )
            {
                if ( no_scan_results == WICED_TRUE )
                {
                    /* Display table header */
                    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_table_start );
                    no_scan_results = WICED_FALSE;
                }

                /* Write start of row */
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_row_start );

                /* Write device name */
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_cell_start );
                wiced_http_response_stream_write( stream, temp->remote_device.name, strlen( temp->remote_device.name ) );
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_cell_end );


                /* Write unique HTML ID for the device address label */
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_dev_addr_start1 );
                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = sprintf( buffer, "%lu", i );
                wiced_http_response_stream_write( stream, buffer, buffer_length );
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_dev_addr_start2 );

                /* Write device address */
                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = sprintf( buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
                                         temp->remote_device.address[5],
                                         temp->remote_device.address[4],
                                         temp->remote_device.address[3],
                                         temp->remote_device.address[2],
                                         temp->remote_device.address[1],
                                         temp->remote_device.address[0]);
                wiced_http_response_stream_write( stream, buffer, buffer_length );
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_dev_addr_end1 );
                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = sprintf( buffer, "%lu", i );
                wiced_http_response_stream_write( stream, buffer, buffer_length );
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_dev_addr_end2 );

                /* Write end of row */
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_row_end );
                i++;
            }

            temp = temp->next;
        }

        /* Write end of html page */
        wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_table_end );

        if ( no_scan_results == WICED_TRUE )
        {
            //WPRINT_APP_INFO(("[App] No SCAN Results\n"));
            wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_no_device_found );
        }
    }

    return WICED_SUCCESS;
}

/* Display list of connected devices to the webpage
 */
static wiced_result_t display_connection_list( wiced_http_response_stream_t* stream )
{
    char buffer[256];
    uint32_t buffer_length = 0;
    uint32_t i = 0;
    wiced_bool_t no_connections = WICED_TRUE;

    /* List Connected Device List */
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connection_title );

    for ( i = 0; i < MAX_CONCURRENT_CONNECTIONS; i++ )
    {
        wiced_bt_smartbridge_socket_status_t status;

        wiced_bt_smartbridge_get_socket_status( &smartbridge_socket[i], &status );

        if ( status == SMARTBRIDGE_SOCKET_CONNECTED || status == SMARTBRIDGE_SOCKET_CONNECTING )
        {
            if ( no_connections == WICED_TRUE )
            {
                /* Display table header */
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_table_start );
                no_connections = WICED_FALSE;
            }

            /* Write start of row */
            wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_row_start );

            /* Write device name */
            wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_cell_start );
            wiced_http_response_stream_write( stream, smartbridge_socket[i].remote_device.name, strlen( smartbridge_socket[i].remote_device.name ) );
            wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_cell_end );

            if ( status == SMARTBRIDGE_SOCKET_CONNECTED )
            {
                /* Write unique HTML ID for the device address label */
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connected_device_start1 );
                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = sprintf( buffer, "%lu", i );
                wiced_http_response_stream_write( stream, buffer, buffer_length );
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connected_device_start2 );

                /* Write device address */
                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = sprintf( buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
                                         smartbridge_socket[i].remote_device.address[5],
                                         smartbridge_socket[i].remote_device.address[4],
                                         smartbridge_socket[i].remote_device.address[3],
                                         smartbridge_socket[i].remote_device.address[2],
                                         smartbridge_socket[i].remote_device.address[1],
                                         smartbridge_socket[i].remote_device.address[0]);

                wiced_http_response_stream_write( stream, buffer, buffer_length );
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connected_device_end1 );
                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = sprintf( buffer, "%lu", i );
                wiced_http_response_stream_write( stream, buffer, buffer_length );
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connected_device_end2 );
                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = sprintf( buffer, "%lu", i );
                wiced_http_response_stream_write( stream, buffer, buffer_length );
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connected_device_end3 );
            }
            else
            {
                /* Write device address */
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_cell_start );

                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = sprintf( buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
                                         smartbridge_socket[i].remote_device.address[5],
                                         smartbridge_socket[i].remote_device.address[4],
                                         smartbridge_socket[i].remote_device.address[3],
                                         smartbridge_socket[i].remote_device.address[2],
                                         smartbridge_socket[i].remote_device.address[1],
                                         smartbridge_socket[i].remote_device.address[0]);

                wiced_http_response_stream_write( stream, buffer, buffer_length );

                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_cell_end );

                /* Write "Connecting" */
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connecting );
            }
        }
    }

    /* Write end of html page */
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_table_end );


    if ( no_connections == WICED_TRUE )
    {
        wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_no_device_connected );
    }

    return WICED_SUCCESS;
}
/* Display list of attributes to the webpage
 *  */
static wiced_result_t display_attributes_list( wiced_http_response_stream_t* stream, wiced_bt_smartbridge_socket_t* socket )
{
    char                          buffer[256];
    wiced_iso8601_time_t          curr_time;
    wiced_bt_smart_attribute_t    attribute;
    uint16_t                      starting_handle = 0;
    uint32_t                      buffer_length   = 0;

    /* Write device name and address */
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connected_page_start );
    wiced_http_response_stream_write( stream, socket->remote_device.name, strlen( socket->remote_device.name ) );
    wiced_http_response_stream_write( stream, " ( ", 3 );

    memset( buffer, 0, sizeof( buffer ) );
    buffer_length = sprintf( buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
            socket->remote_device.address[5],
            socket->remote_device.address[4],
            socket->remote_device.address[3],
            socket->remote_device.address[2],
            socket->remote_device.address[1],
            socket->remote_device.address[0]);

    wiced_http_response_stream_write( stream, buffer, buffer_length );
    wiced_http_response_stream_write( stream, " )", 2 );
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connected_page_start_end );

    /* Write the time */
    wiced_time_get_iso8601_time( &curr_time );
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_time_start );
    wiced_http_response_stream_write( stream, curr_time.hour, WEB_PAGE_TIME_LENGTH );
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_time_end );

    /* Write the date */
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_date_start );
    wiced_http_response_stream_write( stream, curr_time.year, WEB_PAGE_DATE_LENGTH );
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_date_end );
    /* Look for Primary Service. If found, write the Service UUID and Characteristic Value(s) to the HTTP server */
    while ( wiced_bt_smartbridge_get_attribute_cache_by_uuid( socket, &uuid_list[0], starting_handle, 0xffff, &attribute, sizeof( attribute ) ) == WICED_SUCCESS )
    {
        uint16_t char_start_handle;
        uint16_t char_end_handle;

        /* Update starting handle for the next search */
        starting_handle = attribute.handle + 1;

        /* Copy UUID to buffer */
        memset( buffer, 0, sizeof( buffer ) );

        if ( attribute.value.service.uuid.len == UUID_16BIT )
        {
            buffer_length = sprintf( buffer, "%04X", attribute.value.service.uuid.uu.uuid16 );
        }
        else
        {
            buffer_length = sprintf( buffer, "%04X%04X-%04X-%04X-%04X-%04X%04X%04X",
                    attribute.value.service.uuid.uu.uuid128[7],
                    attribute.value.service.uuid.uu.uuid128[6],
                    attribute.value.service.uuid.uu.uuid128[5],
                    attribute.value.service.uuid.uu.uuid128[4],
                    attribute.value.service.uuid.uu.uuid128[3],
                    attribute.value.service.uuid.uu.uuid128[2],
                    attribute.value.service.uuid.uu.uuid128[1],
                    attribute.value.service.uuid.uu.uuid128[0]);
        }

        /* Write Service UUID */
        wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_service_name_start );
        wiced_http_response_stream_write( stream, buffer, buffer_length );
        wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_service_name_end );

        /* Search characteristic and characteristic values. If found, write to HTTP server.
         *          * Set the search start and end handles to the service start and end handles
         *                   */
        char_start_handle = attribute.value.service.start_handle;
        char_end_handle   = attribute.value.service.end_handle;
        while ( wiced_bt_smartbridge_get_attribute_cache_by_uuid( socket, &uuid_list[1], char_start_handle, char_end_handle, &attribute, sizeof( attribute ) ) == WICED_SUCCESS )
        {
            /* Update characteristic start handle */
            char_start_handle = attribute.value.characteristic.value_handle + 1;

            /* Get Characteristic Value using the handle */
            if ( wiced_bt_smartbridge_get_attribute_cache_by_handle( socket, attribute.value.characteristic.value_handle, &attribute, sizeof( attribute ) ) == WICED_SUCCESS )
            {
                uint32_t i = 0;

                /* Copy UUID to buffer */
                memset( buffer, 0, sizeof( buffer ) );

                /* Write Characteristic Value UUID */
                wiced_http_response_stream_write_resource(stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_char_start );

                if ( attribute.type.len == UUID_16BIT )
                {
                    buffer_length = sprintf( buffer, "%04X", attribute.type.uu.uuid16 );
                }
                else
                {
                    buffer_length = sprintf( buffer, "%04X%04X-%04X-%04X-%04X-%04X%04X%04X",
                            attribute.type.uu.uuid128[7],
                            attribute.type.uu.uuid128[6],
                            attribute.type.uu.uuid128[5],
                            attribute.type.uu.uuid128[4],
                            attribute.type.uu.uuid128[3],
                            attribute.type.uu.uuid128[2],
                            attribute.type.uu.uuid128[1],
                            attribute.type.uu.uuid128[0]);
                }

                wiced_http_response_stream_write( stream, buffer, buffer_length );

                if ( attribute.value_length > 0 )
                {
                    /* Write Characteristic Value */
                    wiced_http_response_stream_write( stream, " [ ", 3 );

                    for ( i = 0; i < attribute.value_length; i++ )
                    {
                        buffer_length = sprintf( buffer, "%02X ", attribute.value.value[i] );
                        wiced_http_response_stream_write( stream, buffer, buffer_length );
                    }

                    wiced_http_response_stream_write( stream, "]", 1 );
                }
                wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_char_end );
            }
        }
    }

    /* Send end of html page */
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_bt_smartbridge_DIR_data_html_connected_page_end );
    return WICED_SUCCESS;

}

/* Convert Bluetooth device address string to device address structure
 */
static wiced_result_t convert_address_string_to_type( const char* string, wiced_bt_device_address_t* address )
{
    uint32_t i;
    uint32_t j;
    unsigned long int temp;
    uint8_t *ptr = (uint8_t *)address;

    /* BD_ADDR is passed within the URL. Parse it here */
    for ( i = 6, j = 0; i > 0; i--, j += 3 )
    {
        char  buffer[3] = {0};
        char* end;

        buffer[0] = string[j];
        buffer[1] = string[j + 1];

        temp = strtoul( buffer, &end, 16 );

        ptr[i-1] = (uint8_t)temp;
    }
    ptr = (uint8_t *)address;
    //WPRINT_APP_INFO(("--> %x %x %x %x %x %x\n", *(address[0]), *(address[1]), *(address[2]), *(address[3]), *(address[4]), *(address[5]) ));
    return WICED_SUCCESS;
}

/* Find Bond Info of device with given address and address type in DCT
 */
static wiced_result_t find_bond_info( const wiced_bt_device_address_t* address, wiced_bt_smart_address_type_t type, wiced_bt_smart_bond_info_t* bond_info )
{
    bt_smartbridge_bond_info_dct_t* dct;
    uint32_t bond_info_count = sizeof( dct->bond_info ) / sizeof( wiced_bt_smart_bond_info_t );
    uint32_t a;

    /* DCT API isn't thread-safe. Lock mutex */
    wiced_rtos_lock_mutex( &dct_mutex );

    wiced_dct_read_lock( (void**) &dct, WICED_FALSE, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    for ( a = 0; a < bond_info_count; a++ )
    {
        if ( ( memcmp( &( dct->bond_info[a].peer_address ), address, sizeof( *address ) ) == 0 ) && ( dct->bond_info[a].address_type == type ) )
        {
            memcpy( bond_info, &dct->bond_info[a], sizeof( *bond_info ) );

            wiced_dct_read_unlock( (void*) dct, WICED_FALSE );

            wiced_rtos_unlock_mutex( &dct_mutex );

            return WICED_SUCCESS;
        }
    }

    wiced_dct_read_unlock( (void*) dct, WICED_FALSE );

    wiced_rtos_unlock_mutex( &dct_mutex );

    return WICED_NOT_FOUND;
}

/* Store new Bond Info into DCT
 */
static wiced_result_t store_bond_info( const wiced_bt_smart_bond_info_t* bond_info )
{
    bt_smartbridge_bond_info_dct_t* bond_info_dct = NULL;
    uint32_t bond_info_count = sizeof( bt_smartbridge_bond_info_dct_t ) / sizeof( wiced_bt_smart_bond_info_t );
    uint32_t a;

    /* DCT API isn't thread-safe. Lock mutex */
    wiced_rtos_lock_mutex( &dct_mutex );

    /* Read DCT to local copy so it can be modified */
    wiced_dct_read_lock( (void**) &bond_info_dct, WICED_TRUE, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    /* Search DCT if device with address and address type specified is found. If found, update bond info */
    for ( a = 0; a < bond_info_count; a++ )
    {
        if ( ( memcmp( &bond_info_dct->bond_info[a].peer_address, &bond_info->peer_address, sizeof( bond_info->peer_address ) ) == 0 ) && ( bond_info_dct->bond_info[a].address_type == bond_info->address_type ) )
        {
            /* Device is found in the DCT. Update bond info */
            memcpy( &bond_info_dct->bond_info[a], bond_info, sizeof( *bond_info ) );

            /* Write updated bond info to DCT */
            wiced_dct_write( (const void*) bond_info_dct, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );
            wiced_dct_read_unlock( (void*) bond_info_dct, WICED_TRUE );

            /* Unlock mutex */
            wiced_rtos_unlock_mutex( &dct_mutex );

            return WICED_SUCCESS;
        }
    }

    /* Bond info for the device isn't found. Store at the next index */
    memcpy( &bond_info_dct->bond_info[bond_info_dct->current_index], bond_info, sizeof( *bond_info ) );

    /* Update current index */
    bond_info_dct->current_index++;
    bond_info_dct->current_index %= bond_info_count;

    /* Write updated bond info to DCT */
    wiced_dct_write( (const void*) bond_info_dct, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );
    wiced_dct_read_unlock( (void*) bond_info_dct, WICED_TRUE );

    wiced_rtos_unlock_mutex( &dct_mutex );

    return WICED_SUCCESS;
}

/*
 * Print BLE Scan results
 */
void print_ble_scan_results( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

	imx_cli_print( "BLE Scan to be implemented\r\n" );
}

#else
#pragma message "BLE: Not enabled in system"
/*
 * Dummy routines
 */
void ble_scan( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

	imx_cli_print( "BLE Not enabled on this platform\r\n" );

}
/*
 * Print BLE Scan results
 */
void print_ble_scan_results( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

	imx_cli_print( "BLE Not enabled on this platform\r\n" );
}

#endif

