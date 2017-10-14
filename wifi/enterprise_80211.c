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

/** @file 80211_enterprise.c
 *
 *
 *  Created on: September, 2016
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "stdlib.h"
#include "string.h"

#include "wwd_wlioctl.h"
#include "wwd_wifi.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "network/wwd_network_interface.h"
#include "wwd_management.h"
#include "internal/wwd_sdpcm.h"
#include "internal/wwd_internal.h"
#include "network/wwd_buffer_interface.h"
#include "wiced_management.h"
#include "wiced_crypto.h"
#include "wiced.h"
//#include "wiced_security.h"
#include "internal/wiced_internal_api.h"
#include "besl_host.h"
#include "besl_host_interface.h"
#include "wiced_tls.h"
#include "wiced_utilities.h"
#include "wiced_supplicant.h"

#include "../defines.h"
#include "../cli/interface.h"
#include "../device/dcb_def.h"
#include "../device/cert_util.h"

#include "enterprise_80211.h"


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MAX_PASSPHRASE_LEN   ( 64 )
#define MIN_PASSPHRASE_LEN   (  8 )
#define A_SHA_DIGEST_LEN     ( 20 )
#define DOT11_PMK_LEN        ( 32 )
#define	MSCHAP_PASSWORD_LENGTH	32
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
extern dcb_t dcb;
static wiced_tls_session_t tls_session = { 0 };
static char last_joined_ssid[SSID_NAME_SIZE+1] = ""; /* 32 characters + terminating null */
char last_started_ssid[SSID_NAME_SIZE+1] = "";       /* 32 characters + terminating null */

/******************************************************
 *               Function Definitions
 ******************************************************/
static uint16_t wifi_join(char* ssid, uint8_t ssid_length, wiced_security_t auth_type, uint8_t* key, uint16_t key_length, char* ip, char* netmask, char* gateway);
static void analyse_failed_join_result( wiced_result_t join_result );

/**
  * @brief
  * @param  None
  * @retval : None
  */
/*!
 ******************************************************************************
 * Joins an access point using enterprise security
 *
 * @return  0 for success, otherwise error
 */

