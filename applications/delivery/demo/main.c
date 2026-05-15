#include <stdio.h>
#include <stdlib.h>

// For trace functions.
#include "fullsdktrace.h"
#include "time.h"

#include "fullsdkextapi.h"
#include "fullsdkfragapi.h"
#include "fullsdkl2.h"
#include "fullsdkmgt.h"
#include "fullsdknet.h"
#include "platform.h"

int main() {
  /*
    const uint8_t localhost_ip[] = {0x7F, 0x00, 0x00, 0x01};
    const uint8_t src_port[] = {0x16, 0x34};
    const uint8_t dst_port[] = {0x16, 0x33};

    tpl_set_index_param_value(0, localhost_ip, IPV4_ADDRESS_LENGTH_BYTES);
    tpl_set_index_param_value(1, localhost_ip, IPV4_ADDRESS_LENGTH_BYTES);
    tpl_set_index_param_value(2, src_port, IP_PORT_LENGTH_BYTES);
    tpl_set_index_param_value(3, dst_port, IP_PORT_LENGTH_BYTES);
   */

  printf("Demo SCHC app works!");
}