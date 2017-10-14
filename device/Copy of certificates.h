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

const uint8_t *WIFI_ROOT_CERTIFICATE_STRING = ( uint8_t *) \
		"-----BEGIN CERTIFICATE-----\r\n"\
		"MIIE3TCCA8WgAwIBAgIJANPrTP8jDqqIMA0GCSqGSIb3DQEBBQUAMIGQMQswCQYD\r\n"\
"VQQGEwJVUzEPMA0GA1UECBMGTmV2YWRhMRQwEgYDVQQHEwtaZXBoeXIgQ292ZTEX\r\n"\
"MBUGA1UEChMOU2llcnJhIFRlbGVjb20xKDAmBgkqhkiG9w0BCQEWGXN1cHBvcnRA\r\n"\
"c2llcnJhdGVsZWNvbS5uZXQxFzAVBgNVBAMTDlNpZXJyYSBUZWxlY29tMCAXDTE2\r\n"\
"MTIwODIxMjI0MVoYDzIwNjYxMTI2MjEyMjQxWjCBkDELMAkGA1UEBhMCVVMxDzAN\r\n"\
"BgNVBAgTBk5ldmFkYTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFzAVBgNVBAoTDlNp\r\n"\
"ZXJyYSBUZWxlY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJyYXRlbGVj\r\n"\
"b20ubmV0MRcwFQYDVQQDEw5TaWVycmEgVGVsZWNvbTCCASIwDQYJKoZIhvcNAQEB\r\n"\
"BQADggEPADCCAQoCggEBAJk6izeXIh8isd00eePsc5RTr5UJrxJbe+qxBBIr0du6\r\n"\
"XX6LDLOpt1UEmO2EWBI2vLH+i0plOt6gHkieeddIpa22FaUbtzLs8LkPLJRizasD\r\n"\
"VxQvz7TWUHWrZc/FcxrNhDC/TGs6nCGOnzASVWjkS86pptroeOlXUozI2L3KLoZN\r\n"\
"eA65g41Vzg7UXNSquCiDtnW9B2pybeI2egvFTlj+auNkYtVeZyl9hn7tPFQ9e21s\r\n"\
"W/wbi4BRKJYVHiVaWIGI1dgGITcQMbKuxFAyvj+WoFPmuK25pt9lMqy3gYS7knyC\r\n"\
"jyojzDGEF1JKp6JislHohfaUkiJmIlhKM9PlMtzSMbkCAwEAAaOCATQwggEwMB0G\r\n"\
"A1UdDgQWBBS4JM/sr12p+SHe4tEBBqn151vEXzCBxQYDVR0jBIG9MIG6gBS4JM/s\r\n"\
"r12p+SHe4tEBBqn151vEX6GBlqSBkzCBkDELMAkGA1UEBhMCVVMxDzANBgNVBAgT\r\n"\
"Bk5ldmFkYTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFzAVBgNVBAoTDlNpZXJyYSBU\r\n"\
"ZWxlY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJyYXRlbGVjb20ubmV0\r\n"\
"MRcwFQYDVQQDEw5TaWVycmEgVGVsZWNvbYIJANPrTP8jDqqIMA8GA1UdEwEB/wQF\r\n"\
"MAMBAf8wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL3d3dy5leGFtcGxlLmNvbS9l\r\n"\
"eGFtcGxlX2NhLmNybDANBgkqhkiG9w0BAQUFAAOCAQEAGo2b0UQ49oDzYeVEXWRr\r\n"\
"PHjF0i7A5aE6PTjuoNeiW3UPCfor/wckpLgd1ZyeSbBemjMqo+WurZaWBhhX7tm2\r\n"\
"1L0fvn4a015ErYJmSrzTB355QEBj08t4ngRKRbvjIGhNrECb9k7VGgkUA2PpcquY\r\n"\
"s0f2wCk4jmDUp7BXGb7KdL1NBjUwnuLQrlxA9O778HxNB3FwjqWrREVcy2C918JN\r\n"\
"qwILdQtfdbfGS5OjdOSE/NWh5O6EUou/mnSvlApiYDdhg1/kKHmnnUlPtQxFEuNr\r\n"\
"fXeSR+D8T26d4R60AdS5aXCeQL5t32XL41EZmE3Y2n6UDrh0AK945Pv0dT/h8dem\r\n"\
"CA==\r\n"\
		"-----END CERTIFICATE-----\r\n"\
		"\0"\
		"\0";

