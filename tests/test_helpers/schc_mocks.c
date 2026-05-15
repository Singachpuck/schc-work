#include "schc_mocks.h"

#include <fullsdkl2a.h>

bool l2a_get_dev_iid(uint8_t **dev_iid) {
    (void)dev_iid;
    return false;
}

 l2a_technology_t l2a_get_technology(void) {
    return L2A_DEFAULT;
}

uint16_t l2a_get_mtu(void) {
    return 0;
}

l2a_status_t l2a_send_data(const uint8_t *p_data, uint16_t data_size) {
    (void)p_data;
    (void)data_size;
    return L2A_SUCCESS;
}

uint32_t l2a_get_next_tx_delay(uint16_t data_size) {
    (void)data_size;
    return 0;
}

l2a_status_t l2a_process(void) {
    return L2A_SUCCESS;
}

bool mocked_ext_compress(bit_buffer_t *output_bb_ptr, bit_string_t *input_bs_ptr) {
    (void)output_bb_ptr;
    (void)input_bs_ptr;
    return true;
}

bool mocked_ext_decompress(bit_buffer_t *p_out_data, bit_buffer_t *p_in_data) {
    (void)p_out_data;
    (void)p_in_data;
    return true;
}

comp_callbacks_t comp_mocked_cb = {
        .get_dev_iid = l2a_get_dev_iid,
        .ext_compress = mocked_ext_compress,
        .ext_decompress = mocked_ext_decompress
};

static bool copy_payload_compress(bit_buffer_t *output_bb_ptr,
                                  bit_string_t *input_bs_ptr) {
    move_n_bits(output_bb_ptr->buffer_ptr + (output_bb_ptr->put_bit_index / CHAR_BIT),
                output_bb_ptr->put_bit_index % CHAR_BIT, input_bs_ptr->value_ptr,
                input_bs_ptr->start_bit_index, input_bs_ptr->value_length_bits);
    output_bb_ptr->put_bit_index += input_bs_ptr->value_length_bits;
    return true;
}

static bool copy_payload_decompress(bit_buffer_t *output_bb_ptr,
                                    bit_buffer_t *input_bb_ptr) {
    uint16_t nb_bits = input_bb_ptr->last_bit_index - input_bb_ptr->get_bit_index + 1;
    move_n_bits(output_bb_ptr->buffer_ptr + (output_bb_ptr->put_bit_index / CHAR_BIT),
                output_bb_ptr->put_bit_index % CHAR_BIT, input_bb_ptr->buffer_ptr,
                input_bb_ptr->get_bit_index, nb_bits);
    output_bb_ptr->put_bit_index += nb_bits;
    input_bb_ptr->get_bit_index += nb_bits;
    return true;
}