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
 * so agrees to indemnity Sierra against all liability.
 */

/** @file factory_def.c
 *
 *  Created on: Sept 1, 2017
 *      Author: greg.phillips
 */



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
const IOT_Device_Config_t factory_default_config = {
    .product_name = "",
    .device_name = "",
    .sn.serial1 = 0,
    .sn.serial2 = 0,
    .sn.serial3 = 0,
    .device_serial_number = "",
    .default_ap_ssid = "",
    .default_ap_wpa = "",
    .default_st_ssid = "",
    .default_st_wpa = "",
    .ap_ssid = "",
    .ap_wpa = "",
    .st_ssid = "",
    .st_wpa = "",
    .password = "",
    .imatrix_public_url = "dev.coap.imatrix.io",
    .ota_public_url = "dev.ota.imatrix.io",
    .product_id = 0,
    .manufactuer_id = 0,
    .last_ntp_updated_time = 0,
    .reboots = 0,
    .default_ap_security_mode = 0,
    .default_st_security_mode = 0,
    .default_ap_eap_mode = 0,
    .default_st_eap_mode = 0,
    .ap_security_mode = 0,
    .st_security_mode = 0,
    .ap_eap_mode = 0,
    .st_eap_mode = 0,
    .no_sensors = 0,
    .no_controls = 0,
    .AT_variable_data_timeout = IMX_AT_VAR_DATA_TIMEOUT,
    .imatrix_batch_check_time = 1000,              // Check every 1 seconds
    .ota_fail_sflash_write = 0,
    .ota_fail_sflash_crc = 0,
    .ota_fail_communications_link = 0,
    .valid_config = IMX_MAGIC_CONFIG,
    .local_seconds_offset_from_utc = -28800,    // PST
    .latitude = IMX_FACTORY_LATITUDE_DEFAULT,
    .longitude = IMX_FACTORY_LONGITUDE_DEFAULT,             // Zephyr Cove Office
    .elevation = IMX_FACTORY_ELEVATION_DEFAULT,
    .indoor_x = 0,
    .indoor_y = 0,
    .level_id = 0,
    .building_id = 0,
    .imatrix_enabled = 1,
    .cli_enabled = 1,
    .telnet_enabled = 1,
    .ssh_enabled = 1,
    .username_password_enabled = 1,
    .comm_mode = COMM_TCP,
    .mobile_device = false,
    .indoor_device = false,
    .enable_imatrix = true,
    .use_rssi = true,
    .use_rfnoise = true,
    .use_wifi_channel = true,
    .use_temperatue = true,
    .use_red_led = true,
    .use_green_led = true,
    .send_now_on_warning_level = IMX_ADVISORY,
    .provisioned = 0,
    .do_SFLASH_load = 0,
    .AP_setup_mode = 1,
    .connected_to_imatrix = 1,      // Use this during provisioning to reset if it fails
    .log_to_imatrix = 0,
    .AT_echo = true,
    .AT_verbose = IMX_AT_VERBOSE_STANDARD_STATUS,
    .log_wifi_AP = false,
    .log_wifi_rssi = false,
    .log_wifi_rfnoise = false,
    // .ccb = { 0 },
    // .scb = { 0 },
};
/******************************************************
 *               Function Definitions
 ******************************************************/
