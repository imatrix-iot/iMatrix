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
/** @file cli.c
 *
 *
 *  Created on: December, 2016
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "wiced.h"
#include "wwd_constants.h"
#include "besl_constants.h"
#include "platform.h"

#include "../storage.h"
#include "../arduino/arduino.h"
#include "../at_cmds/at_cmds.h"
#include "../ble/ble_manager.h"
#include "../device/icb_def.h"
#include "../device/config.h"
#include "../imatrix/imatrix.h"
#include "telnetd.h"
#include "interface.h"
#include "print_dct.h"
#include "cli_debug.h"
#include "cli_help.h"
#include "cli_dump.h"
#include "cli_reboot.h"
#include "cli_status.h"
#include "cli_set_serial.h"
#include "cli_set_ssid.h"
#include "print_dct.h"
#include "cli.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define RX_BUFFER_SIZE			64
#define COMMAND_LINE_LENGTH		129	// 128 Characters + 1 NULL
#define SENSOR_TEST_COUNT		1000
#define UINT16_T_MAX            0xFFFF
#define DO_NOT_STOP_STORM       ( UINT16_T_MAX )

/******************************************************
 *                   Enumerations
 ******************************************************/
enum cli_states {
    CLI_SETUP_CONSOLE,
    CLI_SETUP_TELNET,
    CLI_GET_CMD,
	CLI_PROCESS_CMD
};
enum cmds {				// Must match commands variable order
	CLI_HELP, 			// ?
	CLI_ARDUINO,		// Get status of Arduino
	CLI_AT_CMD,			// AT Commands
	CLI_BLE_SCAN_PRINT,	// Print results of BLE Scan
	CLI_BLE_SCAN_START,	// Start a background BLE scan
	CLI_BLE_SCAN_STOP,	// Stop a background BLE scan
	CLI_DEBUG,          // Turn On / Off Debug messages
	CLI_GET_MAC,		// Get MAC and Serial Number
	CLI_IMATRIX,        // Status of iMatrix Subsystem
	CLI_PRINT_CONFIG,	// Print the configuration
	CLI_PRINT_DCT,		// Print the DCT
	CLI_LED,			// Set the LED and Mode
	CLI_DUMP_MEMORY,	// dump internal memory
	CLI_MFG_TEST,       // Do a manufacturing test / function
	CLI_OTA_GET_LATEST, // Load the latest firmware using OTA
	CLI_DUMP_SFLASH,	// dump sflash memory
	CLI_SETUP_MODE,		// Set to setup mode for provisioning
	CLI_REBOOT,			// reboot
	CLI_PRINT_LUT,      // print the LUT
	CLI_PRINT_STATUS,	// s
	CLI_SSID,			// set SSID and passphrase
	CLI_SET_SERIAL,		// Set the serial number of a unit
	NO_CMDS
};
/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
void cli_set_led( uint16_t arg);
/******************************************************
 *               Variable Definitions
 ******************************************************/
static uint16_t cli_state;
extern uint16_t active_device;
extern IOT_Device_Config_t device_config;
static wiced_uart_config_t uart_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};

static wiced_ring_buffer_t rx_buffer;
static uint8_t rx_data[ RX_BUFFER_SIZE ];
static char terminal_command_line[ COMMAND_LINE_LENGTH ], telnet_command_line[ COMMAND_LINE_LENGTH ], *command_line;
static uint16_t terminal_cmd_index, telnet_cmd_index, *cmd_index, cmd_found;

cli_commands_t command[ NO_CMDS ] = {
		{ "?",	&cli_help, NO_CMDS,  "Print this help" },	// Help
		{ "a", &print_arduino, 0, "Print the status of the Arduino sub system" },
		{ "AT", &cli_at, 0, "AT Commands &IC/&IS to set Controls & Sensor values" },
		{ "ble_print", &print_ble_scan_results, 0, "Print BLE Scan Results" },
		{ "ble_start", &ble_scan, true, "Start background BLE Scan" },
		{ "ble_stop", &ble_scan, false, "Stop background BLE Scan" },
		{ "c", &imatrix_print_config, 0, "Print the device Configuration" },
		{ "debug", &cli_debug, 0, "Debug <on|off>", },
		{ "dct", &print_dct, 0, "Print the DCT" },
        { "f", &cli_dump, DUMP_SFLASH, "f [ <start address> ] [ <length> ] if no start, start at 0, if no length, print out 1k of SFLASH data" },
		//{ "get_mac", &get_sn_mac, 0, "Get MAC and SN from Manufacturing server" },
		//{ "get_latest", &cli_get_latest, 0, "Load the latest firmware with OTA" },
		//{ "imatrix", &imatrix_status, 0, "Display status of iMatrix client system" },
		//{ "l", &print_lut, 0, "Print the LUT" },
		//{ "led", &cli_set_led, 0, "Set led state <led led_no | state <on|off|blink_rate per second>" },
		//{ "m", &cli_dump, DUMP_MEMORY, "m [ <start address> ] [ <length> ] if no start, start at 0, if no length, print out 1k of SRAM data" },
		//{ "mfg", mfg_test, 0, "mfg <test/function number>" },
		//{ "reboot", &cli_reboot, 0, "reboot the device" },	// reboot the device
		{ "s", &cli_status, 0, "Print the status" },		// Current Status
		//{ "setup", &cli_wifi_setup, 0, "setup <on | off> - Sets to use Soft AP to provision" },
        { "set_serial", &cli_set_serial, 0, "Set the serial number of a unit <serial number>" },    // Set SSID and PSK
		{ "ssid", &cli_set_ssid, 0, "Set the SSID and PSK for WPA2PSK mode. ssid <ssid> <passphrase>" },	// Set SSID and PSK
};
/******************************************************
 *               Function Definitions
 ******************************************************/

