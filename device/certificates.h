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

#define WIFI_USER_CERTIFICATE_STRING  (const uint8_t*)\
"-----BEGIN CERTIFICATE-----\r\n"\
"MIID+DCCA2GgAwIBAgIJALVGBV3GrJvwMA0GCSqGSIb3DQEBCwUAMF8xEzARBgNV\r\n"\
"BAMMCldJQ0VTIFRFU1QxEjAQBgNVBAoMCXdpY2VkLmNvbTEUMBIGA1UECwwLZ2Vu\r\n"\
"ZXJhdGUtQ0ExHjAcBgkqhkiG9w0BCQEWD3dpY2VkQHdpY2VkLmNvbTAeFw0xNzA5\r\n"\
"MjYxMjQ1NDZaFw0zMjA5MjIxMjQ1NDZaMFsxDzANBgNVBAMMBmNsaWVudDESMBAG\r\n"\
"A1UECgwJd2ljZWQuY29tMRQwEgYDVQQLDAtnZW5lcmF0ZS1DQTEeMBwGCSqGSIb3\r\n"\
"DQEJARYPd2ljZWRAd2ljZWQuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKB\r\n"\
"gQD2b6OGIsaQyNik7ArsTgYmwazvAoXtiinrAfTSHoO+bVwJ14CWYkLji3rh/J1A\r\n"\
"56Wcqfe+XvG0TGqR1wI9mM+MKiaBxURf3L2a11jYeyu6eRx7SBdL9YmpaaViFH0G\r\n"\
"tjYFoOlBaegYzv7v8qFXyP7pEcFnP8ylICCOVpQ10PR6gQIDAQABo4IBvjCCAbow\r\n"\
"DAYDVR0TAQH/BAIwADARBglghkgBhvhCAQEEBAMCBkAwCwYDVR0PBAQDAgXgMCEG\r\n"\
"CWCGSAGG+EIBDQQUFhJCcm9rZXIgQ2VydGlmaWNhdGUwHQYDVR0OBBYEFAes9Drm\r\n"\
"9vXV1iYSxSzCMVyIDFImMIGRBgNVHSMEgYkwgYaAFLP9T+mRPngmbmm5zLWua5XK\r\n"\
"EnAKoWOkYTBfMRMwEQYDVQQDDApXSUNFUyBURVNUMRIwEAYDVQQKDAl3aWNlZC5j\r\n"\
"b20xFDASBgNVBAsMC2dlbmVyYXRlLUNBMR4wHAYJKoZIhvcNAQkBFg93aWNlZEB3\r\n"\
"aWNlZC5jb22CCQDXSVxrY/fHfjBEBgNVHREEPTA7hwQKKAJphxD+gAAAAAAAADLj\r\n"\
"daK6xziWhwR/AAABhxAAAAAAAAAAAAAAAAAAAAABgglsb2NhbGhvc3QwbgYDVR0g\r\n"\
"BGcwZTBjBgMrBQgwXDAcBggrBgEFBQcCARYQaHR0cDovL2xvY2FsaG9zdDA8Bggr\r\n"\
"BgEFBQcCAjAwMAwWBVdJQ0VEMAMCAQEaIFRoaXMgQ0EgaXMgZm9yIGEgdGVzdGlu\r\n"\
"ZyBwdXJwb3NlMA0GCSqGSIb3DQEBCwUAA4GBAAMcEceIM1mzZFBUtsog3fO+CV4r\r\n"\
"c3N+cFzBgRF0WFl3ygHJqg7Las+yIT8wfnIZNCkZiFiXZzz0ezKxQO4LZccrYrcv\r\n"\
"ffOnwkT7vgKYtQ+RXc197U0odFVJj1lPfjOL7eaKXrxQB4dxEBjgX7/J0/OZIupH\r\n"\
"rkC4k1mY1JxmtsOu\r\n"\
"-----END CERTIFICATE-----\r\n"\
"\0"\
"\0"
#define WIFI_USER_PRIVATE_KEY_STRING  \
"-----BEGIN RSA PRIVATE KEY-----\r\n"\
"MIICXwIBAAKBgQD2b6OGIsaQyNik7ArsTgYmwazvAoXtiinrAfTSHoO+bVwJ14CW\r\n"\
"YkLji3rh/J1A56Wcqfe+XvG0TGqR1wI9mM+MKiaBxURf3L2a11jYeyu6eRx7SBdL\r\n"\
"9YmpaaViFH0GtjYFoOlBaegYzv7v8qFXyP7pEcFnP8ylICCOVpQ10PR6gQIDAQAB\r\n"\
"AoGBAIDrpGNu2+wwdk3JAmne447w8TRUYJYFGqrL6jTmK8u6UFUBvU5u90ks1ctV\r\n"\
"qTqkNNqBfI7GArJs66+CCLUKtUA6m6uFvGtLzQCvJNrLx+YLDOy2tXZajCOhcSXC\r\n"\
"D/BL0/NuhklwNCxQunOuaVL3D8HdYLsygrvUwHKj+d3bnidFAkEA+77VI+MxvuOm\r\n"\
"acJv/580+J78SIh0ouW6Br5z7hxNO2t6SRN3Z0TL0ypQddiVg4QJk7rEbgU+h6OG\r\n"\
"Lbt/Pta5ewJBAPqZ1eGtRjfGIlzlKUGb0JqFEbdzFiI235ULHtbpJAScqKOtf5eg\r\n"\
"VR+PoGJtIXSgp9sh+5F7jNwH/IukzC8AZTMCQQDdmWETQxVx/ABrzj06XZGKXKCk\r\n"\
"1IEfN2smLpXr/ru5V2WAWANeQv/MdM34vlr2Ns8bXGc2laUyTq55KTfcP7VrAkEA\r\n"\
"gJssIVoMPBlJj0TnrEzrfDEoS24bgMmKanG/jOku29MN1Jn4bfcRWFYcKAb7cV/1\r\n"\
"ZMcG1L2EJ3NNzthvFPvt8wJBAIj4dxCpI8ICRShnq2qLy7rgHq6qg+crnpeXF2m7\r\n"\
"olYYFGWTVHhbZmRE5jQQ75W9sJbsBkIrVW/74lKgSLszkZ4=\r\n"\
"-----END RSA PRIVATE KEY-----\r\n"\
"\0"\
"\0"

