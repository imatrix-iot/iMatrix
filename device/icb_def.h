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
/*
 * deb_def.h
 *
 *  Created on: Dec 28, 2016
 *      Author: greg.phillips
 */

#ifndef ICB_DEF_H_
#define ICB_DEF_H_

/** @file icb_def.h
 *
 *	iMatrix Control Block - Initialized in storage.h
 *
 */
#include "../coap/coap.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/
enum {
    UDP_STATS,
    TCP_STATS,
    NO_PROTOCOL_STATS,
};
enum main_state_t {
    MAIN_WIFI_SETUP,
    MAIN_NO_WIFI_SETUP,
    MAIN_NO_WIFI,
    MAIN_WIFI,
};


/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct {
    uint32_t packets_received;
    uint32_t packets_unitcast_received;
    uint32_t packets_received_errors;
    uint32_t packets_multicast_received;
    uint32_t packet_creation_failure;
    uint32_t fail_to_send_packet;
    uint32_t packets_sent;
    wiced_result_t rec_error;
} ip_stats_t;

typedef struct {
	uint8_t *root_certificate;
	uint8_t *wifi_8021X_key;
	uint8_t *wifi_8021X_certificate;
	uint8_t *dtls_certificate;
	uint16_t wifi_state;
	uint16_t no_ble_updates;
	uint16_t no_ble_devices;
	uint16_t no_host_coap_entries;
	uint32_t print_msg;
	uint32_t print_telnet_msg;
	uint32_t boot_count;
	uint32_t wifi_dropouts;
	uint32_t dns_failure_count;
	uint32_t imatrix_upload_count;
	uint32_t AT_commands_processed;
	uint32_t AT_command_errors;
	uint32_t spi_errors;
    uint32_t wifi_failed_connect_count;
    uint32_t wifi_success_connect_count;
	uint32_t indoor_x, indoor_y, indoor_z, indoor_level;
	float longitude, latitude, elevation;
	ip_stats_t ip_stats[ NO_PROTOCOL_STATS ];
	wiced_ip_address_t gw_ip;
	wiced_ip_address_t my_ip;
    wiced_ip_address_t ap_ip_address;
    wiced_ip_address_t st_ip_address;
    wiced_ip_address_t imatrix_public_ip_address;
    wiced_ip_address_t imatrix_private_ip_address;
    wiced_ip_address_t syslog_public_ip_address;
    wiced_ip_address_t syslog_private_ip_address;
	wiced_time_t dns_lookup_time;
	wiced_time_t reboot_time;
	wiced_utc_time_t fake_utc_boot_time;
	wiced_utc_time_t boot_time;
	wiced_utc_time_t sunrise;
	wiced_utc_time_t sunset;
	uint32_t var_pool_size;
	void *ccm_heap_start;
	void *ccm_heap_end;
	void *ccm_next_entry;
	void (*wifi_notification)( bool state );
	CoAP_entry_t *coap_entries;
	unsigned int imatrix_no_packet_avail    : 1;
	unsigned int using_ccmsram              : 1;
	unsigned int running_in_background      : 1;
	unsigned int comm_mode                  : 4;
	unsigned int dns_lookup			        : 1;
	unsigned int send_manfuacturing	        : 1;
    unsigned int registration_in_process    : 1;
	unsigned int send_registration          : 1;
	unsigned int send_gps_coords	        : 1;
	unsigned int send_indoor_coords	        : 1;
	unsigned int send_host_sw_revision      : 1;
	unsigned int ble_initialized	        : 1;
	unsigned int arduino_valid		        : 1;
	unsigned int wifi_change		        : 1;
	unsigned int locked_dowm		        : 1;
	unsigned int ble_scan_active	        : 1;
	unsigned int get_latest_active	        : 1;
	unsigned int ota_loader_active          : 1;
	unsigned int reboot				        : 1;
	unsigned int wifi_up			        : 1;
    unsigned int time_set_with_NTP	        : 1;	// Time was set with real NTP
} iMatrix_Control_Block_t;
/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

#endif /* ICB_DEF_H_ */