uint16_t join_ent( char *ssid, eap_type_t eap_type, wiced_security_t auth_type, uint8_t *device_identity, uint8_t *device_password )
{
    supplicant_workspace_t supplicant_workspace;

    /* When using PEAP note:
     * There are two identities that are being passed, outer identity (un encrypted) and inner identity (encrypted).
     * Different servers have different requirements for the outer identity.
     * For windows IAS its something like "wiced@wiced.local: ( where wiced.local in your server's domain )
     * FreeRadius doesn't care much about outer name
     * Cisco must have the outer identity in the user list e.g. "wiced" ( i.e. outer and inner identities must match ).
     *
     * When using EAP-TLS note:
     * If the RADIUS server is configured to check the user name against the certificate common name then eap_identity needs to be the same as the certificate common name.
     * For example:
     * char eap_identity[] = "wifi-user@wifilabs.local";
     *
     *	Command from console: join_ent <ssid> <eap_tls|peap> [username] [password] <wpa2|wpa2_tkip|wpa|wpa_tkip>
     *						  join_ent Security2 eap_tls wpa2
     *						  join_ent Security-IOG761 eap_tls wpa2
     *
     *						  Debug iwpriv ra0 set Debug=4
     */
    char eap_identity[] = "device@sierratelecom.net"; // "120002555974"; //
    wiced_tls_context_t context;
    wiced_tls_identity_t identity;

    print_status( "Starting 802.1X Process\r\n" );
    /*
     * Make sure we are pointing to the correct certs
     */
    update_cert_pointers();

    // print_credentials();
    if ( !( ( eap_type == EAP_TYPE_TLS ) || ( eap_type == EAP_TYPE_PEAP ) ) ) {
        cli_print("Unsupported security type\n" );
        return WICED_ERROR;
    }

    if ( auth_type != WICED_SECURITY_WPA2_AES_ENT ) {
        print_status("Unsupported authentication type\n" );
        return WICED_ERROR;
    }

	print_status( "Setting TLS initialization\r\n" );
    if ( eap_type == EAP_TYPE_PEAP ) {
        wiced_tls_init_identity( &identity, NULL, 0, NULL, 0 );
    } else {
        wiced_tls_init_identity( &identity, (char *) dcb.wifi_8021X_key, (uint32_t) strlen( (char*) dcb.wifi_8021X_key ), dcb.wifi_8021X_certificate,
        		(uint32_t)strlen( (char*) dcb.wifi_8021X_certificate ) );
    }

	print_status( "Setting TLS context\r\n" );
    wiced_tls_init_context( &context, &identity, NULL );

    if ( tls_session.id_len > 0 ) {
        memcpy( &context.session, &tls_session, sizeof(wiced_tls_session_t) );
    } else {
        memset( &context.session, 0, sizeof(wiced_tls_session_t) );
    }

    print_status( "Initializing Root CA certificates\r\n" );
    wiced_tls_init_root_ca_certificates( (char*) dcb.root_certificate, (uint32_t)strlen( (char*) dcb.root_certificate ) );

    print_status( "Initializing Supplicant\r\n" );
    if ( besl_supplicant_init( &supplicant_workspace, eap_type, WWD_STA_INTERFACE ) == BESL_SUCCESS ) {
        print_status( "Enabling TLS\r\n" );
        wiced_supplicant_enable_tls( &supplicant_workspace, &context );
        print_status( "Setting Identity\r\n" );
        besl_supplicant_set_identity( &supplicant_workspace, eap_identity, strlen( eap_identity ) );
        if ( eap_type == EAP_TYPE_PEAP ) {
        	print_status( "Setting PEAP MSCHAPV2 identity\r\n" );
            /* Default for now is MSCHAPV2 */
            supplicant_mschapv2_identity_t mschap_identity;
            char mschap_password[MSCHAP_PASSWORD_LENGTH];

            /* Convert ASCII to UTF16 */
            int i;
            uint8_t*  password = device_password;
            uint8_t*  unicode  = (uint8_t*) mschap_password;

            if(strlen( (char *) device_password) > MSCHAP_PASSWORD_LENGTH ) {
            	print_status( "Password too long must be less than %u charcters\r\n", MSCHAP_PASSWORD_LENGTH );
            	return WICED_ERROR;
            }
            for ( i = 0; i <= strlen( (char *) device_password); i++ ) {
                *unicode++ = *password++;
                *unicode++ = '\0';
            }

            mschap_identity.identity = device_identity;
            mschap_identity.identity_length = strlen((char *)device_identity);

            mschap_identity.password = (uint8_t*) mschap_password;
            mschap_identity.password_length = 2*(i-1);

            print_status( "Setting inner identity\r\n" );
            besl_supplicant_set_inner_identity( &supplicant_workspace, eap_type, &mschap_identity );

        }
        print_status( "Starting Supplicant\r\n" );
        if ( besl_supplicant_start( &supplicant_workspace ) == BESL_SUCCESS ) {
//            print_status( "Attempting to get a Enterprise connection to: %s, using EAP Type: %u, Authentication Type: 0x%08lx\r\n", ssid, eap_type, (uint32_t) auth_type );
            if ( wifi_join( ssid, strlen(ssid), auth_type, NULL, 0, NULL, NULL, NULL ) == true ) {
                memcpy( &tls_session, &context.session, sizeof(wiced_tls_session_t) );
                print_status( "Supplicant successfully started\r\n" );
            } else {
            	return WICED_ERROR;
            	print_status( "Failed to Join network\r\n" );
            }
        } else {
        	print_status( "Supplicant failed to started\r\n" );
        	return WICED_ERROR;
        }

    } else {
        cli_print("Unable to initialize supplicant\n" );
        wiced_tls_deinit_context( &context );
        wiced_tls_deinit_root_ca_certificates();
        wiced_tls_deinit_identity( &identity );

        besl_supplicant_deinit( &supplicant_workspace );

        return WICED_ERROR;
    }
    wiced_tls_deinit_context( &context );
    wiced_tls_deinit_root_ca_certificates();
    wiced_tls_deinit_identity( &identity );

    besl_supplicant_deinit( &supplicant_workspace );

    return WICED_SUCCESS;
}

