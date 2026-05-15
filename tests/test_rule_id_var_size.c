#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <cmocka.h>

#include "decompressor.h"
#include "schccomp.h"

#include "test_helpers/schc_mocks.h"

#ifndef NO_COMP_RULE_ID
#define NO_COMP_RULE_ID 150
#endif

#define TEST_INPUT_SIZE 4
#define TEST_BUFFER_SIZE 64

#ifndef RULE_ID_BITS
#define RULE_ID_BITS 4
#endif

typedef struct {
    rule_field_t bytes_field;
    rule_field_t *rule_fields[1];
    rule_t rule;
    rule_t *rule_array[1];
    rules_t rules;
    comp_callbacks_t callbacks;
    uint8_t input[TEST_INPUT_SIZE];
    uint8_t compressed[TEST_BUFFER_SIZE];
    uint8_t decompressed[TEST_BUFFER_SIZE];
} rule_id_fixture_t;

static uint16_t expected_rule_id(void) {
#if RULE_ID_BITS == 12
    return 0x0ABC;
#elif RULE_ID_BITS == 5
    return 0x001B;
#elif RULE_ID_BITS == 16
    return 0xA55A;
#else
    return (uint16_t)((1u << RULE_ID_BITS) - 1u);
#endif
}

static uint16_t read_bits_be(const uint8_t *buffer, uint8_t nb_bits) {
    uint16_t value = 0;
    for (uint8_t i = 0; i < nb_bits; i++) {
        value = (uint16_t) ((value << 1) | get_bit_at(buffer, i));
    }
    return value;
}

static int fixture_setup(void **state) {
    static rule_id_fixture_t fixture;
    memset(&fixture, 0, sizeof(fixture));

    static uint8_t header_bytes[] = {0x11, 0x22, 0x33, 0x44};
    static const uint16_t header_bits = sizeof(header_bytes) * CHAR_BIT;
    static target_value_t header_tv = {
            TV_BIT_STRING, {{header_bytes, 0, header_bits}}
    };

    fixture.bytes_field = (rule_field_t) {FID_BYTES, 1, DIR_BI, &header_tv, header_bits,
                                          MO_EQUAL, {0}, CDA_NOT_SENT};

    init_rule(&fixture.rule, expected_rule_id(), STACK_NONE, fixture.rule_fields);
    add_rule_field(&fixture.rule, &fixture.bytes_field);

    init_rules(&fixture.rules, fixture.rule_array, NO_COMP_RULE_ID);
    add_rule(&fixture.rules, &fixture.rule);

    fixture.callbacks = comp_mocked_cb;

    memcpy(fixture.input, header_bytes, 4);

    *state = &fixture;

    return 0;
}

static void test_rule_id_round_trip(void **state) {
    rule_id_fixture_t *fixture = *state;
    uint16_t comp_size_bits = 0;
    uint16_t decomp_size = 0;
    uint16_t expected_id = expected_rule_id();

    assert_int_equal(
            schc_compress(&fixture->rules, fixture->compressed, TEST_BUFFER_SIZE,
                          &comp_size_bits, fixture->input, TEST_INPUT_SIZE,
                          &fixture->callbacks),
            COMP_SUCCESS);
    assert_int_equal(comp_size_bits, RULE_ID_BITS);
    assert_int_equal(read_bits_be(fixture->compressed, RULE_ID_BITS), expected_id);

    memset(fixture->decompressed, 0, sizeof(fixture->decompressed));
    assert_int_equal(
            schc_decompress(&fixture->rules, fixture->decompressed,
                            TEST_BUFFER_SIZE, &decomp_size, fixture->compressed,
                            comp_size_bits, &fixture->callbacks),
            COMP_SUCCESS);
    assert_int_equal(decomp_size, TEST_INPUT_SIZE);
    assert_memory_equal(fixture->decompressed, fixture->input, TEST_INPUT_SIZE);
}

int main(void) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test_setup(test_rule_id_round_trip, fixture_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}


