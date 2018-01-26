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
 *  wifi.c
 *
 *  General Wi Fi set up and management routines
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "storage.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../ble/ble_manager.h"
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../cli/telnetd.h"
#include "../CoAP/coap_setup.h"
#include "../device/config.h"
#include "../device/imx_leds.h"
#include "../imatrix_upload/imatrix_get_ip.h"
#include "../networking/http_get_sn_mac_address.h"
#include "../networking/keep_alive.h"
#include "../time/ck_time.h"
#include "../time/sntp.h"
#include "../wifi/wifi_logging.h"

#include "../CoAP/udp_transport.h"
#include "../CoAP/tcp_transport.h"

#include "wiced_tls.h"
#include "wiced_utilities.h"
#include "wiced_supplicant.h"

#include "wifi.h"
#include "enterprise_80211.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_HISTORY
    #undef PRINTF
	#define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_HISTORY ) != 0x00 ) st_log_imx_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif

/******************************************************
 *                    Constants
 ******************************************************/
#define HOSTNAME_PREFIX		"ST IoT - "
/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
extern IOT_Device_Config_t device_config;
extern iMatrix_Control_Block_t icb;
typedef struct
{
    uint8_t  uint8_var ;
    uint32_t uint32_var;
    char     string_var[ 50 ];

} dct_read_write_app_dct_t;

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const wiced_ip_setting_t ap_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS( 255,255,255,  0 ) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
};


/******************************************************
 *               Function Definitions
 ******************************************************/
