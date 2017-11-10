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
 * .h
 *
 *  Created on: Oct 23, 2013
 *      Author: greg.phillips
 */

#ifndef WIFI_H_
#define WIFI_H_

/** @file wifi.h
 *
 * Function definitions for wifi.c
 *
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
 *               Function Definitions
 ******************************************************/

uint16_t wifi_init(void);
void wifi_shutdown();
uint16_t check_valid_ip_address(void);
//void wifi_teardown(void);
void wifi_logging(void);
char* get_wifi_ssid( char* buffer, uint16_t index );
void wifi_set_default_ap_ssid(void);
void set_wifi_ap_ssid( char *ssid, char *passphrase, wiced_security_t security );
void wifi_set_default_st_ssid(void);
void set_wifi_st_ssid( char *ssid, char *passphrase, wiced_security_t security, uint16_t eap_mode );

void st_printf_wifi_ssid(void);
void link_up(void);
void link_down(void);
void stop_network();
void set_fixture_dct_name(void);
void cli_wifi_setup( uint16_t arg );
#endif /* _H_ */