#define WIFI_ROOT_CERTIFICATE_STRING  \
"-----BEGIN CERTIFICATE-----\r\n"\
"MIICjDCCAfWgAwIBAgIJANdJXGtj98d+MA0GCSqGSIb3DQEBCwUAMF8xEzARBgNV\r\n"\
"BAMMCldJQ0VTIFRFU1QxEjAQBgNVBAoMCXdpY2VkLmNvbTEUMBIGA1UECwwLZ2Vu\r\n"\
"ZXJhdGUtQ0ExHjAcBgkqhkiG9w0BCQEWD3dpY2VkQHdpY2VkLmNvbTAeFw0xNzA5\r\n"\
"MjYxMjQ1NDBaFw0zMjA5MjIxMjQ1NDBaMF8xEzARBgNVBAMMCldJQ0VTIFRFU1Qx\r\n"\
"EjAQBgNVBAoMCXdpY2VkLmNvbTEUMBIGA1UECwwLZ2VuZXJhdGUtQ0ExHjAcBgkq\r\n"\
"hkiG9w0BCQEWD3dpY2VkQHdpY2VkLmNvbTCBnzANBgkqhkiG9w0BAQEFAAOBjQAw\r\n"\
"gYkCgYEAyFGnrpV42vuVuTVwWp47Js091x+FYUQarT7SnwkmWk3X/DEtNCQs/tYZ\r\n"\
"W/GhPAv6PVyIkoV4YZoNboqHRKi1Er0oEo1UitbSf4d6v7k3u9psisQOWEMhA+Xv\r\n"\
"Ep+KUCNU2ge19qyMUXJ5Cdd9cFZOXPJdHgX4ktSMYtc64MzXsIcCAwEAAaNQME4w\r\n"\
"HQYDVR0OBBYEFLP9T+mRPngmbmm5zLWua5XKEnAKMB8GA1UdIwQYMBaAFLP9T+mR\r\n"\
"Pngmbmm5zLWua5XKEnAKMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQELBQADgYEA\r\n"\
"eEoo2PSQMNsThDGxiVBS4mZ4+BSlDSkaCXeCsDprFpybzMNIeiUNUIWFdrlkc+Hb\r\n"\
"ERKPM3+GfXDmJsAY2qjmFUlAS2xlSOYNXFZfsubQVJfN6sLB7d0ItNnz0irw0D/i\r\n"\
"eIav5qn6diBbr8+TQ1dN2IzfxFpEM9DharE7+x51qyA=\r\n"\
"-----END CERTIFICATE-----\r\n"\
"\0"\
"\0"

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