uint16_t wifi_init(void)
{
	wiced_result_t wiced_result;
	wiced_interface_t interface;

	icb.wifi_up = false;

	/*
	 * Start with all LEDs off
	 */
    imx_set_led( 0, IMX_LED_ALL_OFF, 0 );


    imx_set_led( IMX_LED_GREEN_RED, IMX_LED_OTHER, IMX_LED_FLASH | IMX_LED_FLASH_1 | IMX_LED_BLINK_1_5 | IMX_LED_BLINK_2_3 );

	imx_printf( "Initializing Wi Fi\r\n" );
    /*
     * Kill the network - Code for test purposes only
     */
    // wiced_network_suspend();
    // return;

	wiced_result = wiced_wlan_connectivity_init();
    if ( wiced_result != WICED_SUCCESS ) {
        imx_printf( "wiced_wlan_connectivity_init() failed with error code: %u.\n", wiced_result );
        return false;
    }
#ifdef BLE_ENABLED
    /*
     * If we have BLE Scanning enabled - do the init
     */
	/*
	 * See if we need to set up the BLE - only do this after Wi Fi is set up
	 */
    init_ble();
#endif
	if( device_config.AP_setup_mode == true ) {
	    /*
	     *	Be an AP to allow for provisioning
	     */
		wifi_set_default_ap_ssid();
	    interface = WICED_AP_INTERFACE;
	    wiced_result = wiced_network_up(interface, WICED_USE_INTERNAL_DHCP_SERVER, &ap_ip_settings);
	    log_wifi_join_event_results();
	    if( wiced_result != WICED_SUCCESS ) {
	        imx_printf( "Network failed to start in Access Point Mode, failed with error code: %u.\n", wiced_result );
	        return false;
	    }
        wiced_ip_get_ipv4_address( WICED_AP_INTERFACE, &icb.my_ip );
        /*
         * Network setup done
         */
        imx_set_led( 0, IMX_LED_ALL_OFF, 0 );

	    /*
	     * Set the RED Link to blink 1 per second to indicate Set up mode
	     */
        imx_set_led( IMX_LED_RED, IMX_LED_OTHER, IMX_LED_BLINK_1 | IMX_LED_BLINK_1_5 );

	} else {
	    /*
	     *  Clear counters and add values at end
	     */
	    wiced_reset_wifi_event_counters( WICED_SUCCESSFUL_WIFI_JOIN_EVENT_COUNTER | WICED_FAILED_WIFI_JOIN_EVENT_COUNTER );

		// Normal Operating Mode - connect to a remote AP and use that connection for all communications.
	    // Bring up the STA (client) interface
	    interface = WICED_STA_INTERFACE;
	    switch( device_config.st_security_mode ) {
	    	case IMX_SECURITY_8021X_EAP_TLS :
	    		wiced_result = join_ent( device_config.st_ssid, EAP_TYPE_TLS, WICED_SECURITY_WPA2_AES_ENT, (uint8_t *) "", (uint8_t *) "" );
	    		break;
	    	case IMX_SECURITY_8021X_PEAP :
	    		wiced_result = join_ent( "Security", EAP_TYPE_PEAP, WICED_SECURITY_WPA2_AES_ENT, (uint8_t *) "120002555974" /* device_config.serial_no */, (uint8_t *) device_config.password );
	    		break;
	    	case IMX_SECURITY_WEP :
	    	case IMX_SECURITY_WPA2 :
	    	default:
	    		set_wifi_st_ssid( device_config.st_ssid, device_config.st_wpa, device_config.st_security_mode );	// What is stored in the configuration
	    	    wiced_result = wiced_network_up(interface, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
	    	    // This network_up function checks the network status and only does something if the network is not up already.
	    	    break;
	    }
	    log_wifi_join_event_results();
	    /*
	     * Update the counters
	     */
	    icb.wifi_success_connect_count += wiced_successful_wifi_joins();
	    icb.wifi_failed_connect_count += wiced_failed_wifi_joins();
	    /*
	     * See if the network came up.
	     */
	    if( wiced_result != WICED_SUCCESS ) {
	    	if( device_config.connected_to_imatrix == false ) {	// Was this a first time attempt fail?
	    		imx_printf( "Unable to connect. Reverting to Provisioning Mode\r\n" );
	    		wifi_set_default_ap_ssid();
	    		device_config.AP_setup_mode = true;
	    		imatrix_save_config();
	    	}
	    	imx_printf( "WiFi Network Failed to Initialize, will re-attempt...\r\n" );
	    	icb.wifi_up = false;
	        goto connectivity_deinit_and_fail;
	    }
	    /*
	     * Update LED flash to show progress
	     */
	    imx_set_led( IMX_LED_GREEN_RED, IMX_LED_OTHER, IMX_LED_FLASH | IMX_LED_FLASH_1 | IMX_LED_BLINK_1_7 | IMX_LED_BLINK_2_2 );

	    /*
	     * Network Is up
	     */
		icb.wifi_change = false;	// If we changed and it worked good, if not make sure we don't change again
		wiced_ip_get_ipv4_address( WICED_STA_INTERFACE, &icb.my_ip );
		wiced_ip_get_gateway_address( interface, &icb.gw_ip );

	    if( check_valid_ip_address() == false ) {
	        imx_printf( "Wi-Fi Network Failed to get IP Addresses, will re-attempt...\r\n" );
	    	icb.wifi_up = false;
	        goto connectivity_deinit_and_fail;
	    }

		// Register callbacks.

		if ( wiced_network_register_link_callback( link_up, link_down, interface ) != WICED_SUCCESS ) {
	        goto connectivity_deinit_and_fail;
		}
	}
    /*
     * Update LED flash to show progress
     */
	if( device_config.AP_setup_mode == false )
	    imx_set_led( IMX_LED_GREEN_RED, IMX_LED_OTHER, IMX_LED_FLASH | IMX_LED_FLASH_1 | IMX_LED_BLINK_1_8 | IMX_LED_BLINK_2_1 );
	/*
	 * Always set up the UDP Server
	 */
	if( init_udp( interface ) == false ) {
	    imx_printf( "Failed to initialize UDP\r\n" );
		goto connectivity_deinit_and_fail;
	}

	if( device_config.AP_setup_mode == false ) {
	    /*
	     * Initialize TCP TLS Connection
	     */
	    init_tcp();

	    /* Get the gateway that the STA is connected to */
		if( wiced_ip_get_gateway_address( interface, &icb.gw_ip ) != WICED_SUCCESS ) {	// In the current version of WICED, this always returns success
			imx_printf( "Failed to get Gateway Address\r\n" );
			goto connectivity_deinit_and_fail;
		}

		/* Print Gateway Address to the UART */
		imx_printf("Successfully Connected to Gateway: %u.%u.%u.%u\r\n", (unsigned int)((GET_IPV4_ADDRESS( icb.gw_ip ) >> 24) & 0xFF),
				(unsigned int)((GET_IPV4_ADDRESS( icb.gw_ip ) >> 16) & 0xFF),
				(unsigned int)((GET_IPV4_ADDRESS( icb.gw_ip ) >>  8) & 0xFF),
				(unsigned int)((GET_IPV4_ADDRESS( icb.gw_ip ) >>  0) & 0xFF ) );
	    /*
	     * Get iMatrix Address for UDP
	     */
	    if( get_imatrix_ip_address( 0 ) == false )
	        imx_printf( "iMatrix IP Address Failed, will try later\r\n" );
	    else {
	        if( device_config.connected_to_imatrix == false ) {
	            /*
	             * We got here assume we can connect to the server
	             *
	             * @TODO add additional check to ping gateway and server at a later stage
	             *
	             */
	            device_config.connected_to_imatrix = true;
	            imatrix_save_config();
	        }

	    }
	}
	icb.wifi_up = true;
    if( device_config.AP_setup_mode == false ) {
        imx_set_led( 0, IMX_LED_ALL_OFF, 0 );
        imx_set_led( IMX_LED_GREEN, IMX_LED_ON, 0 );
    }
    /*
     * Log this connection
     */
    log_wifi_connection();
	/*
	 * If we are powered up and online and don't have a serial number contact the server and get one.
	 */
	if( ( strlen( device_config.device_serial_number ) == 0 ) && ( device_config.AP_setup_mode == false ) )
	    get_sn_mac( 0 );
#ifdef BLE_ENABLED
	/*
	 * Start BLE Scanning if enabled
	 */
	init_ble_scan();
    icb.ble_initialized = true;
#endif
	telnetd_init();// Returns void( all it does is set the state variable so that the first call to tellnetd() will do the init.

    /*
     * All done here
     */
    if( device_config.AP_setup_mode == false )
        imx_set_led( 0, IMX_LED_ALL_OFF, 0 );
    return true;

connectivity_deinit_and_fail:
/*
 * REVIEW - THIS USED TO BE NEEDED - NOW CAUSES CRASH ON NETWORK RETRY
 */
    // wiced_wlan_connectivity_deinit();// Always returns success even when it fails.
    /*
     * De init TCP only frees up TCP resources if they were allocated.
     */
    deinit_tcp();

    imx_set_led( 0, IMX_LED_ALL_OFF, 0 );
    /*
     * Update LED flash to show progress - Short Green - Long Red
     */
    imx_set_led( IMX_LED_GREEN_RED, IMX_LED_OTHER, IMX_LED_FLASH | IMX_LED_FLASH_1 | IMX_LED_BLINK_1_2 | IMX_LED_BLINK_2_7 );

    return false;// When the network was successfully torn down after an error.
}

void wifi_shutdown()
{

	wiced_interface_t interface;

	imx_printf( "Shutting Down the Wi Fi Interface\r\n" );
	if( device_config.AP_setup_mode == true ) {
		interface = WICED_AP_INTERFACE;
	} else {
		// Normal Operating Mode - connect to a remote AP and use that connection for all communications.
	    // Bring up the STA (client) interface
	    interface = WICED_STA_INTERFACE;
	}

    // Tear down networking.

    telnetd_deinit();// No return code, so I can't do error handling here. Error handling must be in telnetd().

    deinit_udp( interface );
    deinit_tcp();

	wiced_network_deregister_link_callback( link_up, link_down, interface );// These callbacks indicate link status but do not interact with multicasting or udp.

	icb.wifi_up = false;

    wiced_wlan_connectivity_deinit();// Always returns success even when it fails.

    imx_printf( "Completed shutting Down the Wi Fi Interface\r\n" );
}

uint16_t check_valid_ip_address(void)
{
	if( icb.wifi_up == true  && device_config.AP_setup_mode == false) {
		/* Save the gateway and IP address for the STA*/
		wiced_ip_get_gateway_address( WICED_STA_INTERFACE, &icb.gw_ip );
		wiced_ip_get_ipv4_address( WICED_STA_INTERFACE, &icb.my_ip );
		if( ( icb.gw_ip.ip.v4 == 0 ) || ( icb.my_ip.ip.v4 == 0  ) ) {
			cli_print( "0.0.0.0 Zero IP Address - failing network\r\n" );
	    	icb.wifi_up = false;
	        return false;
		}
	}
	return true;

}

void link_up(void)
{
	icb.print_msg |= MSG_WIFI_UP;
	icb.wifi_up = true;
}

void link_down(void)
{

    icb.print_msg |= MSG_WIFI_DN;
	icb.wifi_up = false;
	icb.wifi_dropouts += 1;
}

char* get_wifi_ssid( char* buffer, uint16_t index )
{
	platform_dct_wifi_config_t *wifi;
    wifi = (platform_dct_wifi_config_t*)wiced_dct_get_current_address( DCT_WIFI_CONFIG_SECTION );


    return (char*)wifi->stored_ap_list[ index ].details.SSID.value;

    //+ OFFSETOF(platform_dct_wifi_config_t, SSID );
}

/**
  * @brief  Set the DCT to the default SSID to use for a station
  * @param  None
  * @retval : None
  */
void wifi_set_default_st_ssid(void)
{
	set_wifi_st_ssid( device_config.default_st_ssid, device_config.default_st_wpa, device_config.default_st_security_mode );
}

/**
  * Permanently set the SSID passphrase and hostname in the DCT
  * The hostame is generated using the serial number stored in the device_config.
  * The SSID and passphrase are null terminated strings when passed in, but
  * are stored as length specified strings in the DCT.
  *
  */
void set_wifi_st_ssid( char *ssid, char *passphrase, wiced_security_t security )
{
    platform_dct_wifi_config_t* dct_wifi = NULL;
    platform_dct_network_config_t* dct_network = NULL;
    union of_each_dct_section {
    	platform_dct_wifi_config_t wifi;
    	platform_dct_network_config_t network;
    } save;

    if( ( strlen( ssid ) > IMX_SSID_LENGTH ) || ( strlen( passphrase ) > IMX_WPA2PSK_LENGTH ) ) {
    	imx_printf( "Failed to set SSID//Pass phrase too long, SSID: %s, Pass phrase: %s\r\n", ssid, passphrase );
    	return;
    }
    /*
     * Save in config
     */
    imx_printf( "Setting SSID: %s, Passphrase: ->%s<-, Security Code: 0x%08lx\r\n", ssid, passphrase, (uint32_t) security );
    strcpy( device_config.st_ssid, ssid );
    strcpy( device_config.st_wpa, passphrase );
    device_config.st_security_mode = security;

    // Modify the SSID and Passphrase in DCT

    dct_wifi = (platform_dct_wifi_config_t*)wiced_dct_get_current_address( DCT_WIFI_CONFIG_SECTION );
    memmove( &( save.wifi ), dct_wifi, sizeof( platform_dct_wifi_config_t ) );

    strncpy( (char *) save.wifi.stored_ap_list[0].details.SSID.value, ssid, IMX_SSID_LENGTH );
    save.wifi.stored_ap_list[0].details.SSID.length = strlen( ssid );
    strncpy( save.wifi.stored_ap_list[0].security_key, passphrase, SECURITY_KEY_SIZE );
    save.wifi.stored_ap_list[0].security_key_length = strlen( passphrase );
    save.wifi.stored_ap_list[0].details.security = security;
    wiced_dct_write( &( save.wifi ), DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) );

    // Make sure the hostname is correct

    dct_network = (platform_dct_network_config_t*)wiced_dct_get_current_address( DCT_NETWORK_CONFIG_SECTION );
    memmove( &( save.network ), dct_network, sizeof( platform_dct_network_config_t ) );

    strncpy( (char *) save.network.hostname.value, device_config.device_name, HOSTNAME_SIZE - ( IMX_DEVICE_SERIAL_NUMBER_LENGTH + 1 ) );
    strncat( (char *) save.network.hostname.value, ":", HOSTNAME_SIZE - ( IMX_DEVICE_SERIAL_NUMBER_LENGTH + 1 ) );
    if( strlen( device_config.device_serial_number ) == 0 )
        strncat( (char *) save.network.hostname.value, "No SN", HOSTNAME_SIZE - ( IMX_DEVICE_SERIAL_NUMBER_LENGTH + 1 ) );
    else
        strncat( (char *) save.network.hostname.value, device_config.device_serial_number, HOSTNAME_SIZE - ( IMX_DEVICE_SERIAL_NUMBER_LENGTH + 1 ) );
    wiced_dct_write( &( save.network ), DCT_NETWORK_CONFIG_SECTION, 0, sizeof(platform_dct_network_config_t) );

    if( device_config.AP_setup_mode == true ) { // We were in set up mode - turn off the blinking led
        imx_set_led( IMX_LED_RED, IMX_LED_ALL_OFF, 0 );           // Set RED Led to off
        device_config.AP_setup_mode = false;
    }
    icb.wifi_up = false;
    imatrix_save_config();

}