const uint8_t *WIFI_USER_CERTIFICATE_STRING  = ( uint8_t *)\
		"-----BEGIN CERTIFICATE-----\r\n"\
		"MIIEpAIBAAKCAQEA7RxUDUuLkq972aiWJ9PdSS8NHHt3taItGhP+6DYKLsoF1h5O\r\n"\
"wxZtWTFNDRYpVb6Afw0h2YNs9V+MlQhUPq9bKKjotTLCAUfavnppBjmUEzXABtRH\r\n"\
"7ZQYcQxYKtl8LElQ9ErP+TRK0fx6X2/SnC22oIQmxdT7yfykjYau9YSBFD6S1nCr\r\n"\
"6DmPJVRllixmIxI9gnHwfZavUsyW1lrRJdjd9eqgueSTA8WvpTwQ7H5Tbk0f0s/S\r\n"\
"YKiM1t7hWt9oYhC0VXmRRc4RQMenx6AtQlA1+53VUGGHShbUmHoZ2J/advTnZuad\r\n"\
"0H4d+b7ubbPm7DWFBlQaqFte2F1dK1IPdtyCqwIDAQABAoIBAQDjk8sAr7+Y3KFs\r\n"\
"uajVNT4xHd3htoehZ7UOUJ7a1fNUEUckyhYinQX7E5y2sMyfpabDSyFwGfqcUEco\r\n"\
"S8RAHiLKlcQ1FjCq9gJ3zBm9hcof3fkfFOSsnPYQmx6Mkg3im8P9lUikEZK2lTQH\r\n"\
"IdbtW9rrmat1OgUpSznNNLX8pF8Xw1ZNmIKB4lMWUqrnMybaSxAD7kWfX0W4YXkR\r\n"\
"wW29SlnjTcFZL5fF6c12gUHGsL+ZSuOUGrLl5lv0N8N0NWy2Axu6fPdwnndn7Hfv\r\n"\
"/FHvgDKcPsX6v/C6J+m7jz/bdv+nPSM6QQRm9oX/u9maqtjIxWu/bqcL3V5cVPCz\r\n"\
"tz/AsOJxAoGBAPdX26O09yecz1S3urDcs7OLa5x+1r7wHDjTTmk23wn1RmLXIkm+\r\n"\
"AY34xveU1BPrZVm0zyzd9FzMY8WldPXgwlM6GObysRi+bRUIODRzPXLNmkSdPw6n\r\n"\
"6/oq0autL/lqOEQRiKTR6claD8LA3n0b3gJrOp3BFBQ79wV64YxQcmZnAoGBAPVo\r\n"\
"yfz3RmOO+bwN6hO0hgG+ceMfxMcUBcVIYUOL3gCFBrZn2sZGa0YIPspGLc7ZsDNz\r\n"\
"+Or89uBZ+Uxqmcq3fADKytKohdmX3g2x9275DITUCFvLQsmpyqjLgQluNhEY3Jm7\r\n"\
"LUESM0kpf0mq6S26gJjU7hwayuiaSUvLZfDp1i8dAoGBAJEJ5fPDRELoQvOsoT7g\r\n"\
"zEd2c+3autZQOlxgHFS6JmPSfimreaHAV20G+rVngk0D9fR6gBoRNR/Ngpv9f+5G\r\n"\
"k+z13nBe09Zso853eW3YPao6QkIF61oOweN6hGSlM8KPtNrFZVkw9mRp+V/8dPrs\r\n"\
"Am1sr3yeIcYTu7Bs9CGxcsP1AoGAFyYpV4ljxGRqPkj5uZ4fve5fA9OtVjIXUULA\r\n"\
"mgSJjnb96Rrm0ik2WUbR1XbFP2vdaR0Zb+Eo0ITsNT8g/rRMKC747uIiriHbOmN9\r\n"\
"OaSYf7cYisAjrq4rhbyckW0qAHbd6Ep1vFuhJteZQDO1BzE+jCwZweDtHv/exh2L\r\n"\
"oiwMnikCgYBRs4fqmkKoQjv8+Nlg2Nqf/09nj4tIUIok6c6wnVjdw9koWTEmZD8e\r\n"\
"tr9TPw87b5mLfTunuEqpCFo8zpsUi6pxBKusOyVbWO2XS9bZKBFuCmC/9g+z5Yd6\r\n"\
"IA4N76UkT0TJx16MA9Doze9s6z9HNiYmDfi71DG1PdVfe2zljWzDbg==\r\n"\
		"-----END CERTIFICATE-----\r\n"\
		"\0"\
		"\0";

