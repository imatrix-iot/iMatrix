/*
 * $Copyright 2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file
 *
 *  wrong_group.c
 *
 *  Created on: September, 2017
 *      Author: greg.phillips
 *
 */

int imx_wrong_group( CoAP_msg_detail_t *cd );
uint16_t imx_in_my_groups( uint16_t group );
wiced_result_t imx_get_group_from_query_str( char* query_str, uint16_t *group );
wiced_result_t imx_get_group_from_json( char *data, uint16_t data_length, uint16_t *group );
uint16_t imx_skip_whitespace( int direction, char *data, uint16_t start, uint16_t data_length );
uint16_t imx_end_of_string( char *data, uint16_t start, uint16_t data_length );
uint16_t imx_end_of_object( char *data, uint16_t start, uint16_t data_length );
