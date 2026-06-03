#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <cmocka.h>

//#include "fullsdkl2a.h"
#include <fullsdkfragapi.h>
#include <fullsdkmgt.h>
#include <fullsdknet.h>
#include <fullsdkl2.h>
#include <platform.h>

#define RECEIVE_BUFFER_SIZE 1500
#define TRANSMISSION_BUFFER_SIZE 1500

// Assume (MAX_PAYLOAD_SIZE + MGT_PROTO_SIZE) must be 4-bytes aligned
#define MAX_PAYLOAD_SIZE RECEIVE_BUFFER_SIZE
#define MAX_MTU_SIZE 256 // Assume MAX_MTU_SIZE must be 4-bytes aligned

// Memory block size provided to mgt_initialize.
#define MEM_BLOCK_SIZE                                                         \
(MAX_MTU_SIZE * 4u + (MAX_PAYLOAD_SIZE + MGT_PROTO_SIZE) * 4u +              \
MGT_SCHC_ACK_PACKET_SIZE * 3u + 64 * 2u)

bool mgt_process_request = false;
bool volatile terminate = false;
static uint8_t mgt_mem_block[MEM_BLOCK_SIZE];

//static const char* ahoi_port = "/run/user/1000/slv_cons";
static const char* ahoi_port = "/run/user/1000/slv_triggered_prod";

static TimerEvent_t sdk_timers[3];

// Timers
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

// NET callbacks
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

//    app_process_request = true;
//    app_send_data_request = true;
    terminate = true;
}

static net_callbacks_t net_callbacks = {
        net_transmission_result,
        net_data_received
};

void test_ahoi_l2(void** state) {
    (void) state;

    // SDK timers initialization.
    TimerInit(&sdk_timers[0], sdk_timer_1_event);
    TimerInit(&sdk_timers[1], sdk_timer_2_event);
    TimerInit(&sdk_timers[2], sdk_timer_3_event);

    l2_set_mtu(20);
    l2_set_serial_port(ahoi_port);
    l2_set_iid(0x0A);

    // Informs the SDK regarding the application mode in order to use the correct
    // fragmentation profile.
    mgt_set_mode(SDK_DEVICE_MODE);

    if (mgt_initialize(&mgt_callbacks, mgt_mem_block, MEM_BLOCK_SIZE,
                       MAX_MTU_SIZE, MAX_PAYLOAD_SIZE) != MGT_SUCCESS) {
        PRINT_MSG("Error : mgt_initialize() failed\n");
        assert_true(false);
    }

    const net_status_t status = net_initialize(&net_callbacks);
    if (status != NET_SUCCESS) {
        PRINT_MSG("Error : net_initialize() failed (status %d)\n", status);
        assert_true(false);
    }

    bool app_send_data_request = true;
    while (!terminate) {
        if (mgt_process_request) {
            mgt_process_request = false;
            const mgt_status_t mgt_status = mgt_process();

            if (mgt_status != MGT_SUCCESS) {
                PRINT_MSG("Error processing SCHC packet (%d)", mgt_status);
                assert_true(false);
            }
        }
        platform_enter_low_power_ll();
    }

    l2_deinit();
}


int main(void) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_ahoi_l2),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

