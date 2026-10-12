#include "logging.h"

static log_level_t log_level = LOG_LEVEL_NONE;

log_level_t get_log_level() {
  return log_level;
}

void set_log_level(log_level_t level) {
  log_level = level;
}