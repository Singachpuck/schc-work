#pragma once

#include <stdint.h>
#include <stdbool.h>

void l2_ahoi_set_mtu(uint16_t mtu);

uint16_t l2_ahoi_get_mtu(void);


l2_status_t l2_get_dutycycle(bool *value);

l2_status_t l2_set_dutycycle(bool dutycycle_on);

/**
 * @brief This function initializes L2A layer.
 *
 * All mandatory callbacks must be provided. An optional callback set to NULL
 * means that the callback is not available.
 *
 * @note Implementation is mandatory.
 *
 * @param[in] pp_callbacks callbacks
 * @param[in] p_receive_buffer buffer allocated by the client code used to store
 * received data
 * @param[in] receive_buffer_size size of the receive buffer
 *
 * @return status of the initialization:
 *   - L2A_SUCCESS: success
 *   - L2A_REQ_CALLBACK_ERR: a mandatory callback is not defined
 */
l2a_status_t l2_ahoi_initialize(const l2a_callbacks_t *pp_callbacks,
                            uint8_t *p_receive_buffer,
                            uint16_t receive_buffer_size);

l2a_status_t l2a_send_data(const uint8_t *p_data, uint16_t data_size);

bool l2a_get_dev_iid(uint8_t **dev_iid);

uint32_t l2a_get_next_tx_delay(uint16_t data_size);

