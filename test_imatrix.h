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

/** @file .h
 *
 *  Created on: October 14, 2017
 *      Author: greg.phillips
 *
 */

#ifndef TEST_IMATRIX_H_
#define TEST_IMATRIX_H_

/*
 *	Defines for
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
/*
* Product and Operating details
*/
#define IMX_ORGANIZATION_ID    1583546208
#define IMX_PRODUCT_ID         0xb14adfc
#define IMX_PRODUCT_NAME       "Apollo LED Light Controller"
#define IMX_MANUFACTURING_SITE "bind.imatrix.io"
#define IMX_IMATRIX_SITE       "sierratelecom.imatrix.io"
#define IMX_OTA_SITE           "ota.imatrix.io"
/*
* Define Control IDs for Controls
*/
/*
* Integrated Controls
*/
#define IMX_NO_INTEGRATED_CONTROLS 0
/*
* Product Controls
*/
#define DIM_LEVEL_1598884125            0x5f4d091d
#define FIXTURE_1775572943              0x69d517cf
#define LED_FIXTURE_GREEN_1610619002    0x6000187a
#define LED_FIXTURE_RED_190481833       0xb5a85a9
#define IMX_NO_PRODUCT_CONTROLS             ( 4 )
/*
* Smart Arduino Controls
*/
#define IMX_NO_ARDUINO_CONTROLS ( 0 )

#define CCB_DIM_LEVEL_1598884125        0x0
#define CCB_FIXTURE_1775572943          0x1
#define CCB_LED_FIXTURE_GREEN_1610619002 0x2
#define CCB_LED_FIXTURE_RED_190481833   0x3

#define IMX_NO_CONTROLS ( IMX_NO_INTEGRATED_CONTROLS + IMX_NO_PRODUCT_CONTROLS )

/*
* Define Sensor IDs for Sensors
*/
/*
* Integrated Sensors
*/
#define WIFI_RSSI_101600364             0X060E4C6C
#define WIFI_CHANNEL_173916172          0X0A5DC00C
#define WIFI_RF_NOISE_1248909411        0X04A70D863
#define IMX_NO_INTEGRATED_SENSORS           ( 3 )
/*
* Product Sensors
*/
#define HUMIDITY_1999303476             0x772af334
#define LUX_LEVEL_662034396             0x2775d7dc
#define OCCUPANCY_COUNT_1354634051      0x50be1343
#define PIR_OCCUPANCY_678141803         0x286b9f6b
#define POWER_955933699                 0x38fa6403
#define SEISMIC_610683442               0x24664a32
#define SMART_OCCUPANCY_934172293       0x37ae5685
#define SOUND_LEVEL_814393198           0x308aa76e
#define SOUND_OCCUPANCY_1912437427      0x71fd7ab3
#define TEMPERATURE_1654801688          0x62a24518
#define IMX_NO_PRODUCT_SENSORS              ( 10 )
/*
* Smart Arduino Sensors
*/
#define IMX_NO_ARDUINO_SENSORS              ( 0 )

#define SCB_WIFI_RSSI_101600364         0x0
#define SCB_WIFI_CHANNEL_173916172      0x1
#define SCB_WIFI_RF_NOISE_1248909411    0x2
#define SCB_HUMIDITY_1999303476         0x3
#define SCB_LUX_LEVEL_662034396         0x4
#define SCB_OCCUPANCY_COUNT_1354634051  0x5
#define SCB_PIR_OCCUPANCY_678141803     0x6
#define SCB_POWER_955933699             0x7
#define SCB_SEISMIC_610683442           0x8
#define SCB_SMART_OCCUPANCY_934172293   0x9
#define SCB_SOUND_LEVEL_814393198       0xA
#define SCB_SOUND_OCCUPANCY_1912437427  0xB
#define SCB_TEMPERATURE_1654801688      0xC

#define IMX_NO_SENSORS ( IMX_NO_INTEGRATED_SENSORS + IMX_NO_PRODUCT_SENSORS )

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

#endif /* TEST_IMATRIX_H_ */
