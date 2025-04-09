#include <stdlib.h>

#include <fullsdkfragapi.h>
#include <fullsdkmgt.h>
#include <fullsdknet.h>
#include <platform.h>
#include <arpa/inet.h>

#include "time.h"

#include "common_defs.h"
#include "net_helper.h"
#include "schc_helper.h"

#define APP_PAYLOAD_SIZE 100

static sdk_mode_t sdk_mode = SDK_DEVICE_MODE;

static bool app_process_request = false;
static bool app_send_data_request = false;

static uint8_t app_payload[APP_PAYLOAD_SIZE] = {0};

// NET callbacks.
static void net_transmission_result(net_status_t status, uint16_t error) {
  if (status != NET_SUCCESS) {
    PRINT_MSG("transmission result KO (status %d)\n", status);
    return;
  }
  PRINT_MSG("transmission result OK\n");
}

static void net_data_received(const uint8_t *buffer, uint16_t data_size,
                              net_status_t status) {
  PRINT_MSG("data received %d bytes - status %d\n", data_size, status);
  uint8_t b;
  for (uint16_t i = 0; i < data_size; i++) {
    b = buffer[i];
    PRINT_MSG("%02X ", b);
    if ((i + 1) % 10 == 0)
      PRINT_MSG("\n");
  }
  PRINT_MSG("\n");

  app_process_request = true;
  app_send_data_request = true;
}

static net_callbacks_t net_callbacks = {
  net_transmission_result,
  net_data_received
};

/**
 * Device sends a dummy CoAP packet with sensor value to the SCHC Core using IPv4 tunnel.
 */
int main() {
  PRINT_MSG("FullSDK version: %s\n", mgt_get_version());

  start_config();

  with_mtu(100);
  with_sdk_mode(sdk_mode);
  with_tunnel(
    DEVICE_TUNNEL_ADDR,
    DEVICE_TUNNEL_PORT,
    CORE_TUNNEL_ADDR,
    CORE_TUNNEL_PORT
  );
  with_ipv6_src_param(DEVICE_ADDR);
  with_ipv6_udp_src_param(DEVICE_PORT);
  with_ipv6_dst_param(APP_ADDR);
  with_ipv6_udp_dst_param(APP_PORT);
  with_net_callbacks(net_callbacks);

  if (config_schc() != SCHC_CONFIG_OK) {
    return EXIT_FAILURE;
  }

  // Initialize the IPv6 --> UDP --> CoAP payload.
  /*
    Frame 112783: 72 bytes on wire (576 bits), 72 bytes captured (576 bits) on interface lo, id 0
    Ethernet II, Src: 00:00:00_00:00:00 (00:00:00:00:00:00), Dst: 00:00:00_00:00:00 (00:00:00:00:00:00)
    Internet Protocol Version 4, Src: 127.0.0.1, Dst: 127.0.0.1
    User Datagram Protocol, Src Port: 53924, Dst Port: 5683
    Constrained Application Protocol, Confirmable, POST, MID:32251
        01.. .... = Version: 1
        ..00 .... = Type: Confirmable (0)
        .... 0000 = Token Length: 0
        Code: POST (2)
        Message ID: 32251
        Opt Name: #1: Uri-Path: notifications
        Opt Name: #2: Uri-Path: temp
        Opt Name: #3: Content-Format: application/cbor
        End of options marker: 255
        Payload: Payload Content-Format: application/cbor, Length: 3
        [Uri-Path: /notifications/temp]
    Concise Binary Object Representation
        Float: 18

    ---------------

    40 02 7d fb bd 00 6e 6f 74 69 66 69 63 61 74 69
    6f 6e 73 04 74 65 6d 70 11 3c ff f9 4c 80

    Flow Label:
    06 72 4d

   */

  const uint8_t coap_payload[] = {
    0x40, 0x02, 0x7d, 0xfb, 0xbd, 0x00, 0x6e, 0x6f, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x04, 0x74, 0x65, 0x6d, 0x70, 0x11, 0x3c, 0xff, 0xf9, 0x4c, 0x80
  };
  generate_ipv6_udp_packet(app_payload, DEVICE_ADDR, DEVICE_PORT, APP_ADDR, APP_PORT, coap_payload, sizeof(coap_payload));

  // PRINT_MSG("\n");
  // for (uint16_t i = 0; i < IPV6_HEADERS_BYTES + UDP_HEADERS_BYTES + sizeof(coap_payload); i++) {
  //   uint8_t b = app_payload[i];
  //   PRINT_MSG("%02X ", b);
  //   if ((i + 1) % 10 == 0)
  //     PRINT_MSG("\n");
  // }
  // PRINT_MSG("\n");

  app_send_data_request = true;
  while (true) {
    if (mgt_process_request) {
      mgt_process_request = false;
      const mgt_status_t mgt_status = mgt_process();

      if (mgt_status != MGT_SUCCESS) {
        PRINT_MSG("Error processing SCHC packet (%d)", mgt_status);
        return EXIT_FAILURE;
      }
    }
    if (app_send_data_request) {
      net_status_t net_status = net_sendto(app_payload, IPV6_HEADERS_BYTES + UDP_HEADERS_BYTES + sizeof(coap_payload));

      if (net_status != NET_SUCCESS)
      {
        PRINT_MSG("net_sendto() failed (status %d)\n", net_status);
        return EXIT_FAILURE;
      }

      app_send_data_request = false;
    }

    if (!app_send_data_request) {
      platform_enter_low_power_ll();
    }
  }

  return EXIT_SUCCESS;
}