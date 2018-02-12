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
 *	hal_rssi - get the current rssi from the active link
 *  Created on: December, 2015
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../device/icb_def.h"
#include "../device/config.h"

#include "hal_wifi.h"

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
extern iMatrix_Control_Block_t icb;
extern IOT_Device_Config_t device_config;

/******************************************************
 *               Function Definitions
 ******************************************************/
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Init RSSI
  * @param  None
  * @retval : None
  */
void imx_init_rssi(void)
{

}
/**
  * @brief	get the rssi from the wifi radio
  * @param  None
  * @retval : None
  */
imx_result_t  imx_sample_rssi(uint16_t arg, void *value )
{
	int32_t foo;
	UNUSED_PARAMETER(arg);

	foo = hal_get_wifi_rssi();
	if( foo != 0 ) {
		memcpy( value, &foo, sizeof( foo ) );
		return IMX_SUCCESS;
	} else
		return IMX_NO_DATA;
}
/**
  * @brief	get the rssi from the wifi radio
  * @param  None
  * @retval : None
  */
int32_t hal_get_wifi_rssi(void)
{
	int32_t rssi;

	rssi = 0;
    if( ( icb.wifi_up == true ) && ( device_config.AP_setup_mode == false ) )
		wwd_wifi_get_rssi( &rssi );

	return( rssi );
}
/**
  * @brief  Init RF Noise
  * @param  None
  * @retval : None
  */
void imx_init_rfnoise(uint16_t arg)
{

}
/**
  * @brief	get the rssi from the wifi radio
  * @param  None
  * @retval : None
  */
imx_result_t  imx_sample_rfnoise(uint16_t arg, void *value )
{
	int32_t foo;
	UNUSED_PARAMETER(arg);

	foo = hal_get_wifi_rfnoise();
	if( foo != 0 ) {
		memcpy( value, &foo, sizeof( foo ) );
		return IMATRIX_SUCCESS;
	} else
		return IMX_NO_DATA;
}
/**
  * @brief	get the noise from the wifi radio
  * @param  None
  * @retval : None
  */
int32_t hal_get_wifi_rfnoise(void)
{
	int32_t noise;

	noise = 0;
	if( ( icb.wifi_up == true ) && ( device_config.AP_setup_mode == false ) )
		wwd_wifi_get_noise( &noise );

	return( noise );
}
/**
  * @brief	get the noise from the wifi radio
  * @param  None
  * @retval : None
  */
int16_t hal_get_wifi_tx_power(void)
{
	uint8_t tx_power;

	wwd_wifi_get_tx_power( &tx_power );

	return( (uint16_t) tx_power );
}
/**
  * @brief	get the rssi from the wifi radio
  * @param  None
  * @retval : None
  */
imx_result_t  imx_sample_wifi_channel(uint16_t arg, void *value )
{
	int32_t foo;
	UNUSED_PARAMETER(arg);

	foo = hal_get_wifi_channel();
	if( foo != 0 ) {
		memcpy( value, &foo, sizeof( foo ) );
		return IMATRIX_SUCCESS;
	} else
		return IMX_NO_DATA;
}
/**
  * @brief	get the wifi radio channel
  * @param  None
  * @retval : None
  */
uint32_t hal_get_wifi_channel(void)
{
	uint32_t channel;

	channel = 0;
	if( icb.wifi_up == true ) {
		if( device_config.AP_setup_mode == true )
			wwd_wifi_get_channel( WWD_AP_INTERFACE, &channel );
		else
			wwd_wifi_get_channel( WWD_STA_INTERFACE, &channel );
	}
	return( channel );
}
/**
  * @brief  get the BSSID from the wifi radio
  * @param  pointer to BSSID
  * @retval : None
  */
void hal_get_wifi_bssid( wiced_mac_t *bssid )
{

    if( ( icb.wifi_up == true ) && ( device_config.AP_setup_mode == false ) )
        wwd_wifi_get_bssid( bssid );
}
/**
  * @brief  Additional modules used in sensor definitions
  * @param  None
  * @retval : None
  */
void imx_init_wi_fi_bssid(uint16_t arg)
{

}
imx_result_t  imx_sample_wi_fi_bssid(uint16_t arg, void *value )
{
    UNUSED_PARAMETER(arg);

    return IMX_NO_DATA;
}
void imx_init_wi_fi_channel(uint16_t arg)
{

}
imx_result_t  imx_sample_wi_fi_channel(uint16_t arg, void *value )
{
    UNUSED_PARAMETER(arg);

    return IMX_NO_DATA;
}
