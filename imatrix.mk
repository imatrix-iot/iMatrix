#
# Copyright 2015, Sierra Telecom, Inc.
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Sierra Telecom, Inc.
#
#
# Copyright 2017, Sierra Telecom, Inc. or a subsidiary of 
# Sierra Telecom, Inc.. All Rights Reserved.
# This software, including source code, documentation and related
# materials ("Software"), is owned by Sierra Telecom, Inc.
# or one of its subsidiaries ("Sierra") and is protected by and subject to
# worldwide patent protection (United States and foreign),
# United States copyright laws and international treaty provisions.
# Therefore, you may use this Software only as provided in the license
# agreement accompanying the software package from which you
# obtained this Software ("EULA").

# Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Sierra
# reserves the right to make changes to the Software without notice. Sierra
# does not assume any liability arising out of the application or use of the
# Software or any product or circuit described in the Software. Sierra does
# not authorize its products for use in any products where a malfunction or
# failure of the Sierra product may reasonably be expected to result in
# significant property damage, injury or death ("High Risk Product"). By
# including Sierra's product in a High Risk Product, the manufacturer
# of such system or application assumes all risk of such use and in doing
# so agrees to indemnify Sierra against all liability.
#

NAME := iMatix_Library_$(PLATFORM)

$(NAME)_SOURCES := imatrix.c imatrix.h storage.c storage.h \
at_cmds/at_cmds.c at_cmds/at_cmds.h \
cli/cli.c cli/cli.h cli/cli_help.c cli/cli_help.h cli/cli_reboot.c cli/cli_reboot.h cli/cli_status.c cli/status.h \
cli/cli_set_ssid.c cli/cli_set_ssid.h cli/cli_dump.c cli/cli_dump.h cli/cli_set_serial.c cli/cli_set_serial.h \
cli/interface.c cli/interface.h cli/print_dct.c cli/print_dct.h cli/telnetd.c cli/telnetd.h cli/cli_debug.c cli/cli_debug.h \
coap/coap.c coap/coap.h coap/coap_setup.c coap/coap_setup.h coap/coap_udp_recv.c coap_udp_recv.h \
coap/add_coap_option.c coap/add_coap_option.h coap/coap_process.c coap/coap_process.h \
coap/coap_reliable_udp.c coap/coap_reliable_udp.h \
coap/que_manager.c coap/queue_manager.h \
coap/sent_message_list.c coap/sent_message_list.h \
coap/coap_token.c coap/coap_token.h \
coap/coap_receive.c coap/coap_receive.h \
coap/coap_transmit.c coap/coap_transmit.h \
coap/tcp_transport.c coap/tcp_transport.h \
coap/coap_tcp_recv.c coap/coap_tcp_recv.h \
coap/udp_transport.c coap/udp_transport.h \
coap_interface/coap_config_imatrix.c coap_interface/coap_config_imatrix.h \
coap_interface/coap_control_cs_ctrl.c coap_interface/coap_control_cs_ctrl.h \
coap_interface/coap_control_getlatest.c coap_interface/coap_control_getlatest.h \
coap_interface/coap_control_imatrix.c coap_interface/coap_control_imatrix.h \
coap_interface/coap_control_otaupdate.c coap_interface/coap_control_otaupdate.h \
coap_interface/coap_control_reboot.c coap_interface/coap_control_reboot.h \
coap_interface/coap_control_securessid.c coap_interface/coap_control_securessid.h \
coap_interface/coap_control_security.c coap_interface/coap_control_security.h \
coap_interface/coap_def.c coap_interface/coap_def.h \
coap_interface/coap_msg_get_store.c coap_interface/coap_msg_get_store.h \
coap_interface/coap_sensor_rssi.c coap_interface/coap_sensor_rssi.h \
coap_interface/get_uint_from_query_str.c coap_interface/get_uint_from_query_str.h \
coap_interface/match_uri.c coap_interface/match_uri.h \
coap_interface/token_string.c coap_interface/token_string.h \
cs_ctrl/controls. cs_ctrl/controls.h cs_ctrl/sensors.c cs_ctrl/sensors.h cs_ctrl/simulated.c cs_ctrl/simulated.h \
cs_ctrl/hal_event.c cs_ctrl/hal_event.h cs_ctrl/hal_sample.c cs_ctrl/hal_sample.h  \
device/cert_util.c device/cert_util.h device/config.c device/config.h device/device.c device/device.h \
device/hal_wifi.c device/hal_wifi.h device/set_serial.c device/set_serial.h device/system_init.c device/system_init.h device/var_data.c device/var_data.h \
imatrix/add_internal.c imatrix/add_internal.h imatrix/imatrix_get_ip.c imatrix/imatrix_get_ip.h imatrix/imatrix.c imatrix/imatrix.h imatrix/registration.c imatrix/registration.h \
location/location.c location/location.h \
manufacturing/manufacturing.c manufacturing/manufacturing.h \
networking/get_inbound_destination_ip.c networking/get_inbound_destination_ip.h networking/http_get_sn_mac_address.c networking/http_get_sn_mac_address.h \
networking/keep_alive.c networking/keep_alive.h networking/utility.c networking/utility.h \
time/ck_time.c time/ck_time.h time/ntp_success.c time/ntp_success.h time/sntp.c time/sntp.h \
wifi/enterprise_80211.c wifi/enterprise_80211.h wifi/imx_wifi.c wifi/process_wifi.c wifi/process_wifi.h wifi/wifi/c wifi/wifi/h
$(NAME)_DEFINES += IMATRIX_CLIENT

GLOBAL_INCLUDES := .

#$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)

APPLICATION_DCT := device_app_dct.c
