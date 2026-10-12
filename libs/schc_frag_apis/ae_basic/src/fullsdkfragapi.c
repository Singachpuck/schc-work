#include "fullsdkfragapi.h"
#include "profile.h"

// Fragmentation rule ID
// #define DEVICE_APP_RULE_ID 20
// #define APP_DEVICE_RULE_ID 21

sdk_mode_t sdk_mode = SDK_DEVICE_MODE;

// Default.
static frag_profile_t up_aoe_profile = {
  .mode = FRAG_MODE_AOE,
  .direction = PROFILE_DIR_UP,
  .rule_id = AOE_UPLINK_FRAG_RULE_ID,
  .rule_id_size_bits = 8,
  .dtag_size_bits = 2,
  .l2_word_size_bits = 8,
  .ae.tile_size_bits = 32,
  .ae.m = 2,
  .n = 6,
  .ae.window_size = 63,
  .rcs_size_bytes = 4,
  .calc_rcs = profile_compute_rcs,
  .ae.max_ack_requests = 3,
  .ae.rt_exp_time = 0, // Retransmission timer computed dynarcsally.
  .it_exp_time = 600000,
  .ae.no_tile_in_all1_flag = 1,
  .ae.one_l2_word_smaller_flag = 0,
  .ae.resend_contiguous_tiles_flag = 1,
  // .padding_bits_values.frag_padding_bit = 0,
  // .padding_bits_values.byte_padding_bit = 0
};

static frag_profile_t down_aoe_profile = {
  .mode = FRAG_MODE_AOE,
  .direction = PROFILE_DIR_DOWN,
  .rule_id = AOE_DOWNLINK_FRAG_RULE_ID,
  .rule_id_size_bits = 8,
  .dtag_size_bits = 2,
  .l2_word_size_bits = 8,
  .ae.tile_size_bits = 32,
  .ae.m = 2,
  .n = 6,
  .ae.window_size = 63,
  .rcs_size_bytes = 4,
  .calc_rcs = profile_compute_rcs,
  .ae.max_ack_requests = 3,
  .ae.rt_exp_time = 0, // Not used.
  .it_exp_time = 600000,
  .ae.no_tile_in_all1_flag = 1,
  .ae.one_l2_word_smaller_flag = 0,
  .ae.resend_contiguous_tiles_flag = 1,
  // .padding_bits_values.frag_padding_bit = 0,
  // .padding_bits_values.byte_padding_bit = 0
};

static frag_profiles_t *get_point_to_point_profiles(void)
{
  if (sdk_mode == SDK_DEVICE_MODE)
  {
    // In SDK_DEVICE_MODE:
    // - profile_1 is considered as uplink profile
    // - profile_2 is considered as downlink profile
    up_aoe_profile.direction = PROFILE_DIR_UP;
    down_aoe_profile.direction = PROFILE_DIR_DOWN;
  }
  else
  {
    // In SDK_APP_MODE:
    // - profile_1 is considered as downlink profile
    // - profile_2 is considered as uplink profile
    up_aoe_profile.direction = PROFILE_DIR_DOWN;
    down_aoe_profile.direction = PROFILE_DIR_UP;
  }

  static frag_profile_t *profiles_list[2];
  static frag_profiles_t profiles;

  init_profiles(&profiles, profiles_list);
  add_profile(&profiles, &up_aoe_profile);
  add_profile(&profiles, &down_aoe_profile);

  return &profiles;
}

frag_profiles_t *mgt_load_profiles(void)
{
  return get_point_to_point_profiles();
}
