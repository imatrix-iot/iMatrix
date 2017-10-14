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

/** @file certificates.h
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

const char *WIFI_ROOT_CERTIFICATE_STRING = \
"-----BEGIN CERTIFICATE-----\r\n"\
"MIIE1zCCA7+gAwIBAgIJAOXZzKTf/mXKMA0GCSqGSIb3DQEBBQUAMIGOMQswCQYD\r\n"\
"VQQGEwJVUzELMAkGA1UECBMCQ0ExFDASBgNVBAcTC1plcGh5ciBDb3ZlMRUwEwYD\r\n"\
"VQQKEwxHU0EgVGVzdCBMYWIxLjAsBgkqhkiG9w0BCQEWH2dyZWcucGhpbGxpcHNA\r\n"\
"c2llcnJhdGVsZWNvbS5uZXQxFTATBgNVBAMTDEdTQSBUZXN0IExhYjAgFw0xNzA5\r\n"\
"MTQwMDI4MjFaGA8yMDY3MDkwMjAwMjgyMVowgY4xCzAJBgNVBAYTAlVTMQswCQYD\r\n"\
"VQQIEwJDQTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFTATBgNVBAoTDEdTQSBUZXN0\r\n"\
"IExhYjEuMCwGCSqGSIb3DQEJARYfZ3JlZy5waGlsbGlwc0BzaWVycmF0ZWxlY29t\r\n"\
"Lm5ldDEVMBMGA1UEAxMMR1NBIFRlc3QgTGFiMIIBIjANBgkqhkiG9w0BAQEFAAOC\r\n"\
"AQ8AMIIBCgKCAQEAttV7xu/VpufLhlEqeljaoeNftIvWly5PyChiVg4yNq9sGZWt\r\n"\
"pSG4+TkIUxgaYfKm7mxVlJGE7yrGtlkn2rswzzi3SWlt5ccQkj3XQodCYDsIaqcZ\r\n"\
"FIEDc2odPEZ6te2U5TpAo2A/GVQStErdSbKLKXteRt4Qq9uj+/6hOabzdBAXt6W8\r\n"\
"+zZvaJVBYqB90V8ljiLjC+a4D83OjTPBgOxto3WqP6mLifP/gZZbNzkUNcsskkl8\r\n"\
"2lOqOvTRdNfsptoaCnXBmGuWnM+N3FxtC0qY8Xs90HoRGUNehbqS5HxGr51qo/h7\r\n"\
"kKqDQK2z7QInXNrVrjiUkxLsnfdDQDBGq3tf+QIDAQABo4IBMjCCAS4wHQYDVR0O\r\n"\
"BBYEFEFPrwbAJeOyYYJaqJzA57PUEULHMIHDBgNVHSMEgbswgbiAFEFPrwbAJeOy\r\n"\
"YYJaqJzA57PUEULHoYGUpIGRMIGOMQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0Ex\r\n"\
"FDASBgNVBAcTC1plcGh5ciBDb3ZlMRUwEwYDVQQKEwxHU0EgVGVzdCBMYWIxLjAs\r\n"\
"BgkqhkiG9w0BCQEWH2dyZWcucGhpbGxpcHNAc2llcnJhdGVsZWNvbS5uZXQxFTAT\r\n"\
"BgNVBAMTDEdTQSBUZXN0IExhYoIJAOXZzKTf/mXKMA8GA1UdEwEB/wQFMAMBAf8w\r\n"\
"NgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL3d3dy5leGFtcGxlLmNvbS9leGFtcGxl\r\n"\
"X2NhLmNybDANBgkqhkiG9w0BAQUFAAOCAQEAPXjiRdsoM1B8FbzohPQqO8S95qmb\r\n"\
"MLsot5cISikSZeA1UxP1MoafCdZ0hX+Eznipn6la9CQ/lq+V2Bu7+9LmszXys0co\r\n"\
"muz6H62LabeIO3f2+5VAF3cCue0VahmSQuoXztxb6DFCvy4EiQaRtAVTmgxgZca1\r\n"\
"SXSPkh2pTvMS7n8j2ybYWpb/Ccu5pYlCeyOTvxIZj+sTl2QREmsk9Jq8l9q26fOP\r\n"\
"hOm0rnG9No4u5bdPZq0+tHQ/PEjzMPwHw87bg1YtB9j0Rleeb/DZDh6Pcef/3lT2\r\n"\
"9+mn+cavYcZBtPQpHkyPydKUUp7I4nfBHpNioJPjHHbMwxQSFWKZ2IettA==\r\n"\
"-----END CERTIFICATE-----\r\n"\
		"\0"\
		"\0";

const char *WIFI_USER_CERTIFICATE_STRING  = \
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

const char *WIFI_USER_PRIVATE_KEY_STRING = \
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


const char *DEFAULT_DTLS_CERTIFICATE = \
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
