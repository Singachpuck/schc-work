/**
 * @copyright
 * Copyright (c) 2018-2023 ACKLIO SAS
 * Copyright (c) 2024 ACTILITY SA - All Rights Reserved
 * 
 * This file is part of lab.SCHC FullSDK.
 * 
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * 
 * Author: dancho.iliev@ackl.io
 */

#ifndef APP_INC_PLATFORM_H_
#define APP_INC_PLATFORM_H_

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define PRINT_MSG printf
#define PRINT_HEX_BUF(buf, size) platform_print_hex_buffer(buf, size);
#define PRINTF printf
#define PRINTNOW()                         \
  do                                       \
  {                                        \
    time_t mytime = time(NULL);            \
    char *time_str = ctime(&mytime);       \
    time_str[strlen(time_str) - 1] = '\0'; \
    printf("%s\n", time_str);              \
  } while (0)

typedef struct TimerEvent_s
{
  timer_t timerid;
  struct itimerspec duration;
  void (*callback)(void *);
  void *context;
  bool started;
} TimerEvent_t;

typedef time_t TimerTime_t;

typedef TimerEvent_t timer_obj_t;

/**
 * @brief Initializes timer module
 */
void platform_timer_init(void);

/**
 * @brief Adds a timer
 *
 * @param tmr Timer object
 * @param id Timer ID
 * @param cb Callback to be executed when the timer goes off
 * @param cb_arg Argument to the callback
 */
void platform_timer_add(timer_obj_t *tmr, uint16_t id, void (*cb)(void *),
                        void *cb_arg);

/**
 * @brief Starts a timer
 *
 * @param tmr Timer object
 */
void platform_timer_start(timer_obj_t *tmr);

/**
 * @brief Sets the duration of a timer
 *
 * @param tmr Timer object
 * @param duration Duration of the timer in milliseconds
 */
void platform_timer_set_duration(timer_obj_t *tmr, uint32_t duration);

/**
 * @brief Stops a timer
 *
 * @param tmr Timer object
 */
void platform_timer_stop(timer_obj_t *tmr);

void TimerInit(TimerEvent_t *obj, void (*callback)(void *));
void TimerSetValue(TimerEvent_t *obj, uint32_t value);
void TimerStart(TimerEvent_t *obj);
void TimerStop(TimerEvent_t *obj);
void TimerSetContext(TimerEvent_t *obj, void *context);
bool TimerIsStarted(TimerEvent_t *obj);
void TimerReset(TimerEvent_t *obj);
TimerTime_t TimerGetCurrentTime(void);

void platform_print_hex_buffer(const uint8_t *buffer, uint16_t size);

void platform_hw_init(void);

void print_usage(void);

void platform_error_handler(void);

void platform_configure_sleep_mode(void);

void platform_enter_low_power_ll(void);

void ENABLE_IRQ();

void DISABLE_IRQ();

void BEGIN_CRITICAL_SECTION();

void END_CRITICAL_SECTION();

int platform_entropy_hardware_poll(void *data, unsigned char *output,
                                   size_t len, size_t *olen);

/**
 * This function allows to register a file descriptor on which
 * LPM_EnterLowPower() will poll() for any input event.
 *
 * Parameters :
 * - fd: file descriptor to watch
 * - callback: function to call upon event. This is optional, specify
 *             NULL if nothing has to be performed upon event.
 *
 * Return : true on success, false if this fd has already been registered
 *          or if the maximum number of watched fd has been reached, or
 *          invalid fd value (negative value)
 */
bool watch_fd_for_input(int fd, void (*callback)(void));
/**
 * This function unregister a previously registered fd
 * with watch_fd_for_input().
 *
 * Parameter :
 * - fd: file descriptor to unwatch
 *
 * Return : true on success, false if this fd is invalid (negative value)
 *          or if this fd is not registered.
 */
bool unwatch_fd_for_input(int fd);

// See man strftime
bool time_as_formatted_string(char *str, size_t str_size, const char *format);

void bin_to_hex(const uint8_t *bin, size_t bin_size, uint8_t *hex);

/**
 * @brief Read the current time (ms)
 *
 * @param time returns current time (ms)
 */
uint32_t platform_get_current_time(void);

/**
 * @brief  Return the Time elapsed since a fixed moment (ms)
 *
 * @param[in] fixed moment (ms)
 * @return time returns elapsed time
 */
uint32_t
platform_get_elpased_time(uint32_t past);

#endif /* APP_INC_PLATFORM_H_ */
