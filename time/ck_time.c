/*
 * $Copyright 2014, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file
 *
 *  ck_time.c
 *  Time related functions for the ConnectKit
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "wiced_crypto.h"
#include "sntp.h"

#include "../system.h"
#include "../defines.h"
#include "../hal.h"
#include "../cli/interface.h"
#include "../device/device_app_dct.h"
#include "../device/dcb_def.h"
#include "../device/config.h"
#include "ck_time.h"


/******************************************************
 *                      Macros
 ******************************************************/
#define PI 3.14159265359
/******************************************************
 *                    Constants
 ******************************************************/
#define TIME_SYNC_PERIOD (1 * DAYS)

// constants copied from WICED\internal\time.c
#define SECONDS_IN_365_DAY_YEAR  (31536000)
#define SECONDS_IN_A_DAY         (86400)
#define SECONDS_IN_A_HOUR        (3600)
#define SECONDS_IN_A_MINUTE      (60)
static const uint32_t secondsPerMonth[12] =
{
    31*SECONDS_IN_A_DAY,
    28*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    30*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    30*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    30*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    30*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
};

#define INVALID_SNTP_STARTUP_DELAY (0xFFFF)

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
extern wiced_timed_event_t sync_ntp_time_event;// defined in sntp.c

extern uint32_t ntp_succeeded_since_boot;// defined in sntp.c
extern uint32_t save_time_to_sflash;// defined in sntp.c
extern IOT_Device_Config_t device_config;	// Defined in device/config.h and saved in DCT

wiced_utc_time_t sunrise;
wiced_utc_time_t sunset;

static uint32_t sntp_random_delay_ms = INVALID_SNTP_STARTUP_DELAY;
static wiced_time_t when_sntp_timer_started = 0;


/******************************************************
 *               Function Definitions
 ******************************************************/
//#include "sntp.h" duplicated see above

/**
 * The new WICED 4.0 version of sntp_start_auto_time_sync() frequently causes the watchdog to byte because of a delay of up to 76.5 sec.
 * The two functions start_stopped_auto_time_sync_if_random_delay_is_finished() and start_random_delay_timer_for_sntp() replace this.
 *
 * For start_random_delay_timer_for_sntp() below, set the timer start time and get a random delay.
 * The calculation here assumes that there are less than 255 Auto controllers per NTP server and that processing each NTP request
 * by the server will take less than 10 ms.
 *
 * written by Eric Thelin 29 November 2016
 */
void start_random_delay_timer_for_sntp()
{
	uint8_t random_byte = 0;

	wiced_time_get_time(  &when_sntp_timer_started );

    /* prevent thundering herd scenarios by randomizing per RFC4330 */
    wiced_crypto_get_random(&random_byte, 1);
    sntp_random_delay_ms = 10 * (unsigned int)random_byte;

}

/**
 * Do an asychronous SNTP update and schedule additional updates every TIME_SYNC_PERIOD,
 * but do nothing unless both when_sntp_timer_started and sntp_random_delay_ms are valid.
 * This constraint helps prevent starting SNTP if it was already started or if the timer was never started,
 * but no direct check is made to ensure that the timed event has not already been scheduled.
 *
 * written by Eric Thelin 29 November 2016
 */
wiced_result_t start_stopped_auto_time_sync_if_random_delay_is_finished()
{
	wiced_time_t now;
	wiced_result_t result;

	if ( ( when_sntp_timer_started != 0 ) || ( sntp_random_delay_ms != INVALID_SNTP_STARTUP_DELAY ) ) {// Zero means the random delay timer is not running.
		wiced_time_get_time( &now );
		if ( is_later( now, when_sntp_timer_started + sntp_random_delay_ms ) ) {
		    when_sntp_timer_started = 0;
		    sntp_random_delay_ms = INVALID_SNTP_STARTUP_DELAY;

		    /* Synchronize time with NTP server and schedule for re-sync every one day */
		    result = wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, sync_ntp_time, NULL );
		    wiced_rtos_register_timed_event( &sync_ntp_time_event, WICED_NETWORKING_WORKER_THREAD, sync_ntp_time, TIME_SYNC_PERIOD, 0 );

		    return result;
		}
	}
	return WICED_SUCCESS;// Assume that if the timer isn't running anymore, that auto_time_sync has already started.
}
/**
 * This function stops and restarts NTP auto time sync. The function assumes that auto tyme sync is
 * already turned on. If the attempt to stop auto time sync fails, then the error code is returned
 * without attempting to restart.
 *
 * NOTE: This function is defined here in ck_time.c to use the locally defined TIME_SYNC_PERIOD
 * rather than defining the function in sntp.c
 *
 * @return error code returned by call to sntp_start_auto_time_sync()
 *
 * written by Eric Thelin 18 June 2015
 **/