const uint8_t *WIFI_USER_PRIVATE_KEY_STRING = ( uint8_t *) \
		"-----BEGIN RSA PRIVATE KEY-----\r\n"\
		"MIIENjCCAx6gAwIBAgIBAjANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCVVMx\r\n"\
"DzANBgNVBAgTBk5ldmFkYTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFzAVBgNVBAoT\r\n"\
"DlNpZXJyYSBUZWxlY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJyYXRl\r\n"\
"bGVjb20ubmV0MRcwFQYDVQQDEw5TaWVycmEgVGVsZWNvbTAgFw0xNjEyMDgyMTIy\r\n"\
"NDFaGA8yMDY2MTEyNjIxMjI0MVowgYUxCzAJBgNVBAYTAlVTMQ8wDQYDVQQIEwZO\r\n"\
"ZXZhZGExFzAVBgNVBAoTDlNpZXJyYSBUZWxlY29tMSIwIAYDVQQDFBlkZXZpY2VA\r\n"\
"c2llcnJhLXRlbGVjb20uY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJy\r\n"\
"YXRlbGVjb20ubmV0MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA7RxU\r\n"\
"DUuLkq972aiWJ9PdSS8NHHt3taItGhP+6DYKLsoF1h5OwxZtWTFNDRYpVb6Afw0h\r\n"\
"2YNs9V+MlQhUPq9bKKjotTLCAUfavnppBjmUEzXABtRH7ZQYcQxYKtl8LElQ9ErP\r\n"\
"+TRK0fx6X2/SnC22oIQmxdT7yfykjYau9YSBFD6S1nCr6DmPJVRllixmIxI9gnHw\r\n"\
"fZavUsyW1lrRJdjd9eqgueSTA8WvpTwQ7H5Tbk0f0s/SYKiM1t7hWt9oYhC0VXmR\r\n"\
"Rc4RQMenx6AtQlA1+53VUGGHShbUmHoZ2J/advTnZuad0H4d+b7ubbPm7DWFBlQa\r\n"\
"qFte2F1dK1IPdtyCqwIDAQABo4GhMIGeMAkGA1UdEwQCMAAwCwYDVR0PBAQDAgXg\r\n"\
"MBMGA1UdJQQMMAoGCCsGAQUFBwMCMDYGA1UdHwQvMC0wK6ApoCeGJWh0dHA6Ly93\r\n"\
"d3cuZXhhbXBsZS5jb20vZXhhbXBsZV9jYS5jcmwwNwYIKwYBBQUHAQEEKzApMCcG\r\n"\
"CCsGAQUFBzABhhtodHRwOi8vd3d3LmV4YW1wbGUub3JnL29jc3AwDQYJKoZIhvcN\r\n"\
"AQELBQADggEBACtt5odXKOkLpe8/JAehrdnATc3hhjFo1xgAL5N1uw0Do/DE28ml\r\n"\
"UDi3pZGJWv7kZBCJhepCrUmgNXKOJBrr2ADuAUE6Fc+/O9U4iY2+svVoT2YepRZr\r\n"\
"7gO0wE9LEEOjdJy7E7F6wIXFQIUWAyFaFuHo5SHTcpZvWi7AnF2Za9yeOGnRwGQJ\r\n"\
"4ClpdqNo0DgV2LDHAVTHUMiAVAuZrafZRsNpf1peREe9600dtfi+trKFZn0x3AXq\r\n"\
"9Qn/63TBZRxzldMm0/r7hQV+L3gsb50OqAGFqxx/TCgzq2ROMX32IXXF4j0SIY+I\r\n"\
"//QPpGHgnAA/lWo4cZi9OGqQSOA+s9p4nsY=\r\n"\
		"-----END RSA PRIVATE KEY-----\r\n"\
		"\0"\
		"\0";


