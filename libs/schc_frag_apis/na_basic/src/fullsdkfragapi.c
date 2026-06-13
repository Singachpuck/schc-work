#include "fullsdkfragapi.h"
#include "profile.h"

// Fragmentation rule ID
// #define DEVICE_APP_RULE_ID 20
// #define APP_DEVICE_RULE_ID 21

static frag_profile_t up_noack_profile = {
  .mode = FRAG_MODE_NA,
  .direction = PROFILE_DIR_UP,
  .rule_id = DEFAULT_NA_DOWNLINK_FRAG_RULE_ID + 1,
  .rule_id_size_bits = 7,
  .dtag_size_bits = 0,
  .l2_word_size_bits = 8,
  .n = 1,
  .it_exp_time = 180000, // Arbitrary value.
  .rcs_size_bytes = 4,
  .calc_rcs = profile_compute_rcs,
  .padding_bits_values.frag_padding_bit = 0,
  .padding_bits_values.byte_padding_bit = 0,
  .polling_enabled = false,
  .na.max_polling_frame = 2
};

static frag_profile_t down_noack_profile = {
  .mode = FRAG_MODE_NA,
  .direction = PROFILE_DIR_DOWN,
  .rule_id = DEFAULT_NA_DOWNLINK_FRAG_RULE_ID,
  .rule_id_size_bits = 7,
  .dtag_size_bits = 0,
  .l2_word_size_bits = 8,
  .n = 1,
  .it_exp_time = 180000, // Arbitrary value.
  .rcs_size_bytes = 4,
  .calc_rcs = profile_compute_rcs,
  .padding_bits_values.frag_padding_bit = 0,
  .padding_bits_values.byte_padding_bit = 0,
  .polling_enabled = false,
  .na.max_polling_frame = 2
};

static frag_profiles_t *get_point_to_point_profiles(void)
{
  static frag_profile_t *profiles_list[2];
  static frag_profiles_t profiles;

  init_profiles(&profiles, profiles_list);
  add_profile(&profiles, &up_noack_profile);
  add_profile(&profiles, &down_noack_profile);

  return &profiles;
}

frag_profiles_t *mgt_load_profiles(void)
{
  return get_point_to_point_profiles();
}