wiced_result_t sntp_restart_auto_time_sync()
{
    int error_code;

    error_code = sntp_stop_auto_time_sync();

    if (error_code != WICED_SUCCESS) {
        return error_code;
    }
    return sntp_start_auto_time_sync( TIME_SYNC_PERIOD );
}

/**
 * Put the character expressions in the iso8601 struct which do not have NULL terminators into a NULL
 * terminated string which is passed in.
 * @param str_out stores the output string if possible
 * @param max_length is the maximum number of characters that can be written to str_out
 * @param time is the iso 8601 time structure
 * @return NULL if the character expressions are too big to fit within max_length. Else return str_out.
 *
 * written by Eric Thelin 18 June 2015
 */
char* sprintf_iso8601_time(char* str_out, uint32_t max_length, wiced_iso8601_time_t *time)
{
    // If output buffer is too small ...
    if ( max_length <= sizeof( wiced_iso8601_time_t ) ) {
        return NULL;
    }

    // Treat the combination of character expressions as a single block of characters.
    char *str_time = (char *) time;

    // Copy time expressions into str_out
    int i = 0;
    while (i < sizeof(wiced_iso8601_time_t)) {
        str_out[i] = str_time[i];
        i++;
    }
    str_out[i] = '\0'; // Add NULL to terminate the string.

    return str_out;
}

/**
 * Adjust UTC time retrieved from the system clock to current local time.
 *
 * NOTE: the local time offset from UTC is currently set by a remote device. This remote device
 * controls the transition between daylight savings and standard time. If for some reason
 * the remote device has never successfully updated this time, The factory default of UTC time
 * will be used.(see factory defaults in sflash/sflash.c) If the offset has ever previously been
 * set, then that value will be used even if it is out of date.
 *
 * @return a time at noon in the year 1970 if the call to the WICED system clock fails.
 *         Otherwise return the system clock time adjusted to local time in seconds.
 *
 * written by Eric Thelin 21 June 2015
 */
wiced_utc_time_t current_local_time()
{

    wiced_utc_time_t utc_seconds_since_1969;
    wiced_result_t error_code = wiced_time_get_utc_time( &utc_seconds_since_1969 );

    if ( error_code != WICED_SUCCESS ) {
        utc_seconds_since_1969 = 1.5 * DAYS; // 12 noon on Friday 2 Jan 1970
    }
    return utc_seconds_since_1969 + device_config.local_seconds_offset_from_utc;
}

/**
 * Extract the numeric day of the week from a standard seconds since 1969 type of datetime value.
 * Sunday is 1..Saturday is 7
 *
 * @param seconds_since_1969 date in.
 * @return numeric day of week
 *
 * Written by Eric Thelin 22 June 2015
 */
uint16_t day_of_week(wiced_utc_time_t seconds_since_1969)
{
    uint32_t days = seconds_since_1969 / (24 * 3600);
    days = days + 4;
    uint16_t result = (uint16_t) ( days % 7 ) + 1;
    return result;
}

/**
 * @param year
 * @return TRUE if year is a leap year, otherwise FALSE
 *
 * In the Gregorian calendar 3 criteria must be taken into account to identify leap years:
 * The year is evenly divisible by 4;
 * but if the year can be evenly divided by 100, it is NOT a leap year;
 * unless the year is also evenly divisible by 400.
 *
 * written by Eric Thelin 7 July 2015
 */
uint8_t is_leap( uint32_t year ) {
    if (( year % 400 ) == 0 ) {
        return true;
    }
    if ((( year % 4 ) == 0 ) && (( year % 100) != 0)) {
        return true;
    }
    return false;
}

/**
 * Generate an estimate of sunrise and sunset on a specific day.
 *
 * @param any_utc_time_on_day - any UTC time within 12 hours of solar noon calculated from the latitude of the fixture.
 * @param sunrise - returned in UTC
 * @param sunset - returned in UTC
 *
 * written  by Eric Thelin 15 July 2015
 */
