#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// SCHC
#include <utils.h>
#include <parser.h>
#include <schccomp.h>

extern comp_callbacks_t comp_mocked_cb;

bool mocked_ext_compress(bit_buffer_t *output_bb_ptr, bit_string_t *input_bs_ptr);

bool mocked_ext_decompress(bit_buffer_t *p_out_data, bit_buffer_t *p_in_data);