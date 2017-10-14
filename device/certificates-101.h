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

const uint8_t *WIFI_USER_PRIVATE_KEY_STRING = ( uint8_t *) \
		"-----BEGIN RSA PRIVATE KEY-----\r\n"\
		"MIIEowIBAAKCAQEAt0iCNQI7IXzsm7jOrZcbhrZOYiV/OoLL9fWI95RiCoHrA1zA\r\n"\
		"2lJ3DSPnbDRkVatRifU/+JRTk9eBaj5SCPPOhH1k1cBfBk1WDvFrU9c6N/dPbLTZ\r\n"\
		"jIq4cOtSKIx6BfilFURAVEDgY3JQkm3gH0SBPP9zSFY8U8abMWZb9K9PwssuYIKr\r\n"\
		"wbVbIUVSQZzYV3FWckAIkQaSVVpjFFOSlkY2xtYnIQcJg+FEmEpdlPE04ClN5aK7\r\n"\
		"4Z1P3jZ6GFjyDgmYVho1oPXqOHTXAcALDmCH3H6kF0FKRKJMhgtlQCg83B6xT366\r\n"\
		"jBi+t3b8svFhQ3S/+GVkZfIVGgRaqBcf8iv7cQIDAQABAoIBAHVsI9dYNY/v3Bqs\r\n"\
		"HOWD8OOueqtQgKF42LkcdILBcspuj6VY0ElPGey2OvbM3hcwqg2rrS9Rv/Xw1H2/\r\n"\
		"rYkEwiOJvodfNsHv3Bm4u907/5WJgT4Q42uyGBZn/Pvze93FfeYREvGnmtq7izk0\r\n"\
		"4ajGl9H/itkmPoIhtJ3CiVlK09vkf8AUH1gWchT9wppyi5Q8IrcPkgWT1XK6xpKI\r\n"\
		"Qug1CKP+hEaLXtBLj2atv5irABIhFMNE9T7LPe2nZ9Nry2L+Tu8ZprN9Fjp7O17g\r\n"\
		"C4Y+M8+m7pvP2JAMvjR0z5XTq8UQlkvcwvM1raIYNrKBDaIQ2FcXvuI/cEeSghSw\r\n"\
		"ztoBGtECgYEA6rirTpD9ee4shUqCMoBuNok+6qBGxzRWKRHWFwxnbk1TEgeRQw1m\r\n"\
		"ckhwmDBDoOLfL8u0P2ONr8BvnOwA1d7A3a+Oe1SMjr3kkw5aOdngONda0fOgs/Rq\r\n"\
		"6iCuaLtAm4bhTB2g51sCJ6Xn0yR5/iHXs+ReqhZ9GoCKpNvhPDFYJN0CgYEAx+YU\r\n"\
		"3PdrNRGpRLYmh+wyAwhyDCLLl1xmsX5DSGe7mPc3K+utzK8VTNaaMdWOe/2GwRAo\r\n"\
		"22bbxvY0Vb0kGlBuD+9GXKhxXuVSRuByAL+rdSuxkLNF1dPmcxaysYVIAPrtUc2e\r\n"\
		"AeuYcQk+Yf5Dx8N9Pfo8I4QtRHQ0ero/wfIhDaUCgYBLQhzDGx/6uVATCLnIIX+1\r\n"\
		"Arpmli87bd92WZ6wUyzSo/5PmJeNzT0cxXc8hLUaX8O52nGXpX7nbCcdRFpcOIjU\r\n"\
		"S5a+mnRazC3+rKpnRCFteDdJe/j52hNxsDrbn7ZuZ4fTVOijZX3CYqJ7A4YN4qH7\r\n"\
		"ruxfUsiX98D8bPfByfx+DQKBgQCWW6LPvi7egQVQEK6UyH2puF2IcVev5ym7Uhyr\r\n"\
		"xpLd8P2HxMM61WrxG+5pk5eT/dOxPkVtGkYvtpsQ7q7MODlBNRo4sd5soZMsisBV\r\n"\
		"fdYq0Q3DqITb3IJXiPgp/PV2/0coo7+49Av64hgRP16eUJXQ23sGv/IIrluXAKba\r\n"\
		"bBSenQKBgFXqBey41kYCGWBRKNS9/JAGFPZQgfhhb3KW2PD9kDyBwSSeljkq1rzP\r\n"\
		"76S2gzEFxBvmQCzLWhRvhb2EODuQbSNb0n40jqTqvKLwSlte4Y+KZsnoXt01zc/4\r\n"\
		"tWpIGPd0KtNHOOt6nDugcrvRN85wnHQFf5w94xWkmOjG+gogeigD\r\n"\
		"-----END RSA PRIVATE KEY-----\r\n"\
		"\0"\
		"\0";