void sunrise_sunset( wiced_utc_time_t any_utc_time_on_day, wiced_utc_time_t* sunrise, wiced_utc_time_t* sunset )
{

    // seconds in 1 full day revolution / degrees in a full revolution * longitude converts to the time offset
    int32_t solar_time_offset_from_utc = ( device_config.longitude ) * 240;  // 240 = 24 * 3600 / 360

//    wiced_utc_time_t now = local_time();
    // Calculate days using true solar time at that longitude.
    uint32_t days_since_1969 = ( any_utc_time_on_day + solar_time_offset_from_utc ) / ( 24 * 3600 );

    // find days since end of last year
    uint32_t days_since_beginning = days_since_1969;
    uint32_t year = 1970;
    while ( (days_since_beginning > 365) || ((! is_leap(year)) && (days_since_beginning > 364))) {
        if ( is_leap( year )) {
            days_since_beginning -= 366;
        } else {
            days_since_beginning -= 365;
        }
        year++;
    }

    //H = | (1/15)*arccos[-tan(L)*tan(23.44*sin(360(D+284)/365))] |. equation from http://www.had2know.com/society/sunrise-sunset-time-calculator-formula.html
    float cos_hr =  -tan( device_config.lattitude * PI / 180 ) * tan( (23.44 * PI/180.0) * sin(2.0 * PI * ( days_since_beginning + 284.0)/365.0));

    // Check for abs(cos_hr) > 1
    if ( cos_hr > 1 ) { // the sun never rises
        print_status("The sun never rises.\r\n");
        *sunrise = 0xFFFFFFFF;
        *sunset = 0;
    }
    else if ( cos_hr < -1 ) { // the sun never sets
        print_status("The sun never sets.\r\n");
        *sunrise = 0;
        *sunset = 0xFFFFFFFF;
    }
    else {
        uint32_t seconds_from_noon =  abs((12.0 * 3600.0/PI) * acos(cos_hr));
        // Change back to using UTC
        wiced_utc_time_t noon = (((days_since_1969 * 24) + 12) * 3600) - solar_time_offset_from_utc;
        *sunrise = noon - seconds_from_noon;
        *sunset = noon + seconds_from_noon;
//        print_status("Longitude/latitude: hour angle %f/%f: %f\r\n", dct_config->header->longitude, dct_config->header->latitude, acos(cos_hr));

    }
}


/**
 * Retrieves the iso8601 character representation of the current time. For use in debugging.
 *
 * This function is copied and altered from wiced_time_get_iso8601_time in WICED/internal/time.c
 * @param iso8601_time - returned char representation of the current time
 * @return is always WICED_SUCCESS
 */
