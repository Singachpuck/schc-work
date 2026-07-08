#include <fullsdkfragapi.h>
#include <fullsdkmgt.h>
#include <fullsdknet.h>
#include <fullsdkl2.h>
#include <platform.h>

#include "fullsdkextapi.h"
#include "schc_al_params.h"
#include "schc_al.h"

#include "logging.h"

static const char *TAG = "MAIN";

// TODO: Consider buffer sizes
// Assume (MAX_PAYLOAD_SIZE + MGT_PROTO_SIZE) must be 4-bytes aligned
#define MAX_PAYLOAD_SIZE IPv6_MAX_PACKET_SIZE

// Memory block size provided to mgt_initialize.
#define MEM_BLOCK_SIZE                                                         \
    (L2_MAX_MTU * 4u + (MAX_PAYLOAD_SIZE + MGT_PROTO_SIZE) * 4u +              \
    MGT_SCHC_ACK_PACKET_SIZE * 3u + 64 * 2)

bool mgt_process_request = false;
static uint8_t mgt_mem_block[MEM_BLOCK_SIZE];

// static const char* ahoi_port = "/run/user/1000/slv_cons";
static const char* ahoi_port = "/run/user/1000/slv_triggered_prod";
// static const char* ahoi_port = "/dev/ttyUSB0";

static const uint8_t ahoi_id = 0x0A;

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
    LOGINFO(TAG, "MGT processing is required");
    mgt_process_request = true;
}

static void cb_mgt_connectivity_state(mgt_status_t status) {
    if (status != MGT_SUCCESS) {
        LOGERROR(TAG, "Connectivity KO (status %d)", status);
        return;
    }
    LOGINFO(TAG, "Connectivity OK");
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
    set_log_level(LOG_LEVEL_ALL);

    // SDK timers initialization.
    TimerInit(&sdk_timers[0], sdk_timer_1_event);
    TimerInit(&sdk_timers[1], sdk_timer_2_event);
    TimerInit(&sdk_timers[2], sdk_timer_3_event);

    l2_set_mtu(L2_MAX_MTU);
    l2_set_next_tx_delay(L2_TX_DELAY);

#ifdef L2_STACK_ahoi_posix_host
    l2_set_serial_port(ahoi_port);
    l2_set_iid(ahoi_id);
#endif

#ifdef L2_STACK_udp6
    // TODO:
#endif

    // Informs the SDK regarding the application mode in order to use the correct
    // fragmentation profile.
#ifdef SCHC_CORE_MODE
    LOGWARN(TAG, "SDK is APPLICATION!");
    sdk_mode = SDK_APP_MODE;

    net_set_host_static_ip(IPv6_PEER_PREFIX IPv6_PEER_IID);
    net_set_host_static_port("5683");
    net_set_remote_static_ip(IPv6_PREFIX IPv6_IID);
    net_set_remote_static_port("12345");
#endif
#ifdef SCHC_DEV_MODE
    LOGWARN(TAG, "SDK is DEVICE!");
    sdk_mode = SDK_DEVICE_MODE;

    net_set_host_static_ip(IPv6_PREFIX IPv6_IID);
    net_set_host_static_port("12345");
    net_set_remote_static_ip(IPv6_PEER_PREFIX IPv6_PEER_IID);
    net_set_remote_static_port("5683");
#endif

    mgt_status_t mgt_status = mgt_initialize(&mgt_callbacks, mgt_mem_block, MEM_BLOCK_SIZE, L2_MAX_MTU, MAX_PAYLOAD_SIZE);
    if (mgt_status != MGT_SUCCESS) {
        LOGERROR(TAG, "mgt_initialize() failed (error %d)", mgt_status);
        goto error;
    }

    net_callbacks = schc_al_get_net_callbacks();
    const net_status_t status = net_initialize(net_callbacks);
    if (status != NET_SUCCESS) {
        LOGERROR(TAG, "net_initialize() failed (status %d)", status);
        goto error;
    }

    if (schc_al_init() != 0) {
        LOGERROR(TAG, "schc_worker_init() failed");
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
                LOGERROR(TAG, "Error processing SCHC packet (status %d)", mgt_status);
            }
        }
        if (schc_al_is_processing_required()) {
            schc_al_process_status_t schc_al_status = schc_al_process();
            if (schc_al_status != SEND_DOWN_OK && schc_al_status != SEND_DOWN_BUSY) {
                LOGERROR(TAG, "schc_al_process critical error");
            }
        }
        fflush(stdout);
        platform_enter_low_power_ll();
    }
    printf("\n");
    LOGINFO(TAG, "Detected signal, terminating");

    terminate();
    return 0;

    error:
    terminate();
    return -1;
}

static void terminate() {
    LOGINFO(TAG, "Terminating the program");
    schc_al_terminate();
#ifdef L2_STACK_ahoi_posix_host
    l2_deinit();
#endif
}