static uint16_t wifi_join(char* ssid, uint8_t ssid_length, wiced_security_t auth_type, uint8_t* key, uint16_t key_length, char* ip, char* netmask, char* gateway)
{
    wiced_network_config_t      network_config;
    wiced_ip_setting_t*         ip_settings = NULL;
    wiced_ip_setting_t          static_ip_settings;
    platform_dct_wifi_config_t* dct_wifi_config;
    wiced_result_t              result;

    print_status( "Starting Join Process to: %s, length: %u, with authentication: 0x%08lx\r\n", ssid, (uint16_t) ssid_length, (uint32_t) auth_type );
    if (ssid_length > SSID_NAME_SIZE)
    {
    	cli_print( "SSID too long\r\n" );
        return false;
    }

    if (wwd_wifi_is_ready_to_transceive(WWD_STA_INTERFACE) == WWD_SUCCESS)
    {
        cli_print("STA already joined; use leave command first\n");
        return true;
    }

    if ( wwd_wifi_is_ready_to_transceive( WWD_AP_INTERFACE ) == WWD_SUCCESS )
    {
        cli_print("AP will be moved to the STA assoc channel" );
        if ( verify_cmd() == true )
        {
            return true;
        }
    }
    print_status( "Updating DCT\r\n" );
    /* Read config */
    wiced_dct_read_lock( (void**) &dct_wifi_config, WICED_TRUE, DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) );

    /* Modify config */
    dct_wifi_config->stored_ap_list[0].details.SSID.length = ssid_length;
    memset( dct_wifi_config->stored_ap_list[0].details.SSID.value, 0, sizeof(dct_wifi_config->stored_ap_list[0].details.SSID.value) );
    memcpy( (char*)dct_wifi_config->stored_ap_list[0].details.SSID.value, ssid, ssid_length );
    dct_wifi_config->stored_ap_list[0].details.security = auth_type;
    if ( ( auth_type & ENTERPRISE_ENABLED ) == 0 )
    {
        /* Save credentials for non-enterprise AP */
        memcpy((char*)dct_wifi_config->stored_ap_list[0].security_key, (char*)key, MAX_PASSPHRASE_LEN);
        dct_wifi_config->stored_ap_list[0].security_key_length = key_length;
    }

    /* Write config */
    wiced_dct_write( (const void*) dct_wifi_config, DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) );
    print_status( "DCT Updated\r\n" );
    /* Tell the network stack to setup it's interface */
    if (ip == NULL )
    {
        network_config = WICED_USE_EXTERNAL_DHCP_SERVER;
    }
    else
    {
        network_config = WICED_USE_STATIC_IP;
        str_to_ip( ip,      &static_ip_settings.ip_address );
        str_to_ip( netmask, &static_ip_settings.netmask );
        str_to_ip( gateway, &static_ip_settings.gateway );
        ip_settings = &static_ip_settings;
    }
    print_status( "About to bring network up\r\n" );
    if ( ( result = wiced_network_up( WICED_STA_INTERFACE, network_config, ip_settings ) ) != WICED_SUCCESS )
    {
    	print_status( "Failed to bring up network: %u\r\n", result );
        if ( auth_type == WICED_SECURITY_WEP_PSK ) /* Now try shared instead of open authentication */
        {
            dct_wifi_config->stored_ap_list[0].details.security = WICED_SECURITY_WEP_SHARED;
            wiced_dct_write( (const void*) dct_wifi_config, DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) );
            WPRINT_APP_INFO(("WEP with open authentication failed, trying WEP with shared authentication...\n"));

            if ( wiced_network_up( WICED_STA_INTERFACE, network_config, ip_settings ) != WICED_SUCCESS ) /* Restore old value */
            {
                WPRINT_APP_INFO(("Trying shared wep\n"));
                dct_wifi_config->stored_ap_list[0].details.security = WICED_SECURITY_WEP_PSK;
                wiced_dct_write( (const void*) dct_wifi_config, DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) );
            }
            else
            {
                wiced_dct_read_unlock( (void*) dct_wifi_config, WICED_TRUE );
                return true;
            }
        }

        analyse_failed_join_result( result );
        wiced_dct_read_unlock( (void*) dct_wifi_config, WICED_TRUE );

        return false;
    }
    print_status( "Network up\r\n" );

    strlcpy( last_joined_ssid, ssid, MIN( sizeof(last_joined_ssid), ssid_length+1 ) );

    wiced_dct_read_unlock( (void*) dct_wifi_config, WICED_TRUE );

    return true;
}

/*!
 ******************************************************************************
 * Analyse failed join result
 *
 * @param[in] join_result  Result of join attempts.
 *
 * @return
 */
static void analyse_failed_join_result( wiced_result_t join_result )
{
    /* Note that DHCP timeouts and EAPOL key timeouts may happen at the edge of the cell. If possible move closer to the AP. */
    /* Also note that the default number of join attempts is three and the join result is returned for the last attempt. */
    WPRINT_APP_INFO( ("Join result %u: ", (unsigned int)join_result) );
    switch( join_result )
    {
        case WICED_ERROR:
            cli_print("General error\n" ); /* Getting a DHCP address may fail if at the edge of a cell and the join may timeout before DHCP has completed. */
            break;

        case WWD_NETWORK_NOT_FOUND:
            cli_print("Failed to find network\n" ); /* Check that he SSID is correct and that the AP is up */
            break;

        case WWD_NOT_AUTHENTICATED:
            cli_print("Failed to authenticate\n" ); /* May happen at the edge of the cell. Try moving closer to the AP. */
            break;

        case WWD_EAPOL_KEY_PACKET_M1_TIMEOUT:
            cli_print("Timeout waiting for first EAPOL key frame from AP\n" );
            break;

        case WWD_EAPOL_KEY_PACKET_M3_TIMEOUT:
            cli_print("Check the passphrase and try again\n" ); /* The M3 timeout will occur if the passphrase is incorrect */
            break;

        case WWD_EAPOL_KEY_PACKET_G1_TIMEOUT:
            cli_print("Timeout waiting for group key from AP\n" );
            break;

        case WWD_INVALID_JOIN_STATUS:
            cli_print("Some part of the join process did not complete\n" ); /* May happen at the edge of the cell. Try moving closer to the AP. */
            break;

        default:
            cli_print("\n" );
            break;
    }
}