wiced_result_t wiced_local_time_get_iso8601_time(wiced_iso8601_time_t* iso8601_time)
{
    wiced_utc_time_t   local_time;
    uint32_t            a;
    uint16_t            year;
    uint8_t             number_of_leap_years;
    uint8_t             month;
    uint8_t             day;
    uint8_t             hour;
    uint8_t             minute;
    uint32_t            second;
    uint32_t            sub_second = 0;
    wiced_bool_t        is_a_leap_year;

    local_time = current_local_time();
    second = local_time;

    /* Calculate year */
    year = (uint16_t)( 1970 + second / SECONDS_IN_365_DAY_YEAR );
    number_of_leap_years = (uint8_t) (( year - 1968 - 1 ) / 4 );
    second -= (uint32_t)( (uint32_t)( year - 1970 ) * SECONDS_IN_365_DAY_YEAR );
    if ( second > ( number_of_leap_years * SECONDS_IN_A_DAY ) )
    {
        second -= (uint32_t) ( ( number_of_leap_years * SECONDS_IN_A_DAY ) );
    }
    else
    {
        second -= (uint32_t) ( ( number_of_leap_years * SECONDS_IN_A_DAY ) + SECONDS_IN_365_DAY_YEAR );
        --year;
    }

    /* Remember if the current year is a leap year */
    is_a_leap_year = ( ( year - 1968 ) % 4 == 0 ) ? WICED_TRUE : WICED_FALSE;

    /* Calculate month */
    month = 1;

    for ( a = 0; a < 12; ++a )
    {
        uint32_t seconds_per_month = secondsPerMonth[a];
        /* Compensate for leap year */
        if ( ( a == 1 ) && is_a_leap_year )
        {
            seconds_per_month += SECONDS_IN_A_DAY;
        }
        if ( second > seconds_per_month )
        {
            second -= seconds_per_month;
            month++;
        }
        else
        {
            break;
        }
    }

    /* Calculate day */
    day    = (uint8_t) ( second / SECONDS_IN_A_DAY );
    second -= (uint32_t) ( day * SECONDS_IN_A_DAY );
    ++day;

    /* Calculate hour */
    hour   = (uint8_t) ( second / SECONDS_IN_A_HOUR );
    second -= (uint32_t)  ( hour * SECONDS_IN_A_HOUR );

    /* Calculate minute */
    minute = (uint8_t) ( second / SECONDS_IN_A_MINUTE );
    second -= (uint32_t) ( minute * SECONDS_IN_A_MINUTE );

    /* Write iso8601 time (Note terminating nulls get overwritten intentionally) */
    unsigned_to_decimal_string( year,             iso8601_time->year,       4, 4 );
    unsigned_to_decimal_string( month,            iso8601_time->month,      2, 2 );
    unsigned_to_decimal_string( day,              iso8601_time->day,        2, 2 );
    unsigned_to_decimal_string( hour,             iso8601_time->hour,       2, 2 );
    unsigned_to_decimal_string( minute,           iso8601_time->minute,     2, 2 );
    unsigned_to_decimal_string( (uint8_t)second,  iso8601_time->second,     2, 2 );
    unsigned_to_decimal_string( sub_second,       iso8601_time->sub_second, 6, 6 );

    iso8601_time->T          = 'T';
    iso8601_time->Z          = ' ';
    iso8601_time->colon1     = ':';
    iso8601_time->colon2     = ':';
    iso8601_time->dash1      = '-';
    iso8601_time->dash2      = '-';
    iso8601_time->decimal    = '.';

    return WICED_SUCCESS;
}


wiced_result_t iso8601_time_of(wiced_utc_time_t utc_time, wiced_iso8601_time_t* iso8601_time)
{
//    wiced_utc_time_t   local_time;
    uint32_t            a;
    uint16_t            year;
    uint8_t             number_of_leap_years;
    uint8_t             month;
    uint8_t             day;
    uint8_t             hour;
    uint8_t             minute;
    uint32_t            second;
    uint32_t            sub_second = 0;
    wiced_bool_t        is_a_leap_year;

//    local_time = current_local_time();
    second = utc_time;

    /* Calculate year */
    year = (uint16_t)( 1970 + second / SECONDS_IN_365_DAY_YEAR );
    number_of_leap_years = (uint8_t) (( year - 1968 - 1 ) / 4 );
    second -= (uint32_t)( (uint32_t)( year - 1970 ) * SECONDS_IN_365_DAY_YEAR );
    if ( second > ( number_of_leap_years * SECONDS_IN_A_DAY ) )
    {
        second -= (uint32_t) ( ( number_of_leap_years * SECONDS_IN_A_DAY ) );
    }
    else
    {
        second -= (uint32_t) ( ( number_of_leap_years * SECONDS_IN_A_DAY ) + SECONDS_IN_365_DAY_YEAR );
        --year;
    }

    /* Remember if the current year is a leap year */
    is_a_leap_year = ( ( year - 1968 ) % 4 == 0 ) ? WICED_TRUE : WICED_FALSE;

    /* Calculate month */
    month = 1;

    for ( a = 0; a < 12; ++a )
    {
        uint32_t seconds_per_month = secondsPerMonth[a];
        /* Compensate for leap year */
        if ( ( a == 1 ) && is_a_leap_year )
        {
            seconds_per_month += SECONDS_IN_A_DAY;
        }
        if ( second > seconds_per_month )
        {
            second -= seconds_per_month;
            month++;
        }
        else
        {
            break;
        }
    }

    /* Calculate day */
    day    = (uint8_t) ( second / SECONDS_IN_A_DAY );
    second -= (uint32_t) ( day * SECONDS_IN_A_DAY );
    ++day;

    /* Calculate hour */
    hour   = (uint8_t) ( second / SECONDS_IN_A_HOUR );
    second -= (uint32_t)  ( hour * SECONDS_IN_A_HOUR );

    /* Calculate minute */
    minute = (uint8_t) ( second / SECONDS_IN_A_MINUTE );
    second -= (uint32_t) ( minute * SECONDS_IN_A_MINUTE );

    /* Write iso8601 time (Note terminating nulls get overwritten intentionally) */
    unsigned_to_decimal_string( year,             iso8601_time->year,       4, 4 );
    unsigned_to_decimal_string( month,            iso8601_time->month,      2, 2 );
    unsigned_to_decimal_string( day,              iso8601_time->day,        2, 2 );
    unsigned_to_decimal_string( hour,             iso8601_time->hour,       2, 2 );
    unsigned_to_decimal_string( minute,           iso8601_time->minute,     2, 2 );
    unsigned_to_decimal_string( (uint8_t)second,  iso8601_time->second,     2, 2 );
    unsigned_to_decimal_string( sub_second,       iso8601_time->sub_second, 6, 6 );

    iso8601_time->T          = 'T';
    iso8601_time->Z          = ' ';
    iso8601_time->colon1     = ':';
    iso8601_time->colon2     = ':';
    iso8601_time->dash1      = '-';
    iso8601_time->dash2      = '-';
    iso8601_time->decimal    = '.';

    return WICED_SUCCESS;
}

