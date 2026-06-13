#include <fullsdkfragapi.h>
#include <fullsdkmgt.h>
#include <fullsdknet.h>
#include <fullsdkl2.h>
#include <platform.h>

#include "schc_al.h"

// TODO: Consider buffer sizes
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
static uint8_t mgt_mem_block[MEM_BLOCK_SIZE];

//static const char* ahoi_port = "/run/user/1000/slv_cons";
static const char* ahoi_port = "/run/user/1000/slv_triggered_prod";

static TimerEvent_t sdk_timers[3];
static const net_callbacks_t *net_callbacks;

static volatile sig_atomic_t terminate_flag = 0;

static void sigint_handler(int signum)
{
    (void)signum;  /* Unused parameter */
    terminate_flag = 1;
}

static void terminate();

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

int main() {

    // SDK timers initialization.
    TimerInit(&sdk_timers[0], sdk_timer_1_event);
    TimerInit(&sdk_timers[1], sdk_timer_2_event);
    TimerInit(&sdk_timers[2], sdk_timer_3_event);

    l2_set_mtu(20);
    l2_set_serial_port(ahoi_port);
    l2_set_iid(0x0A);

    // Informs the SDK regarding the application mode in order to use the correct
    // fragmentation profile.
    // mgt_set_mode(SDK_DEVICE_MODE);

    if (mgt_initialize(&mgt_callbacks, mgt_mem_block, MEM_BLOCK_SIZE,
                       MAX_MTU_SIZE, MAX_PAYLOAD_SIZE) != MGT_SUCCESS) {
        PRINT_MSG("Error : mgt_initialize() failed\n");
        goto error;
    }

    net_callbacks = schc_al_get_net_callbacks();
    const net_status_t status = net_initialize(net_callbacks);
    if (status != NET_SUCCESS) {
        PRINT_MSG("Error : net_initialize() failed (status %d)\n", status);
        goto error;
    }

    if (schc_al_init() != 0) {
        PRINT_MSG("Error : schc_worker_init() failed\n");
        goto error;
    }

    struct sigaction sa = {0};
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1)
    {
        perror("sigaction");
        goto error;
    }

    while (!terminate_flag) {
        if (mgt_process_request) {
            mgt_process_request = false;
            const mgt_status_t mgt_status = mgt_process();

            if (mgt_status != MGT_SUCCESS) {
                PRINT_MSG("Error processing SCHC packet (%d)", mgt_status);
            }
        }
        if (schc_al_is_processing_required()) {
            schc_al_process_status_t schc_al_status = schc_al_process();
            if (schc_al_status != SEND_DOWN_OK && schc_al_status != SEND_DOWN_BUSY) {
                PRINT_MSG("schc_al>critical error\n");
            }
        }
        platform_enter_low_power_ll();
    }

    terminate();
    return 0;

    error:
    terminate();
    return -1;
}

static void terminate() {
    PRINT_MSG("schc_al_main>Terminating the program\n");
    schc_al_terminate();
    l2_deinit();
}