#include "schc_helper.h"

#include <stdlib.h>
#include <arpa/inet.h>

#include <fullsdkextapi.h>
#include <fullsdkl2.h>
#include <platform.h>

#define RECEIVE_BUFFER_SIZE 1500
#define TRANSMISSION_BUFFER_SIZE 1500
#define APP_PAYLOAD_SIZE 100

// Assume (MAX_PAYLOAD_SIZE + MGT_PROTO_SIZE) must be 4-bytes aligned
#define MAX_PAYLOAD_SIZE RECEIVE_BUFFER_SIZE
#define MAX_MTU_SIZE 256 // Assume MAX_MTU_SIZE must be 4-bytes aligned

// Memory block size provided to mgt_initialize.
#define MEM_BLOCK_SIZE                                                         \
(MAX_MTU_SIZE * 4u + (MAX_PAYLOAD_SIZE + MGT_PROTO_SIZE) * 4u +              \
MGT_SCHC_ACK_PACKET_SIZE * 3u + 64 * 2u)

// WARNING!!! Memory Allocation
#define INIT_PARAM(PARAM, SIZE) PARAM->size = SIZE; \
                        if (!PARAM->initialized) { \
                            PARAM->value = malloc(PARAM->size); \
                            memset(PARAM->value, 0, PARAM->size); \
                            PARAM->initialized = true; \
                        }

bool mgt_process_request = false;
static uint8_t mgt_mem_block[MEM_BLOCK_SIZE];

static TimerEvent_t sdk_timers[3];

#define MAX_TEMPLATE_PARAMS 50

static bool initialized = false;
static schc_config_t staging_config = {};
static schc_template_param_t template_params[MAX_TEMPLATE_PARAMS];

schc_config_result_t clean_params();

schc_config_result_t start_config() {
    if (initialized) {
        clean_params();
    }
    memset(&staging_config, 0, sizeof(schc_config_t));

    staging_config.params = template_params;
    initialized = true;
    return SCHC_CONFIG_OK;
}

schc_config_result_t with_mtu(const uint16_t mtu) {
    staging_config.mtu = mtu;
    return SCHC_CONFIG_OK;
}

schc_config_result_t with_sdk_mode(const sdk_mode_t mode) {
    staging_config.mode = mode;
    return SCHC_CONFIG_OK;
}

schc_config_result_t with_tunnel(const char *host_addr, const char *host_port, const char *dst_addr,
                                 const char *dst_port) {
    strcpy(staging_config.host_addr.addr, host_addr);
    strcpy(staging_config.host_addr.port, host_port);
    strcpy(staging_config.dst_addr.addr, dst_addr);
    strcpy(staging_config.dst_addr.port, dst_port);
    return SCHC_CONFIG_OK;
}

schc_config_result_t with_net_callbacks(const net_callbacks_t net_callbacks) {
    staging_config.net_callbacks = net_callbacks;
    return SCHC_CONFIG_OK;
}

schc_config_result_t with_ipv6_src_param(const char* dev_addr) {
    schc_template_param_t* ipv6_prefix_param = &staging_config.params[0];
    schc_template_param_t* ipv6_iid_param = &staging_config.params[1];

    const uint8_t src_addr[IPV6_ADDR_BYTES];
    inet_pton(AF_INET6, dev_addr, src_addr);

    INIT_PARAM(ipv6_prefix_param, IPV6_ADDR_PREFIX_BYTES);
    memcpy(ipv6_prefix_param->value, src_addr, IPV6_ADDR_PREFIX_BYTES);

    INIT_PARAM(ipv6_iid_param, IPV6_ADDR_BYTES - IPV6_ADDR_PREFIX_BYTES);
    memcpy(ipv6_iid_param->value, src_addr + IPV6_ADDR_PREFIX_BYTES, ipv6_iid_param->size);

    return SCHC_CONFIG_OK;
}

schc_config_result_t with_ipv6_dst_param(const char* app_addr) {
    schc_template_param_t* ipv6_prefix_param = &staging_config.params[2];
    schc_template_param_t* ipv6_iid_param = &staging_config.params[3];

    const uint8_t dst_addr[IPV6_ADDR_BYTES];
    inet_pton(AF_INET6, app_addr, dst_addr);

    INIT_PARAM(ipv6_prefix_param, IPV6_ADDR_PREFIX_BYTES);
    memcpy(ipv6_prefix_param->value, dst_addr, IPV6_ADDR_PREFIX_BYTES);

    INIT_PARAM(ipv6_iid_param, IPV6_ADDR_BYTES - IPV6_ADDR_PREFIX_BYTES);
    memcpy(ipv6_iid_param->value, dst_addr + IPV6_ADDR_PREFIX_BYTES, ipv6_iid_param->size);

    return SCHC_CONFIG_OK;
}

