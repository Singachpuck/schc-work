#include "fullsdkl2a.h"
#include "platform.h"
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>

#include <ahoi-module.h>

// L2 technology. It can be overriden by the application.
l2a_technology_t l2a_technology = L2A_DEFAULT;

// L2 MTU. It can be overriden by the application.
static uint16_t l2_mtu = AHOI_MAX_PAYLOAD_SIZE;

#define AHOI_MAX_ENCODED_SIZE ((127 + 6) * 2 + 4)

static const char* ahoi_port = "/dev/ttyUSB0";
static uint8_t iid = 1;

static l2a_callbacks_t l2a_cb;
static uint8_t *l2a_rx_buffer;
static uint16_t l2a_rx_buffer_size;
static int ahoifd;

static enum {
    NO_EVENT = 0,
    CONNECTIVITY_AVAILABLE_EVENT = 1 << 0,
    TRANSMISSION_RESULT_EVENT = 1 << 1,
    DOWNLINK_AVAILABLE_EVENT = 1 << 2,
} event;

static uint8_t phy_rx_buffer[AHOI_MAX_ENCODED_SIZE];

void l2_set_mtu(uint16_t mtu)
{
    l2_mtu = mtu;
}

void l2_set_serial_port(const char* port) {
    ahoi_port = port;
}

void l2_set_iid(uint8_t val) {
    iid = val;
}

static void _downlink_available_callback(void)
{
    event |= DOWNLINK_AVAILABLE_EVENT;
    l2a_cb.processing_required();
}

l2a_status_t l2a_initialize(const l2a_callbacks_t *pp_callbacks,
                            uint8_t *p_receive_buffer,
                            uint16_t receive_buffer_size)
{
    printf("l2a>l2a_initialized() called\n");
    l2a_cb = *pp_callbacks;

    ahoi_init_t ahoi_init = {
            .open_addr = ahoi_port,
            .baud = B115200,
            .id = iid,
            .recv_buf = phy_rx_buffer,
            .recv_buf_len = AHOI_MAX_ENCODED_SIZE
    };
    l2a_rx_buffer = p_receive_buffer;
    l2a_rx_buffer_size = receive_buffer_size;

    event = NO_EVENT;

    if (ahoi_connect(&ahoi_init, 1, &ahoifd) < 0) {
        return L2A_CONNECT_ERR;
    }

    // Indicate that the connectivity is available
    event |= CONNECTIVITY_AVAILABLE_EVENT;
    l2a_cb.processing_required();

    if (!watch_fd_for_input(ahoifd, &_downlink_available_callback))
    {
        printf("l2a>watch_fd_for_input() failed\n");
        return L2A_CONNECT_ERR;
    }
    return L2A_SUCCESS;
}

l2a_status_t l2a_send_data(const uint8_t *p_data, uint16_t data_size)
{
    int ret;

    printf("l2a>l2a_send_data() called\n");
    PRINT_HEX_BUF(p_data, data_size);

    ahoi_header_t hdr = {
        .dst = 0xFF,
        .type = 0x01,
        .flags = AHOI_NO_FLAGS,
        .seq = 0,
        .pl_size = data_size
    };

    do {
        ret = ahoi_write_hdr_pld(ahoifd, &hdr, p_data);
    } while (ret == -1 && errno == EINTR);

    if (ret == -1) {
        return L2A_L2_ERROR;
    }

    event |= TRANSMISSION_RESULT_EVENT;
    l2a_cb.processing_required();

    return L2A_SUCCESS;
}

l2a_technology_t l2a_get_technology(void)
{
    return l2a_technology;
}

uint16_t l2a_get_mtu(void)
{
    printf("l2a>l2a_get_mtu() called\n");

    return l2_mtu;
}

uint32_t l2a_get_next_tx_delay(uint16_t data_size)
{
    (void)data_size;
    printf("l2a>l2a_get_next_tx_delay() called\n");

    // Return 500ms delay.
    return 500;
}

consumer_status_t packet_consumer(ahoi_packet_t* pkt) {
    consumer_status_t ret = CONSUMER_OK;

//    uint16_t pkt_size = AHOI_HEADER_SIZE + pkt->pl_size;
    if (pkt->pl_size > l2a_rx_buffer_size) {
        l2a_cb.data_received(0, L2A_L2_ERROR);
        goto finish;
    }
    memcpy(l2a_rx_buffer, pkt->payload, pkt->pl_size);

    printf("l2a>rx packet received\n");
    PRINT_HEX_BUF(l2a_rx_buffer, pkt->pl_size);
    l2a_cb.data_received(pkt->pl_size, L2A_SUCCESS);

    finish:
    return ret;
}

static void _read_downlink(void)
{
    rx_status_t ret;

    do {
        ret = ahoi_stateful_read(ahoifd, packet_consumer);
    } while (ret != READ_OK && errno == EINTR);

    if (ret != READ_OK) {
        l2a_cb.data_received(0, L2A_L2_ERROR);
    }
}

l2a_status_t l2a_process(void)
{
    printf("l2a>l2a_process() called\n");
    if (event & CONNECTIVITY_AVAILABLE_EVENT)
        l2a_cb.connectivity_available();
    if (event & TRANSMISSION_RESULT_EVENT)
        l2a_cb.transmission_result(L2A_SUCCESS, 0);
    if (event & DOWNLINK_AVAILABLE_EVENT)
        _read_downlink();
    event = NO_EVENT;
    return L2A_SUCCESS;
}

bool l2a_get_dev_iid(uint8_t **dev_iid)
{
    (void)dev_iid;
    return false;
}

void l2_deinit() {
    if (ahoifd >= 0) {
        ahoi_disconnect(ahoifd);
    }
}