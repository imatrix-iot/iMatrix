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

/** @file integrated.h
 *
 *  Created on: Feb 2, 2017
 *      Author: greg.phillips
 */

#ifndef ARDUINO_H_
#define ARDUINO_H_


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define SPI_COMMAND_LENGTH 	    5
#define MAX_SPI_RESPONSE        42
#define SPI_SOH                 0
#define SPI_COMMAND             1
#define SPI_REGISTER            2
#define SPI_LENGTH              3
#define SPI_EOT                 4
/*
 * Define SPI interface commands
 */
#define SPI_NOP			        0
#define SPI_WRITE_REGS	        1
#define SPI_READ_REGS	        2
#define ARDUINO_REG_LENGTH	    4
/*
 * Define values for CSR
 */
#define	ARDUINO_NO_CHANGE	0	// – All operating normally – No update
#define	ARDUINO_NEW_VALUE	1	// – New value stored
#define	ARDUINO_ERROR		2 	// – Control/Sensor is in Error
#define	ARDUINO_UNDEFINED	3 	// – Undefined Control/Sensor
/*
 * Define Registers
 */
#define SPI_CONFIG_REG	0	//	Configuration Register
#define SPI_SYS_STATUS	1	//	System Status Code
#define SPI_ARD_STATUS	2	//	Arduino Status Code (Read Only from ConnectKit)
#define SPI_CSR_MASTER	3	//	Control / Sensor Status Register - Master
#define SPI_CSR_SLAVE	4	//	Control / Sensor Status Register – Slave (Read Only from ConnectKit)
#define SPI_CONTROL_0	5	//	4-12	Control Values (Read / Write)
#define SPI_CONTROL_1	6
#define SPI_CONTROL_2	7
#define SPI_CONTROL_3	8
#define SPI_CONTROL_4	9
#define SPI_CONTROL_5	10
#define SPI_CONTROL_6	11
#define SPI_CONTROL_7	12
#define SPI_SENSOR_0	13	// 13-20	Sensor Values (Read Only from ConnectKit)
#define SPI_SENSOR_1	14
#define SPI_SENSOR_2	15
#define SPI_SENSOR_3	16
#define SPI_SENSOR_4	17
#define SPI_SENSOR_5	18
#define SPI_SENSOR_6	19
#define SPI_SENSOR_7	20
#define SPI_MAX_BYTE	( ( SPI_SENSOR_7 * ARDUINO_REG_LENGTH ) + ARDUINO_REG_LENGTH )

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
void init_arduino(void);
void init_arduino_control( uint16_t arg );
void configure_arduino(void);
void process_arduino( wiced_time_t current_time );
uint16_t update_arduino_control(uint16_t arg, void *value );
void print_arduino( uint16_t arg );

#endif /* ARDUINO_H_ */