/**
  * @brief  Set the DCT to the default SSID to use for a Station
  * @param  None
  * @retval : None
  */
void wifi_set_default_ap_ssid(void)
{
	char ap_ssid[ IMX_SSID_LENGTH ];
	platform_dct_wifi_config_t* dct_wifi = NULL;

	/*
	 * Create Default SSID using Define + last 6 MAC Address
	 */
    dct_wifi = (platform_dct_wifi_config_t*)wiced_dct_get_current_address( DCT_WIFI_CONFIG_SECTION );

    if( ( strlen( device_config.product_name ) + 7 ) > IMX_SSID_LENGTH )
        sprintf( ap_ssid, "%s", device_config.default_ap_ssid );
    else
        sprintf( ap_ssid, "%s-%02X%02X%02X", device_config.product_name, (uint16_t) dct_wifi->mac_address.octet[ 3 ], (uint16_t) dct_wifi->mac_address.octet[ 4 ], (uint16_t) dct_wifi->mac_address.octet[ 5 ] );
    imx_printf( "Setting Access Point to SSID:%s\r\n", ap_ssid );
    set_wifi_ap_ssid( ap_ssid, device_config.default_ap_wpa, device_config.default_ap_security_mode, device_config.default_ap_channel );
}

/**
  * Set the SSID passphrase and hostname in the DCT
  * The hostame is generated using the serial number stored in the device_config.
  * The SSID and passphrase are null terminated strings when passed in, but
  * are stored as length specified strings in the DCT.
  *
  */