const uint8_t *DEFAULT_DTLS_CERTIFICATE	= ( uint8_t *) \
	"-----BEGIN CERTIFICATE-----\r\n"\
	"MIID4TCCAsmgAwIBAgIBBDANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCVVMx\r\n"\
	"DzANBgNVBAgTBk5ldmFkYTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFzAVBgNVBAoT\r\n"\
	"DlNpZXJyYSBUZWxlY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJyYXRl\r\n"\
	"bGVjb20ubmV0MRcwFQYDVQQDEw5TaWVycmEgVGVsZWNvbTAgFw0xNjExMjEwMTQ5\r\n"\
	"MDdaGA8yMDY2MTEwOTAxNDkwN1owgYMxCzAJBgNVBAYTAlVTMQ8wDQYDVQQIEwZO\r\n"\
	"ZXZhZGExFzAVBgNVBAoTDlNpZXJyYSBUZWxlY29tMSEwHwYDVQQDFBhkZXZpY2VA\r\n"\
	"c2llcnJhdGVsZWNvbS5uZXQxJzAlBgkqhkiG9w0BCQEWGGRldmljZUBzaWVycmF0\r\n"\
	"ZWxlY29tLm5ldDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALdIgjUC\r\n"\
	"OyF87Ju4zq2XG4a2TmIlfzqCy/X1iPeUYgqB6wNcwNpSdw0j52w0ZFWrUYn1P/iU\r\n"\
	"U5PXgWo+UgjzzoR9ZNXAXwZNVg7xa1PXOjf3T2y02YyKuHDrUiiMegX4pRVEQFRA\r\n"\
	"4GNyUJJt4B9EgTz/c0hWPFPGmzFmW/SvT8LLLmCCq8G1WyFFUkGc2FdxVnJACJEG\r\n"\
	"klVaYxRTkpZGNsbWJyEHCYPhRJhKXZTxNOApTeWiu+GdT942ehhY8g4JmFYaNaD1\r\n"\
	"6jh01wHACw5gh9x+pBdBSkSiTIYLZUAoPNwesU9+uowYvrd2/LLxYUN0v/hlZGXy\r\n"\
	"FRoEWqgXH/Ir+3ECAwEAAaNPME0wEwYDVR0lBAwwCgYIKwYBBQUHAwIwNgYDVR0f\r\n"\
	"BC8wLTAroCmgJ4YlaHR0cDovL3d3dy5leGFtcGxlLmNvbS9leGFtcGxlX2NhLmNy\r\n"\
	"bDANBgkqhkiG9w0BAQsFAAOCAQEAmDbqh/9wGSHXSs6XTuYZIFzg5SWp40xCZ7Oa\r\n"\
	"QpCCSTKstE3c+P/iTY3s+B1whNiNSe3bbpJ6JKbZzQl3RxYQNF79RPAd6FTQyFNl\r\n"\
	"GLYuN/Evgb7yQKRxcxOF3zABqUobTh/2Np2V+Xb2Ejg+2k8shmOK8TdQi/KGSfDi\r\n"\
	"llozXlInfejXPqXLiB5E544u+fILJFbW+d3tuMB1J8Evk9HO1SFsWlVMnC+/ks3Y\r\n"\
	"5i9sd5RIiBNYBNq6eC8xp9axmWIoLQvnj9jfMeevSAfdhHMvnB++O6oZQ+x8jXzP\r\n"\
	"KDbfGaBPFIQXiA9oRuUq8lfIQ4SMblSPqZr1mLB5Q551k05w4w==\r\n"\
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
