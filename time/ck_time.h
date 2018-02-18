/*
 * $Copyright 2014, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/*
 * .h
 *
 *  Created on: Oct 23, 2013
 *      Author: greg.phillips
 */

#ifndef CK_TIME_H_
#define CK_TIME_H_

/** @file
 *
 * defines for the time system
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
//void time_init(void);

void start_random_delay_timer_for_sntp();

wiced_result_t start_stopped_auto_time_sync_if_random_delay_is_finished();

wiced_result_t sntp_restart_auto_time_sync();

char* sprintf_iso8601_time(char* str_out, uint32_t max_length, wiced_iso8601_time_t *time);
void imx_set_local_seconds_offset_from_utc( int32_t local_seconds_offset_from_utc );
int32_t imx_get_local_seconds_offset_from_utc(void);
wiced_utc_time_t imx_current_local_time(void);
uint16_t imx_day_of_week(wiced_utc_time_t seconds_since_1969);
wiced_result_t wiced_local_time_get_iso8601_time(wiced_iso8601_time_t* iso8601_time);
wiced_result_t iso8601_time_of(wiced_utc_time_t utc_time, wiced_iso8601_time_t* iso8601_time);
void imx_sunrise_sunset( wiced_utc_time_t any_utc_time_on_day, wiced_utc_time_t* sunrise, wiced_utc_time_t* sunset );
wiced_time_t time_difference( wiced_time_t latest, wiced_time_t earliest );
bool imx_is_later(  wiced_time_t time1, wiced_time_t time2 );
uint16_t timer_timeout( wiced_time_t current_time, wiced_time_t start, uint32_t wait );
uint16_t save_ntp_information_to_config_when_needed();
uint16_t imx_ntp_succeeded_at_least_once(void);

#endif /* CK_TIME_H_ */
