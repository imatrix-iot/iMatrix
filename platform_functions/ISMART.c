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

/** @file ISMART_ism43340.c
 *
 *  Created on: October 20, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "wiced.h"

#include "../cli/interface.h"
#include "../storage.h"
#include "../device/lcb_def.h"
#include "ISMART.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define ON_BOARD_LED_RED                WICED_LED2
#define ON_BOARD_LED_GREEN              WICED_LED1

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
static wiced_result_t thermistor_take_sample(wiced_adc_t adc, float* celcius );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief init_led_red
  * @param  None
  * @retval : None
  */
void imx_init_led_red_ismart(uint16_t arg)
{
    UNUSED_PARAMETER(arg);

    wiced_gpio_init( ON_BOARD_LED_RED, OUTPUT_PUSH_PULL );  // GPIO 3 - Red Led
    imx_update_led_red_status_ismart( false );
}
/**
  * @brief init_led_red
  * @param  None
  * @retval : None
  */
void imx_init_led_green_ismart(uint16_t arg)
{
    UNUSED_PARAMETER(arg);

    wiced_gpio_init( ON_BOARD_LED_GREEN, OUTPUT_PUSH_PULL );  // GPIO 4 - Green Led
    imx_update_led_green_status_ismart( false );
}
/**
  * @brief init_led_red
  * @param  None
  * @retval : None
  */
void imx_init_led_blue_ismart(uint16_t arg)
{
    UNUSED_PARAMETER(arg);

    // Add init for Blue
    imx_update_led_blue_status_ismart( false );
}

/**
  * @brief update_led_red_status
  * @param  None
  * @retval : None
  */
void imx_update_led_red_status_ismart( bool state )
{
    if( state == true ) {
        wiced_gpio_output_high( ON_BOARD_LED_RED );
    } else {
        wiced_gpio_output_low( ON_BOARD_LED_RED );
    }
}
/**
  * @brief update_led_green_status
  * @param  None
  * @retval : None
  */
void imx_update_led_green_status_ismart( bool state )
{
    if( state == true ) {
        wiced_gpio_output_high( ON_BOARD_LED_GREEN );
    } else {
        wiced_gpio_output_low( ON_BOARD_LED_GREEN );
    }

}
/**
  * @brief update_led_blue_status
  * @param  None
  * @retval : None
  */
void imx_update_led_blue_status_ismart( bool state )
{
    if( state == true ) {
        // wiced_gpio_output_high( ON_BOARD_LED_BLUE );
    } else {
        // wiced_gpio_output_low( ON_BOARD_LED_BLUE );
    }

}
/**
  * @brief initialize the hardware for on board temperature sensing
  * @param  None
  * @retval : None
  */

void imx_init_temp(uint16_t arg)
{
    UNUSED_PARAMETER(arg);
    /*
     * Temperature sensor connected to ADC0
     *
     * Standard board uses thermister additional support for onewire
     */
    wiced_result_t result;

    result = wiced_adc_init( WICED_ADC_1, 10 );
    if( result != WICED_SUCCESS )
        imx_printf( "Unable to set up Analog 0 input\r\n" );
    else
        imx_printf( "Set up Analog 0 input\r\n" );
}
/**
  * @brief sample the temperature
  * @param  None
  * @retval : None
  */
uint16_t imx_sample_temp(uint16_t arg, void *value )
{
    float foo;
    int temp;
    wiced_result_t result;

    UNUSED_PARAMETER(arg);

    result = thermistor_take_sample(WICED_ADC_1, &foo );
    if( result != WICED_SUCCESS ) {
        imx_printf( "Failed to read thermistor\r\n" );
        return IMX_GENERAL_FAILURE;
    }
    //imx_printf( "Temperature: %3.2fC\r\n", foo );
    /*
     * Onboard returns float, but value is actually just int so change
     */
    temp = (int32_t) foo;
    memcpy( value, &temp, sizeof( temp ) );
    return IMATRIX_SUCCESS;
}
/**
  * @brief Read the ADC0 input to determine temperature
  * @param  None
  * @retval : None
  */
static wiced_result_t thermistor_take_sample(wiced_adc_t adc, float* celcius )
{
    char buffer[ 20 ];
    uint16_t sample_value;
    wiced_result_t result = wiced_adc_take_sample(adc, &sample_value);

    /* Thermistor is Murata NCP18XH103J03RB  (Digi-key 490-2436-1-ND )
     *
     * Part Number details:
     * NC  : NTC Chip Thermistor
     * P   : Plated termination
     * 18  : Size 0603
     * XH  : Temperature Characteristics : Nominal B-Constant 3350-3399K
     * 103 : Resistance 10k
     * J   : Tolerance   +/- 5%
     * 03  : Individual Specs: Standard
     * RB  : Paper Tape 4mm pitch, 4000pcs
     *
     *
     * It has a 43K feed resistor from 3V3
     *
     * Thermistor Voltage    = V_supply * ADC_value / 4096
     * Thermistor Resistance = R_feed / ( ( V_supply / V_thermistor ) - 1 )
     * Temp in kelvin = 1 / ( ( ln( R_thermistor / R_0 ) / B ) + 1 / T_0 )
     * Where: R_feed = 43k, V_supply = 3V3, R_0 = 10k, B = 3375, T_0 = 298.15°K (25°C)
     */
    if (result == WICED_SUCCESS)
    {
        // print_status( "Sample: %u - ", sample_value );
        double thermistor_resistance = 43000.0 / ( ( 4096.0 / (double) sample_value ) - 1 );
        double logval = log( thermistor_resistance / 10000.0 );
        double temperature = ( 1.0 / ( logval / 3380.0 + 1.0 / 298.15 ) - 273.15 );
        temperature = ( temperature ) - 18.0;   // Calibration adjustment for ISMART
        //print_status( "Temp: %f - ", (float) temperature );
        temperature = (temperature > (floor(temperature)+0.5f)) ? ceil(temperature) : floor(temperature);
        sprintf( buffer, "%0.1f", temperature );
        *celcius = (float) atof( buffer );
        //print_status( "Value: %s, %f", buffer, *celcius );
        return WICED_SUCCESS;
    }
    else
    {
        *celcius = 0;
        return result;
    }
}