schc_config_result_t with_ipv6_udp_src_param(const char* src_param) {
    schc_template_param_t* udp_port_param = &staging_config.params[4];

    const uint16_t src_port = htons(atoi(src_param));

    INIT_PARAM(udp_port_param, UDP_PORT_BYTES);
    memcpy(udp_port_param->value, &src_port, UDP_PORT_BYTES);

    return SCHC_CONFIG_OK;
}

schc_config_result_t with_ipv6_udp_dst_param(const char* dst_param) {
    schc_template_param_t* udp_port_param = &staging_config.params[5];

    const uint16_t dst_port = htons(atoi(dst_param));

    INIT_PARAM(udp_port_param, UDP_PORT_BYTES);
    memcpy(udp_port_param->value, &dst_port, UDP_PORT_BYTES);

    return SCHC_CONFIG_OK;
}

static void sdk_timer_1_event(void *context) {
    mgt_timer_timeout(0);
}

static void sdk_timer_2_event(void *context) {
    mgt_timer_timeout(1);
}

static void sdk_timer_3_event(void *context) {
    mgt_timer_timeout(2);
}

// MGT callbacks.
static void cb_mgt_processing_required(void) {
    mgt_process_request = true;
}

static void cb_mgt_connectivity_state(mgt_status_t status) {
    if (status != MGT_SUCCESS) {
        PRINT_MSG("connectivity KO (status %d)\n", status);
        return;
    }
    PRINT_MSG("connectivity OK\n");
}

static void cb_start_timer(uint8_t id, uint32_t duration) {
    TimerSetValue(&sdk_timers[id], duration);
    TimerStart(&sdk_timers[id]);
}

static void cb_stop_timer(uint8_t id) {
    TimerStop(&sdk_timers[id]);
}

static mgt_callbacks_t mgt_callbacks = {
    cb_mgt_processing_required,
    cb_mgt_connectivity_state,
    cb_start_timer,
    cb_stop_timer,
    NULL,
  };

schc_config_result_t clean_params() {
    for (int i = 0; i < MAX_TEMPLATE_PARAMS; ++i) {
        free(staging_config.params[i].value);
        memset(&staging_config.params[i], 0, sizeof(schc_template_param_t));
    }

    return SCHC_CONFIG_OK;
}

schc_config_result_t config_schc() {
    if (!initialized) {
        PRINT_MSG("Error : schc config is not initialized!\n");
        goto finish_failure;
    }

    // SDK timers initialization.
    TimerInit(&sdk_timers[0], sdk_timer_1_event);
    TimerInit(&sdk_timers[1], sdk_timer_2_event);
    TimerInit(&sdk_timers[2], sdk_timer_3_event);

    l2_set_mtu(staging_config.mtu);

#ifdef L2_STACK_udp
    l2_set_ipv4_host_addr(staging_config.host_addr.addr);
    l2_set_ipv4_remote_addr(staging_config.dst_addr.addr);
    l2_set_udp_src_port(staging_config.host_addr.port);
    l2_set_udp_dest_port(staging_config.dst_addr.port);
#endif

    // Informs the SDK regarding the application mode in order to use the correct
    // fragmentation profile.
    mgt_set_mode(staging_config.mode);

    if (mgt_initialize(&mgt_callbacks, mgt_mem_block, MEM_BLOCK_SIZE,
                       MAX_MTU_SIZE, MAX_PAYLOAD_SIZE) != MGT_SUCCESS) {
        PRINT_MSG("Error : mgt_initialize() failed\n");
        goto finish_failure;
    }

    uint8_t nb_params;
    mgt_get_nb_template_params(&nb_params);
    for (int i = 0; i < nb_params; i++) {
        if (staging_config.params[i].initialized) {
            mgt_status_t res = mgt_set_template_param(i, staging_config.params[i].value, staging_config.params[i].size);
            if (res != MGT_SUCCESS) {
                PRINT_MSG("Error : mgt_set_template_param() failed\n");
                goto finish_failure;
            }
        }
    }

    const net_status_t status = net_initialize(&staging_config.net_callbacks);
    if (status != NET_SUCCESS) {
        PRINT_MSG("Error : net_initialize() failed (status %d)\n", status);
        return SCHC_CONFIG_KO;
    }

    finish_success:
    clean_params();
    initialized = false;
    return SCHC_CONFIG_OK;

    finish_failure:
    clean_params();
    initialized = false;
    return SCHC_CONFIG_KO;
}