#include "cli.h"
/**
  * @brief	Initialize the cli
  * @param  None
  * @retval : None
  */
void cli_init(void)
{
    /* Initialize ring buffer */
    ring_buffer_init(&rx_buffer, rx_data, RX_BUFFER_SIZE );

    /* Initialize UART. A ring buffer is used to hold received characters */
    wiced_uart_init( STDIO_UART, &uart_config, &rx_buffer );
    memset( terminal_command_line, 0x00, COMMAND_LINE_LENGTH );
    terminal_cmd_index = 0;
    memset( telnet_command_line, 0x00, COMMAND_LINE_LENGTH );
    telnet_cmd_index = 0;
	active_device = CONSOLE_OUTPUT;
    cli_print( "Command Line Processor\r\n" );
	cli_state = CLI_SETUP_CONSOLE;
}

/**
  * @brief	process the command line entries
  * @param  None
  * @retval : None
  */
void cli_process( void )
{
	char ch, *token;
	uint16_t i;
	uint32_t expected_data_size;

	switch( cli_state ) {
		case CLI_SETUP_CONSOLE :
			memset( terminal_command_line, 0x00, COMMAND_LINE_LENGTH );
			terminal_cmd_index = 0;
            active_device = CONSOLE_OUTPUT;     // This is so when general status print out are made with no cli activity they will go to the console
			if( device_config.AT_verbose == IMX_AT_VERBOSE_STANDARD_STATUS )
			    cli_print( ">" );
			cli_state = CLI_GET_CMD;
			break;
        case CLI_SETUP_TELNET :
            memset( telnet_command_line, 0x00, COMMAND_LINE_LENGTH );
            telnet_cmd_index = 0;
            active_device = TELNET_OUTPUT;     // This is so when general status print out are made with no cli activity they will go to the console
            cli_print( ">" );
            cli_state = CLI_GET_CMD;
            break;
		case CLI_GET_CMD :
			expected_data_size = 1;
			if( wiced_uart_receive_bytes( STDIO_UART, &ch, &expected_data_size, (uint32_t) 0 ) == WICED_SUCCESS ) {
				/*
				 * Got a character from the terminal
				 */
				active_device = CONSOLE_OUTPUT;
				cmd_index = &terminal_cmd_index;
				command_line = &terminal_command_line[ 0 ];
			} else if( telnetd_getch( &ch ) ) {
				/*
				 * Got a character from the telnet session
				 */
				active_device = TELNET_OUTPUT;
				cmd_index = &telnet_cmd_index;
				command_line = &telnet_command_line[ 0 ];
			} else {
				return;
			}
			/*
			 * Got a character - process
			 */
			// print_status( "Processing character[0x%02x] from device: %u\r\n", (uint16_t) ch, active_device );
			switch( ch ) {
				case CHR_BS :	// Rubout the last
					if( *cmd_index > 0 ) {
						cli_print( "\b \b" );
						*cmd_index -= 1;
						command_line[ *cmd_index ] = 0x00;
					}
					break;
				case CHR_LF :
				case CHR_CR :	// End of input
					cli_print( "\r\n" );
					cli_state = CLI_PROCESS_CMD;
					break;
				default :
					if( *cmd_index < COMMAND_LINE_LENGTH - 2 )	 {	// Buffer must have room for null at end
						if( isprint( (uint16_t) ch ) ) {
							command_line[ *cmd_index ] = ch;
							*cmd_index += 1;
							cli_print( "%c", ch );
						}
						else
							cli_print( "\x07" );	// Only accept printable characters
					} else
						cli_print( "\x07" );	// tell user maximum buffer length
					break;
			}
			break;
		case CLI_PROCESS_CMD :
		    // print_status( "Processing command line: >%s<\r\n", command_line );
			token = strtok( command_line, " " );
			if( token != NULL ) {
				cmd_found = false;// Exit do loop when true.
				i = 0;// Exit do loop when i == NO_CMDS
				do {
					if( strcmp( token, command[ i ].command_string ) == 0x00 ) {
						cmd_found = true;
						if( *command[ i ].cli_function != NULL )
							(*command[ i ].cli_function)( command[ i ].arg );
					}
					i++;
				} while ( ( i < NO_CMDS ) && ( cmd_found == false ) );
				if( cmd_found == false )
					cli_print( "Unknown Command: %s\r\n", token );
			}
			if( active_device == CONSOLE_OUTPUT )
			    cli_state = CLI_SETUP_CONSOLE;
			else
			    cli_state = CLI_SETUP_TELNET;
			break;
		default :
			cli_state = CLI_SETUP_CONSOLE;
			break;
	}
}

