#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <cmocka.h>

//#include "definitions.h"
//#include "helpers/log_helper.h"
#include "test_helpers/schc_mocks.h"
#include "test_helpers/packet_builders.h"
#include "schccomp.h"
#include "template.h"

static const uint8_t dev_ip[IPv6_ADDR_LEN] = {
	0x20, 0x01, 0x0d, 0xb8, 0x00, 0x03, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};
static const uint8_t app_ip[IPv6_ADDR_LEN] = {
	0x20, 0x01, 0x0d, 0xb8, 0x00, 0x03, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02
};
static const uint8_t dev_port[UDP_PORT_LEN] = {0x11, 0x11};
static const uint8_t app_port[UDP_PORT_LEN] = {0x16, 0x33};
static const char sensor_id[] = "00";

static int group_setup(void **state) {
	(void)state;
	return logger_init() == LOGGER_INIT_OK ? 0 : -1;
}

static int group_teardown(void **state) {
	(void)state;
	return logger_deinit() == LOGGER_INIT_OK ? 0 : -1;
}

static void init_template(void) {
	template_t *tpl = tpl_init_template();
	assert_non_null(tpl);
	assert_int_equal(tpl_set_index_param_value(0, dev_ip, IPv6_NET_PREFIX_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(1, dev_ip + IPv6_NET_PREFIX_LEN, IPv6_IID_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(2, app_ip, IPv6_NET_PREFIX_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(3, app_ip + IPv6_NET_PREFIX_LEN, IPv6_IID_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(4, dev_port, UDP_PORT_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(5, app_port, UDP_PORT_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(6, (const uint8_t *)sensor_id, (uint16_t)strlen(sensor_id)), TPL_SUCCESS);
	assert_true(tpl_is_configured());
}

static void init_template_with_params(const uint8_t *pull_stat_dev_ip,
                                      const uint8_t *pull_stat_app_ip,
                                      const uint8_t *pull_stat_dev_port,
                                      const uint8_t *pull_stat_app_port,
                                      const char *pull_stat_sensor_id) {
	template_t *tpl = tpl_init_template();
	assert_non_null(tpl);
	assert_int_equal(tpl_set_index_param_value(0, pull_stat_dev_ip, IPv6_NET_PREFIX_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(1, pull_stat_dev_ip + IPv6_NET_PREFIX_LEN, IPv6_IID_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(2, pull_stat_app_ip, IPv6_NET_PREFIX_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(3, pull_stat_app_ip + IPv6_NET_PREFIX_LEN, IPv6_IID_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(4, pull_stat_dev_port, UDP_PORT_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(5, pull_stat_app_port, UDP_PORT_LEN), TPL_SUCCESS);
	assert_int_equal(tpl_set_index_param_value(6, (const uint8_t *)pull_stat_sensor_id,
												(uint16_t)strlen(pull_stat_sensor_id)),
											 TPL_SUCCESS);
	assert_true(tpl_is_configured());
}

static void run_round_trip_case(const uint8_t src_ip[IPv6_ADDR_SIZE],
								const uint8_t dst_ip[IPv6_ADDR_SIZE],
								uint16_t src_port,
								uint16_t dst_port,
								uint8_t coap_type,
								uint8_t coap_code,
								const char *const paths[],
								size_t path_count,
								const uint8_t *payload,
								size_t payload_len) {
	rules_t *rules = tpl_get_template_rules();
	assert_non_null(rules);

	uint8_t original_packet[SCHC_DECOMP_BUF_SIZE] = {0};
	uint8_t expected_packet[SCHC_DECOMP_BUF_SIZE] = {0};
	uint8_t decomp_buf[SCHC_DECOMP_BUF_SIZE] = {0};
	uint8_t comp_buf[SCHC_COMP_BUF_SIZE] = {0};
	uint8_t coap_buf[SCHC_DECOMP_BUF_SIZE] = {0};
	uint16_t comp_size_bits = 0;
	uint16_t decomp_size_bytes = 0;

	size_t coap_len = build_coap_message(coap_buf,
											coap_type,
											coap_code,
											paths,
											path_count,
											payload,
											payload_len);
	size_t original_len = build_ipv6_udp_packet(original_packet, src_ip, dst_ip, src_port, dst_port,
											coap_buf, coap_len);
	size_t expected_len = build_ipv6_udp_packet(expected_packet, dst_ip, src_ip, dst_port, src_port,
											coap_buf, coap_len);
	assert_true(original_len <= UINT16_MAX);
	assert_int_equal(original_len, expected_len);

	hzlog_debug(ok_cat, original_packet, original_len);

	assert_int_equal(schc_compress(rules, comp_buf, sizeof(comp_buf), &comp_size_bits,
								  original_packet, (uint16_t)original_len, &comp_mocked_cb),
					 COMP_SUCCESS);
	hzlog_debug(ok_cat, comp_buf, (comp_size_bits + 7u) / 8u);

	assert_int_equal(schc_decompress(rules, decomp_buf, sizeof(decomp_buf), &decomp_size_bytes,
								 comp_buf, comp_size_bits, &comp_mocked_cb),
					 COMP_SUCCESS);
	hzlog_debug(ok_cat, decomp_buf, decomp_size_bytes);

	assert_int_equal(decomp_size_bytes, expected_len);
	assert_memory_equal(decomp_buf, expected_packet, expected_len);
}

static void test_push_temp_roundtrip(void **state) {
	(void)state;
	init_template();

	static const char *const paths[] = {"sensors", sensor_id, "t"};
	run_round_trip_case(app_ip, dev_ip,
						(uint16_t)((app_port[0] << 8) | app_port[1]),
						(uint16_t)((dev_port[0] << 8) | dev_port[1]),
						COAP_NON_TYPE, COAP_POST_CODE,
						paths, sizeof(paths) / sizeof(paths[0]),
						NULL, 0);
}

static void test_push_status_roundtrip(void **state) {
	(void)state;
	init_template();

	static const char *const paths[] = {"sensors", sensor_id, "status"};
	run_round_trip_case(app_ip, dev_ip,
						(uint16_t)((app_port[0] << 8) | app_port[1]),
						(uint16_t)((dev_port[0] << 8) | dev_port[1]),
						COAP_NON_TYPE, COAP_POST_CODE,
						paths, sizeof(paths) / sizeof(paths[0]),
						NULL, 0);
}

static void test_pull_stat_roundtrip(void **state) {
	(void)state;
	init_template();


	static const char *const paths[] = {"sensors", sensor_id, "stat"};
	run_round_trip_case(app_ip, dev_ip,
                        (uint16_t)((app_port[0] << 8) | app_port[1]),
                        (uint16_t)((dev_port[0] << 8) | dev_port[1]),
						COAP_CON_TYPE, COAP_GET_CODE,
						paths, sizeof(paths) / sizeof(paths[0]),
						NULL, 0);
}

static void test_pull_temp_roundtrip(void **state) {
	(void)state;
	init_template();

	static const char *const paths[] = {"sensors", sensor_id, "t"};
	run_round_trip_case(app_ip, dev_ip,
                        (uint16_t)((app_port[0] << 8) | app_port[1]),
                        (uint16_t)((dev_port[0] << 8) | dev_port[1]),
						COAP_CON_TYPE, COAP_GET_CODE,
						paths, sizeof(paths) / sizeof(paths[0]),
						NULL, 0);
}

static void test_pull_state_roundtrip(void **state) {
	(void)state;
	init_template();

	static const char *const paths[] = {"sensors", sensor_id, "status"};
	run_round_trip_case(app_ip, dev_ip,
                        (uint16_t)((app_port[0] << 8) | app_port[1]),
                        (uint16_t)((dev_port[0] << 8) | dev_port[1]),
						COAP_CON_TYPE, COAP_GET_CODE,
						paths, sizeof(paths) / sizeof(paths[0]),
						NULL, 0);
}

static void test_pull_resp_roundtrip(void **state) {
	(void)state;
	init_template();

    static const uint8_t payload[] = {'2', '5', '.', '4'};
    run_round_trip_case(app_ip, dev_ip,
                        (uint16_t)((app_port[0] << 8) | app_port[1]),
                        (uint16_t)((dev_port[0] << 8) | dev_port[1]),
                        COAP_ACK_TYPE, COAP_CONTENT_CODE,
                        NULL, 0,
                        payload, sizeof(payload));
}

static void test_pull_stat_real(void** state) {
    (void)state;
	static const uint8_t pull_stat_dev_ip[IPv6_ADDR_SIZE] = {
		0x20, 0x01, 0x0d, 0xb8, 0x00, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
	};
	static const uint8_t pull_stat_app_ip[IPv6_ADDR_SIZE] = {
		0x20, 0x01, 0xdb, 0x10, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10
	};
	static const uint8_t pull_stat_dev_port[UDP_PORT_LEN] = {0x16, 0x33};
	static const uint8_t pull_stat_app_port[UDP_PORT_LEN] = {0x30, 0x39};
	static const char pull_stat_sensor_id[] = "01";
    init_template_with_params(pull_stat_dev_ip, pull_stat_app_ip,
                              pull_stat_dev_port, pull_stat_app_port,
                              pull_stat_sensor_id);

    /*
             0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF
    0000000001   60 00 00 00 00 1e 11 3f 20 01 db 10 00 00 00 00   `......? .......
    0000000002   00 00 00 00 00 00 00 10 20 01 0d b8 00 01 00 00   ........ .......
    0000000003   00 00 00 00 00 00 00 01 30 39 16 33 00 1e bd dc   ........09.3....
    0000000004   40 01 12 34 b7 73 65 6e 73 6f 72 73 02 30 31 06   @..4.sensors.01.
    0000000005   73 74 61 74 75 73                                 status
    */
	static const uint8_t packet[] = {
		0x60, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x11, 0x3f,
		0x20, 0x01, 0xdb, 0x10, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
		0x20, 0x01, 0x0d, 0xb8, 0x00, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
		0x30, 0x39, 0x16, 0x33, 0x00, 0x1e, 0xbd, 0xdc,
		0x40, 0x01, 0x12, 0x34, 0xb7, 0x73, 0x65, 0x6e,
		0x73, 0x6f, 0x72, 0x73, 0x02, 0x30, 0x31, 0x06,
		0x73, 0x74, 0x61, 0x74, 0x75, 0x73
	};

	rules_t *rules = tpl_get_template_rules();
	assert_non_null(rules);

	uint8_t comp_buf[SCHC_COMP_BUF_SIZE] = {0};
	uint8_t decomp_buf[SCHC_DECOMP_BUF_SIZE] = {0};
	uint16_t comp_size_bits = 0;
	uint16_t decomp_size_bytes = 0;
	const size_t packet_len = sizeof(packet);

	hzlog_debug(ok_cat, packet, packet_len);

	assert_int_equal(schc_compress(rules, comp_buf, sizeof(comp_buf), &comp_size_bits,
								   (uint8_t *)packet, (uint16_t)packet_len, &comp_mocked_cb),
				   COMP_SUCCESS);
	hzlog_debug(ok_cat, comp_buf, (comp_size_bits + 7u) / 8u);
	assert_true((size_t)((comp_size_bits + 7u) / 8u) < packet_len);

//	assert_int_equal(schc_decompress(rules, decomp_buf, sizeof(decomp_buf),
//									 &decomp_size_bytes, comp_buf, comp_size_bits,
//									 &comp_mocked_cb),
//				   COMP_SUCCESS);
//	hzlog_debug(ok_cat, decomp_buf, decomp_size_bytes);
//
//	assert_int_equal(decomp_size_bytes, packet_len);
//	assert_memory_equal(decomp_buf, packet, packet_len);

    // Based on the diag output of the received coap message set accordingly the template params, execute the schc compression and make sure that the compression happened
}

int main(void) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_push_temp_roundtrip),
		cmocka_unit_test(test_push_status_roundtrip),
		cmocka_unit_test(test_pull_stat_roundtrip),
		cmocka_unit_test(test_pull_stat_real),
		cmocka_unit_test(test_pull_temp_roundtrip),
		cmocka_unit_test(test_pull_state_roundtrip),
		cmocka_unit_test(test_pull_resp_roundtrip),
	};

	return cmocka_run_group_tests(tests, group_setup, group_teardown);
}


