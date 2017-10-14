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
 *  Created on: September, 2016
 *      Author: greg.phillips
 */

#ifndef CERTIFICATES_H_
#define CERTIFICATES_H_

/** @file
 *
 *	Defines for Default Certificates for 802.1X and DTLS connections
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
const uint8_t *WIFI_USER_CERTIFICATE_STRING  = ( uint8_t *)\
		"-----BEGIN CERTIFICATE-----\r\n"\
		"MIID4jCCAsqgAwIBAgIBAjANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCVVMx\r\n"\
		"DzANBgNVBAgTBk5ldmFkYTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFzAVBgNVBAoT\r\n"\
		"DlNpZXJyYSBUZWxlY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJyYXRl\r\n"\
		"bGVjb20ubmV0MRcwFQYDVQQDEw5TaWVycmEgVGVsZWNvbTAgFw0xNjExMjMxNTI0\r\n"\
		"NDNaGA8yMDY2MTExMTE1MjQ0M1owgYQxCzAJBgNVBAYTAlVTMQ8wDQYDVQQIEwZO\r\n"\
		"ZXZhZGExFzAVBgNVBAoTDlNpZXJyYSBUZWxlY29tMSEwHwYDVQQDFBhkZXZpY2VA\r\n"\
		"c2llcnJhdGVsZWNvbS5uZXQxKDAmBgkqhkiG9w0BCQEWGXN1cHBvcnRAc2llcnJh\r\n"\
		"dGVsZWNvbS5uZXQwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDTFyQp\r\n"\
		"NXUMzeO4gvJH//49WLxJx2TAht5T6aEXgO4Dn37+WlEX9vstgOMxhaVawG9a45Tx\r\n"\
		"IAfnRUhABPy2SViZrgAVwf4S3ABkZiqyOB3WAouDdbstkuOJsa8q2bLBCAnICoGl\r\n"\
		"By7Pq6O1jKntEoS1gDpRXWymYHPl12BKhKAx5Jo2rDCfxlaBP6oM+UG9mXSWg6IF\r\n"\
		"3D5Ifzu5K6KcWf7w1DsFvPyeceIQBQY7OS9nVY5libdrDM1VQcmDsPuPZwwQWHHj\r\n"\
		"/hkk9kwLbcgYpRJ1AETFt+tGYBXGrmoUQLIiK2u5wAcNx11YkiPMTbwNfb3WWLrr\r\n"\
		"yOUOCSva1JcSTCwjAgMBAAGjTzBNMBMGA1UdJQQMMAoGCCsGAQUFBwMCMDYGA1Ud\r\n"\
		"HwQvMC0wK6ApoCeGJWh0dHA6Ly93d3cuZXhhbXBsZS5jb20vZXhhbXBsZV9jYS5j\r\n"\
		"cmwwDQYJKoZIhvcNAQELBQADggEBAA5qFfD/ZGe/erJWA4yd0ZgTxI1k+17YQ5fo\r\n"\
		"PAF3nJZIiz5k6Hg639sFddtEBEH/7aC5KGCltLitcolV+dpV4APunU67xLwo1DKg\r\n"\
		"5JKvld5ZXAyunsY0en5+0MT0xiElZjbsQITsQCOk/X8AkYlTqMaYHON1fdzVqa3d\r\n"\
		"tQ/clJrsfAPYNDQTmn8vgTZw8xE+OBA8sJkxmNQaK5kj1OQwj/P5/JwFJLIX64Dv\r\n"\
		"Es/ggQ3g7NQzR1PPVM2EsEla562yHJlvYaCZyyYfpq4QYoKkJKZmfJCAtBw+lP62\r\n"\
		"BZtXSfxhfbEbhqEQoPUIWdfXI8uVf2+ki92bmARGx4jQlZhUUZ8=\r\n"\
		"\0"\
		"\0";

const uint8_t *WIFI_USER_PRIVATE_KEY_STRING = ( uint8_t *) \
		"-----BEGIN RSA PRIVATE KEY-----\r\n"\
		"MIIEpAIBAAKCAQEA0xckKTV1DM3juILyR//+PVi8ScdkwIbeU+mhF4DuA59+/lpR\r\n"\
		"F/b7LYDjMYWlWsBvWuOU8SAH50VIQAT8tklYma4AFcH+EtwAZGYqsjgd1gKLg3W7\r\n"\
		"LZLjibGvKtmywQgJyAqBpQcuz6ujtYyp7RKEtYA6UV1spmBz5ddgSoSgMeSaNqww\r\n"\
		"n8ZWgT+qDPlBvZl0loOiBdw+SH87uSuinFn+8NQ7Bbz8nnHiEAUGOzkvZ1WOZYm3\r\n"\
		"awzNVUHJg7D7j2cMEFhx4/4ZJPZMC23IGKUSdQBExbfrRmAVxq5qFECyIitrucAH\r\n"\
		"DcddWJIjzE28DX291li668jlDgkr2tSXEkwsIwIDAQABAoIBAQDNG+9HG2pgxq4+\r\n"\
		"2v4Qd+3SLibGibicWnAtmRWugNjFLHV5MMkbQCFQVaxssi+5vsJ0zi4rHbSQRAGw\r\n"\
		"Myha92nf6fKrMZRvPtYiuCNEiMJ08IQfeIYoAlKVuNE8EcxfRDA87/iJIjGaGG06\r\n"\
		"OzPh59yLf80qwlyQTahP7LbjsXr2/iXqJ/xuckFIiOxVIw9jr4X/UG8sghB7eKSQ\r\n"\
		"OcPt0ehsCPnHLErl284M7UIrPvg1B8zpVkO45PBAmmrXLfpMdj88n/UYxwEGLImq\r\n"\
		"T4MMjIFC9MBN/6ctQuuuaMiBAHdB0AVWsA4GtY31+0V94lEwomfhPWG367FShuAg\r\n"\
		"XHtsOMoBAoGBAO8aj7bE1fKgi64xPPoQZ3XiHEl7WM4dgWaId45cIDWgImnkHioo\r\n"\
		"SqxSPfBWCzBu2MQ2cQYa8+Xt/+uz3WEsq26wcclh3uBFm8ok7CTfjzrVhxHHrZYw\r\n"\
		"SIKNfZRED+MMkflxZfR5VG7MjnHHgBoug49fJA0WTypPXx7X+8noBVEVAoGBAOIB\r\n"\
		"z+gkE/YJkhEXnw9NvxPfBeeayCVOt7jaY/tkalL4wucxa0rMciD+ofnAo/KrTS8Z\r\n"\
		"matVmBhk5hkLrK670OwoM6Tve/+zsGCeXW9lRqvSdM/tD0Oq5EZajkRHRzlVomw1\r\n"\
		"nNHH5E/iKxQSiWQLy4Oc4IjT4LxLsEeDwtIEwqZXAoGAZIuv2XSmX1QByOttr95M\r\n"\
		"E3dU29Whv3MK4joKxHObG9HOBudlg+bTl+kxoKoeJGzIfKJEHwQgghT2JIDLXDyz\r\n"\
		"jmVmRAADAB5lZ7jlEJBuI5h9Jw3nwhtlsnibUUjZwWmczN9Rt196EOm2EMSK6vea\r\n"\
		"XSTt/Q7flXQxFMBQYlROQA0CgYEAgGSFJxzv1hNDzr/7lacWSQSnBxcQYtlgV/yB\r\n"\
		"VjyKho5ZsxuJF1oj8AiYHhlJSj/0vjAt837Rw/H2If5W4IFERBTmmemPKQl8xrbQ\r\n"\
		"38rKJiyOPd0vkcKQokMplTJlDPZQKmceqrKDSrpyvJl3QcXNnPxPNowfnPjmsQny\r\n"\
		"cHtR1UsCgYBFsndH5OwsSMcFCCt6CE+wnDqLu1eEC50lp55d77V7qU9ghAh8HyQk\r\n"\
		"C+MRl4nhWHwWJw7WMPSObAq7ym14vxq8GkgynsyyqbRPytZN1cR5UZCem6mHgg/2\r\n"\
		"QHufbw1pdkU34N5HoN90MiFEjRx/oaF61Gt2y6dxzik+NntXoJEmqA==\r\n"\
		"-----END RSA PRIVATE KEY-----\r\n"\
		"\0"\
		"\0";

const uint8_t *WIFI_ROOT_CERTIFICATE_STRING = ( uint8_t *) \
		"-----BEGIN CERTIFICATE-----\r\n"\
		"MIIE2jCCA8KgAwIBAgIJAPKleacRtu0HMA0GCSqGSIb3DQEBBQUAMIGQMQswCQYD\r\n"\
		"VQQGEwJVUzEPMA0GA1UECBMGTmV2YWRhMRQwEgYDVQQHEwtaZXBoeXIgQ292ZTEX\r\n"\
		"MBUGA1UEChMOU2llcnJhIFRlbGVjb20xKDAmBgkqhkiG9w0BCQEWGXN1cHBvcnRA\r\n"\
		"c2llcnJhdGVsZWNvbS5uZXQxFzAVBgNVBAMTDlNpZXJyYSBUZWxlY29tMCAXDTE2\r\n"\
		"MTEyMzE1MjQ0M1oYDzIwNjYxMTExMTUyNDQzWjCBkDELMAkGA1UEBhMCVVMxDzAN\r\n"\
		"BgNVBAgTBk5ldmFkYTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFzAVBgNVBAoTDlNp\r\n"\
		"ZXJyYSBUZWxlY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJyYXRlbGVj\r\n"\
		"b20ubmV0MRcwFQYDVQQDEw5TaWVycmEgVGVsZWNvbTCCASIwDQYJKoZIhvcNAQEB\r\n"\
		"BQADggEPADCCAQoCggEBAMB4YOT3ONkPg7sPufEwEZJa7RZY2pVF89AP5t1bd1Ao\r\n"\
		"S3yrKnWeFxF2ypfvJEimHaxom54LVRAo5FQOyDM7SHSedBXL2GggZF21Cy9pSB6m\r\n"\
		"Hj7QHqjo6BTJLEkyJ30PEK544pABHt4sIdAqz7bAxLyarVv2RsnXCND1MWF3T9mL\r\n"\
		"G41yT629hjgENmphTCv7V5Y9AoWp4cEEQ7XfWisD/+LM6L3vdlMFx+lbNpGedsk8\r\n"\
		"ox5xwHxoFhGHq6anVWYkVxkbdcJ9cscLveO4tSOWABGmitInzIFmvYInxQGzVIIY\r\n"\
		"FkRgIw5xIV38VIKNt4FlTyAmh1HDIKnZvqcW+YJBlO0CAwEAAaOCATEwggEtMB0G\r\n"\
		"A1UdDgQWBBQSa6yi05ie+UhngVLg2VOKfcsBCTCBxQYDVR0jBIG9MIG6gBQSa6yi\r\n"\
		"05ie+UhngVLg2VOKfcsBCaGBlqSBkzCBkDELMAkGA1UEBhMCVVMxDzANBgNVBAgT\r\n"\
		"Bk5ldmFkYTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFzAVBgNVBAoTDlNpZXJyYSBU\r\n"\
		"ZWxlY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJyYXRlbGVjb20ubmV0\r\n"\
		"MRcwFQYDVQQDEw5TaWVycmEgVGVsZWNvbYIJAPKleacRtu0HMAwGA1UdEwQFMAMB\r\n"\
		"Af8wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL3d3dy5leGFtcGxlLmNvbS9leGFt\r\n"\
		"cGxlX2NhLmNybDANBgkqhkiG9w0BAQUFAAOCAQEADotBCHZSG2pcp/0OwOGwih0V\r\n"\
		"HXDq/sb/u+ejJLvU1hoFqN3M38s7Cf4m3mMNRd7w3HV4pS/MmWRjsYpA9pPLouiE\r\n"\
		"QFZKzLhCw8t08o7v1LpGCjOTCZTD4BjXDkQOGjei2cIiZv62uo16WgfSZmRix6xc\r\n"\
		"mWPcXhR9Ckrve19f2Mta2c3foDwC8tchK4VuPN5G0Yy64JSBuqfe0YptWpz/a1dK\r\n"\
		"4rjp0hpAO20ckfTs9CDFBIpDKgNjMeENzLDXw5L9/eoZAYqekRn7qWA4IHEV6/nN\r\n"\
		"TfkKAtyskSHSNDAJHYa04SQApIR9iKSlznuT3dCEshZAU0tg9jnUKnLOJTRHFA==\r\n"\
		"-----END CERTIFICATE-----\r\n"\
		"\0"\
		"\0";

const uint8_t *DEFAULT_DTLS_CERTIFICATE	= ( uint8_t *) \
	"-----BEGIN CERTIFICATE-----\r\n"\
	"-----END CERTIFICATE-----\r\n"\
	"\0"\
	"\0";

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

#endif /* CERTIFICATES_H_ */