void set_wifi_ap_ssid( char *ssid, char *passphrase, wiced_security_t security, uint16_t channel )
{
    platform_dct_wifi_config_t* dct_wifi = NULL;
    platform_dct_network_config_t* dct_network = NULL;
    union of_each_dct_section {
    	platform_dct_wifi_config_t wifi;
    	platform_dct_network_config_t network;
    } save;

    if( ( strlen( ssid ) > IMX_SSID_LENGTH ) || ( strlen( passphrase ) > IMX_WPA2PSK_LENGTH ) ) {
    	cli_print( "Failed to set SSID//Pass phrase too long, SSID: %s, Pass phrase: %s\r\n", ssid, passphrase );
    	return;
    }
    /*
     * Save in config
     */
    strcpy( device_config.ap_ssid, ssid );
    strcpy( device_config.ap_wpa, passphrase );
    device_config.ap_security_mode = security;
    device_config.ap_channel = channel;

    // Modify the SSID and Passphrase in DCT

    dct_wifi = (platform_dct_wifi_config_t*)wiced_dct_get_current_address( DCT_WIFI_CONFIG_SECTION );
    memmove( &( save.wifi ), dct_wifi, sizeof( platform_dct_wifi_config_t ) );

    strncpy( (char *) save.wifi.soft_ap_settings.SSID.value, ssid, IMX_SSID_LENGTH );
    save.wifi.soft_ap_settings.SSID.length = strlen( ssid );
    save.wifi.soft_ap_settings.channel = channel;
    strncpy( save.wifi.soft_ap_settings.security_key, passphrase, SECURITY_KEY_SIZE );
    save.wifi.soft_ap_settings.security_key_length = strlen( passphrase );
    save.wifi.soft_ap_settings.security = security;
    wiced_dct_write( &( save.wifi ), DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) );

    // Make sure the hostname is correct

    dct_network = (platform_dct_network_config_t*)wiced_dct_get_current_address( DCT_NETWORK_CONFIG_SECTION );
    memmove( &( save.network ), dct_network, sizeof( platform_dct_network_config_t ) );

    strcpy( (char *) save.network.hostname.value, HOSTNAME_PREFIX );
    strncat( (char *) save.network.hostname.value, device_config.device_serial_number, HOSTNAME_SIZE - 8 );
    wiced_dct_write( &( save.network ), DCT_NETWORK_CONFIG_SECTION, 0, sizeof(platform_dct_network_config_t) );
}
void st_printf_wifi_ssid(void)
{
    platform_dct_wifi_config_t* dct_wifi = NULL;

    // Modify the SSID and passsphrase

    dct_wifi = (platform_dct_wifi_config_t*)wiced_dct_get_current_address( DCT_WIFI_CONFIG_SECTION );
    cli_print( "%s", dct_wifi->stored_ap_list[0].details.SSID.value );
}
void set_fixture_dct_name(void)
{
    platform_dct_network_config_t* dct_network = NULL;
    platform_dct_network_config_t network;

    // Modify the hostname

    dct_network = (platform_dct_network_config_t*)wiced_dct_get_current_address( DCT_NETWORK_CONFIG_SECTION );
    memmove( &( network ), dct_network, sizeof( platform_dct_network_config_t ) );

    strcpy( (char *) network.hostname.value, HOSTNAME_PREFIX );
    strncat( (char *) network.hostname.value, device_config.device_serial_number, HOSTNAME_SIZE - 8 );
    wiced_dct_write( &( network ), DCT_NETWORK_CONFIG_SECTION, 0, sizeof(platform_dct_network_config_t) );
}


void cli_wifi_setup( uint16_t arg )
{
	char *token;

	UNUSED_PARAMETER(arg);
	/*
	 *	command format setup <off|on>
	 */
	token = strtok(NULL, " " );	// Get arg if any
	if( token ) {
		if( strcmp( token, "off" ) == 0x00 ) {
			cli_print( "Setting to Online mode - look System will connect to stored SSID: %s\r\n", device_config.st_ssid );
			device_config.AP_setup_mode = false;
			imatrix_save_config();
			icb.wifi_up = false;
		} else
			cli_print( "on / off required\r\n" );
	} else {
		cli_print( "Setting to setup mode - look for AP and connect with App: %s + MAC\r\n", device_config.ap_ssid );
		device_config.AP_setup_mode = true;
		imatrix_save_config();
		icb.wifi_up = false;
	}
}
/*
 * Return the Wi Fi Status - to tell if we are on a station etc.
 */
imx_wifi_mode_t imx_get_wifi_mode(void)
{
    if( icb.wifi_up == true ) {
        if( device_config.AP_setup_mode == true )
            return IMX_WIFI_ACCESS_POINT;
        else
            return IMX_WIFI_STATION;
    } else
        return IMX_WIFI_OFFLINE;
}
