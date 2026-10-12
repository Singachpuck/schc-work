#include "fullsdkfragapi.h"
#include "profile.h"

// Fragmentation rule ID
// #define DEVICE_APP_RULE_ID 20
// #define APP_DEVICE_RULE_ID 21

sdk_mode_t sdk_mode = SDK_DEVICE_MODE;

#define MAX_ACK_REQ 3
#define SCHC_ACK_REQ_DN_OPPORTUNITY 2
#define INACTIVITY_TIMER_EXP 600000
#define IDLE_POLLING_TIMER_EXP ((INACTIVITY_TIMER_EXP * 3) / 4)
#define RETRANSMISSION_TIMER_EXP_DOWN (INACTIVITY_TIMER_EXP / (MAX_ACK_REQ + 1))
#define SCHC_ACK_REQ_DN_OPPORTUNITY 2
#define ACK_RETRANSMISSION_TIMER_EXP                                           \
(RETRANSMISSION_TIMER_EXP_DOWN / (MAX_ACK_REQ * SCHC_ACK_REQ_DN_OPPORTUNITY))

static frag_profile_t aa_down_profile = {
  .mode = FRAG_MODE_AA,
  .direction = PROFILE_DIR_DOWN,
  .rule_id = AA_DOWNLINK_FRAG_RULE_ID + 1,
  .rule_id_size_bits = 8,
  .dtag_size_bits = 0,
  .l2_word_size_bits = 8,
  .n = 1,
  .rcs_size_bytes = 4,
  .calc_rcs = profile_compute_rcs,
  .polling_enabled = false,
  .aa.max_ack_requests = MAX_ACK_REQ,
  .aa.window_size = 1,
  .aa.rt_exp_time = 0, // Not used.
  .aa.ack_req_dn_opportunity = SCHC_ACK_REQ_DN_OPPORTUNITY,
  .aa.ack_rt_exp_time = ACK_RETRANSMISSION_TIMER_EXP,
  .aa.idle_polling_time = IDLE_POLLING_TIMER_EXP,
  .it_exp_time = INACTIVITY_TIMER_EXP,
  // .padding_bits_values.frag_padding_bit = 0,
  // .padding_bits_values.byte_padding_bit = 0
};

static frag_profile_t aa_up_profile = {
  .mode = FRAG_MODE_AA,
  .direction = PROFILE_DIR_DOWN,
  .rule_id = AA_DOWNLINK_FRAG_RULE_ID,
  .rule_id_size_bits = 8,
  .dtag_size_bits = 0,
  .l2_word_size_bits = 8,
  .n = 1,
  .rcs_size_bytes = 4,
  .calc_rcs = profile_compute_rcs,
  .polling_enabled = false,
  .aa.max_ack_requests = MAX_ACK_REQ,
  .aa.window_size = 1,
  .aa.rt_exp_time = 0, // Not used.
  .aa.ack_req_dn_opportunity = SCHC_ACK_REQ_DN_OPPORTUNITY,
  .aa.ack_rt_exp_time = ACK_RETRANSMISSION_TIMER_EXP,
  .aa.idle_polling_time = IDLE_POLLING_TIMER_EXP,
  .it_exp_time = INACTIVITY_TIMER_EXP,
};

static frag_profiles_t *get_point_to_point_profiles(void)
{
  if (sdk_mode == SDK_DEVICE_MODE)
  {
    // In SDK_DEVICE_MODE:
    // - profile_1 is considered as uplink profile
    // - profile_2 is considered as downlink profile
    aa_up_profile.direction = PROFILE_DIR_UP;
    aa_down_profile.direction = PROFILE_DIR_DOWN;
  }
  else
  {
    // In SDK_APP_MODE:
    // - profile_1 is considered as downlink profile
    // - profile_2 is considered as uplink profile
    aa_up_profile.direction = PROFILE_DIR_DOWN;
    aa_down_profile.direction = PROFILE_DIR_UP;
  }

  static frag_profile_t *profiles_list[2];
  static frag_profiles_t profiles;

  init_profiles(&profiles, profiles_list);
  add_profile(&profiles, &aa_up_profile);
  add_profile(&profiles, &aa_down_profile);

  return &profiles;
}

frag_profiles_t *mgt_load_profiles(void)
{
  return get_point_to_point_profiles();
}
