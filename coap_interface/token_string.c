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
 * token_string.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <ctype.h>
#include <stdint.h>

#include "wiced.h"
#include "../cli/interface.h"
#include "../CoAP/coap.h"
#include "token_string.h"



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

/******************************************************
 *               Function Definitions
 ******************************************************/
/*
 *  Created on: Apr 19, 2015
 *      Author: eric thelin
 */

/**
 * This boolean function returns true if the initial part of each of the strings
 * up to "length" or the NULL character is identical.
 * 
 * @param str1 
 * @param str2
 * @param length - the number of characters that must be identical to return TRUE
 * @returns FALSE if the first part the strings are different
 * 
 * written by Eric Thelin 19 April 2015
 */
int left_str_equals( char* str1, char* str2, int length ) 
{
   int a;
   for ( a = 0; a < length; a++ ) {
      if ( str1[a] != str2[a] ) {
         return false;
      }
      else if ( str1[a] == (char)0 ) {// when both chars are equal and NULL
         return true;
      }
   }
   return true;
}


/**
 * This function identifies the end of the first token in a string. 
 * 
 * @param terminating_chars - a string containing any character other than the
 * the NULL character that indicates the end of a token. 
 * @param str - the string to search.
 * @param max_length - stop searching after parsing this many characters and
 * return the index of the first char in "str" that matches any of the chars in "terminating_chars".
 *        This is also the length up to but not including that character in  "terminating_chars".
          If nothing matched, return "max_length"
 * 
 * written by Eric Thelin 19 April 2015
 */
int get_length_before( char terminating_chars[], char str[], int max_length )
{
   int n, a;
   for ( n = 0; n < max_length; n++ ) {
       
      if ( str[n] == (char)0 ) {
         return n;
      }

      for ( a = 0; a < strlen( terminating_chars ); a++ ) {
         if ( str[n] == terminating_chars[a] ) {
            return n;
         }
      }
   }
   return max_length; // no terminating char was found before reaching max_length
}

/**
 *  This function checks every letter in the string "number" up to the "length"
 *  digit of the string. No negative sign or decimal point is allowed.
 * 
 * @param number - string that should be entirely numeric
 * @param length - the number of digits to check
 * @returns FALSE if any non digit is found, otherwise, returns TRUE
 * 
 * written by Eric Thelin 19 April 2015
 */
uint16_t left_str_is_uint( char  *number, int length )
{
    int i = 0;

    if( length <= 0 )
        return false;

    for( i = 0; i < length; i++ ) {
        if ( number[i] == (char)0 ) {//early end of string OK if there are any digits
           if ( i > 0 ) {//OK
              return true;
           }
           else return false;
	    }
        if( !isdigit( (uint8_t) number[i] ) ) {
                return false;   // Non digit in string
        }
    }
    return true;
}

uint16_t left_str_is_int( char *number, int length )
{
    if( length <= 0 )
        return false;

    if( number[0] == '-' ) {
        return left_str_is_uint( &( number[1] ), length - 1 );
    }

    return left_str_is_uint( number, length );
}

/** 
 * Convert Left part of string to an unsigned 16 bit integer using length characters.
 * 
 * @param str 
 * @param length - number of characters to convert
 * @returns 0 if unsuccessful otherwise returns the 16 bit integer
 * 
 * written by Eric Thelin 20 April 2015
 */
uint16_t make_str_uint( char* str, int length ) {

   uint16_t result = 0;
   int n;
   int digit = 0;
   int base = 1; 

   if ( ( length < 1 ) || ( length > 5 ) ) {// invalid length: max 5 digits in 65535
      imx_printf("Length of string being converted to an integer is invalid\n\r");
      return 0;
   }
   
   for ( n = length - 1; n >= 0; n-- ) {
	   
	   // convert digit into a number
	   digit = str[n] - '0';
	   
	   if ( ( digit < 0 ) || ( 9 < digit ) ) {
		   imx_printf("Attempted to convert non-numeric ASCII character to integer.\n\r");
		   return 0;
	   }
	   
	   if ( ( length - n ) == 5 ) {//check for max size of int
		   
	      if ( ( ( digit == 6 ) && ( result > 5535 ) ) || ( digit > 6 ) ) {
			  imx_printf("Number being converted to unsigned 16 bit integer is too large.\n\r");
			  return 0;
		  }
	   }
				   
	   result = result + digit * base; // multiply by base and add to rest of number
	   base = base * 10;
   }   
   
   return result;
}

/**
 * Convert Left part of string to a 32 bit unsigned integer using length characters.
 *
 * @param str
 * @param length - number of characters to convert
 * @returns 0 if unsuccessful otherwise returns the 32 bit integer
 *
 * written by Eric Thelin 20 April 2015
 */
uint32_t make_str_uint32( char* str, int length ) {

    uint32_t result = 0;
    int n;
    int digit = 0;
    int base = 1;

    if ( ( length < 1 ) || ( length > 10 ) ) {// invalid length: max 10 digits in 4294967295
        imx_printf("Length of string being converted to an integer is invalid\n\r");
        return 0;
    }

    for ( n = length - 1; n >= 0; n-- ) {

        // convert digit into a number
        digit = str[n] - '0';

        if ( ( digit < 0 ) || ( 9 < digit ) ) {
            imx_printf("Attempted to convert non-numeric ASCII character to integer.\n\r");
            return 0;
        }

        if ( ( length - n ) == 10 ) {//check for max size of int

            if ( ( ( digit == 4 ) && ( result > 294967295 ) ) || ( digit > 4 ) ) {
                imx_printf("Number being converted to 32 bit integer is too large.\n\r");
                return 0;
            }
        }

        result = result + digit * base; // multiply by base and add to rest of number
        base = base * 10;
    }

    return result;
}

/**
 * Convert Left part of string to a 32 bit integer using length characters.
 *
 * @param str
 * @param length - number of characters to convert
 * @returns 0 if unsuccessful otherwise returns the 32 bit integer
 *
 * written by Eric Thelin 20 April 2015
 */
int32_t make_str_int32( char* str, int length ) {

    uint32_t unsigned_number;

    // if invalid length: max 10 digits in 2147483647 + sign
    if ( ( length < 1 ) || ( ( str[0] != '-' ) && ( length > 10 ) ) || ( length > 11 ) ) {
        imx_printf("Length of string being converted to an integer is invalid\n\r");
        return 0;
    }

    if (str[0] == '-') {
        unsigned_number = make_str_uint32( &( str[1] ), length - 1 );
    }
    else {
        unsigned_number = make_str_uint32( str, length );
    }

    if ( unsigned_number > 2147483647 ) {
        return 0;
    }

    if (str[0] == '-') {
        return -1 * (int32_t)unsigned_number;
    }
    return (int32_t)unsigned_number;
}