const uint8_t *WIFI_ROOT_CERTIFICATE_STRING = ( uint8_t *) \
		"-----BEGIN CERTIFICATE-----\r\n"\
		"MIIE2jCCA8KgAwIBAgIJAOK9FwwoLCwoMA0GCSqGSIb3DQEBBQUAMIGQMQswCQYD\r\n"\
		"VQQGEwJVUzEPMA0GA1UECBMGTmV2YWRhMRQwEgYDVQQHEwtaZXBoeXIgQ292ZTEX\r\n"\
		"MBUGA1UEChMOU2llcnJhIFRlbGVjb20xKDAmBgkqhkiG9w0BCQEWGXN1cHBvcnRA\r\n"\
		"c2llcnJhdGVsZWNvbS5uZXQxFzAVBgNVBAMTDlNpZXJyYSBUZWxlY29tMCAXDTE2\r\n"\
		"MTEyMTAxNDg1NloYDzIwNjYxMTA5MDE0ODU2WjCBkDELMAkGA1UEBhMCVVMxDzAN\r\n"\
		"BgNVBAgTBk5ldmFkYTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFzAVBgNVBAoTDlNp\r\n"\
		"ZXJyYSBUZWxlY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJyYXRlbGVj\r\n"\
		"b20ubmV0MRcwFQYDVQQDEw5TaWVycmEgVGVsZWNvbTCCASIwDQYJKoZIhvcNAQEB\r\n"\
		"BQADggEPADCCAQoCggEBAJlKDtqtvKaMeWvmMirjUXELdAAcbGkv8P2fWeChdmnn\r\n"\
		"hvGEVH4EMi5wjkMaCnahgOpkO23SCrDicabcwoDWqahJGp+EmsKm3/j0vgj45Vri\r\n"\
		"RqQYhHLO0qbqmnMmhCwMzlokDB9j4HOtK7smgZhuXD0Wvj+JVU233h1GYbV/zgXB\r\n"\
		"us7KJPFVjqEnkHbZS7GXWU/HFnPeYKXqQ0O5+8QAMbQ8QW+7DbQJfr5h7CuHi+1q\r\n"\
		"oSFDSPSTByTlYGT3we1Ti0g+pL7bNoExk8hS/xOk1FwdELFcD+eDQZNm9kIlLiqa\r\n"\
		"wJ2S1IcgolUVkaQQCf6TFEib34ZpWFrMpV+SlOTLnKsCAwEAAaOCATEwggEtMB0G\r\n"\
		"A1UdDgQWBBTY5f7RpfzYXOU/sXYXeJRyIgclMjCBxQYDVR0jBIG9MIG6gBTY5f7R\r\n"\
		"pfzYXOU/sXYXeJRyIgclMqGBlqSBkzCBkDELMAkGA1UEBhMCVVMxDzANBgNVBAgT\r\n"\
		"Bk5ldmFkYTEUMBIGA1UEBxMLWmVwaHlyIENvdmUxFzAVBgNVBAoTDlNpZXJyYSBU\r\n"\
		"ZWxlY29tMSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QHNpZXJyYXRlbGVjb20ubmV0\r\n"\
		"MRcwFQYDVQQDEw5TaWVycmEgVGVsZWNvbYIJAOK9FwwoLCwoMAwGA1UdEwQFMAMB\r\n"\
		"Af8wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL3d3dy5leGFtcGxlLmNvbS9leGFt\r\n"\
		"cGxlX2NhLmNybDANBgkqhkiG9w0BAQUFAAOCAQEAXX+AzPUNU10sy2jzRHwaZpxq\r\n"\
		"4wWYJvanaZEUC9C6m57ZvBYMLS2TaKnnjWPQKijYFvtShzwpHyxnlevh8MmccRoz\r\n"\
		"TAc2nC4FmJBHgM/EZUFMmj+M9Df+e9YWsoPHJM7/m9M2sumzwEp9mjn/rGt4Q2yA\r\n"\
		"JQhfqLwkCoNc7ZmkIRaL+1kKfIpZPa2lyDoxxhgTkU+OYXnq+prXGz8GgyJETvBf\r\n"\
		"9pkCgRoyssEBLDzrHMCjO9zr+1oJb7uKTSFQlu7udIa+Z6y/sYQtE7NWoDDrQZAY\r\n"\
		"wBhDEb901fE/mp8p2fkNY4qX6trFUeB2VmS1zQTv4105MAkDCcHYxX/bXMM3wA==\r\n"\
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