/**
 * Return the difference between two times from the system tick timer which rolls over every 47 days.
 * The times cannot be more than 47 days apart, because only one roll over may have occurred.
 *
 * written by Eric Thelin 21 July 2016
 */
wiced_time_t time_difference( wiced_time_t latest, wiced_time_t earliest )
{
	if ( latest < earliest ) {// tick timer rolled over.
		return latest + ( 0xFFFFFFFF - earliest );
	}
	else {
		return latest - earliest;
	}
}

/**
 * Taking into consideration the possibility of a rollover in the tick time every 47 days,
 * return TRUE if the first time parameter is later than the last.
 * Otherwise return FALSE.
 * NOTE: Assume that the two times are no more than a quarter of the possible range of values apart.
 *
 * written by Eric Thelin 29 June 2016
 */
uint16_t is_later(  wiced_time_t time1, wiced_time_t time2 )
{
	const wiced_time_t range_top_quarter = 0xC0000000;
	const wiced_time_t range_mid_point =   0x80000000;
	const wiced_time_t range_low_quarter = 0x40000000;

	if ( time1 >= range_mid_point ) {
		if ( time2 >= range_low_quarter ) {
			return ( time1 > time2 );
		}
		else {// Assume that time2 has rolled over and is really the bigger number
			return false;
		}
	}
	else {// time1 is in the low half of the range

		if ( time2 <= range_top_quarter ) {
			return ( time1 > time2 );
		}
		else {// Assume time1 has rolled over and is really the bigger number
			return true;
		}
	}
}

/**
 * Taking into consideration the possibility of a rollover in the tick time every 47 days,
 * return TRUE if the current time is later than start + wait.
 * Otherwise return FALSE.
 * NOTE: Assume that the two times are no more than a quarter of the possible range of values apart.
 *
 * written by Eric Thelin 30 June 2016
 */
uint16_t timer_timeout( wiced_time_t current_time, wiced_time_t start, uint32_t wait )
//uint16_t is_later(  wiced_time_t time1, wiced_time_t time2 )
{
	return is_later( current_time, start + wait );
}

/**
 * Update fixture configuration from the main thread after a successful NTP update and return WICED_TRUE.
 * Otherwise return WICED_FALSE.
 *
 * written by Eric Thelin 3 July 2016
 **/
uint16_t save_ntp_information_to_config_when_needed()
{
	if ( save_time_to_sflash ) {
		//dct_config->header->last_ntp_updated_time = last_ntp_updated_time;
		save_time_to_sflash = WICED_FALSE;

        //save_config();
        return WICED_TRUE;
	}
	return WICED_FALSE;
}
/**
 * Return TRUE(1) if the system clock has ever been updated by NTP.
 * Otherwise return FALSE.
 *
 * written by Eric Thelin 5 July 2016
 **/
uint16_t ntp_succeeded_at_least_once()
{
	return (uint16_t) ntp_succeeded_since_boot;
}
