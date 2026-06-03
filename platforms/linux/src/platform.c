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
 * Author: Arthur Josso arthur.josso@ackl.io
 */

#include <errno.h>
#include <poll.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "platform.h"

// Max size of a trace message, including '\0'
#define MAX_TRACE_SIZE 512

// Linux signal raised upon timer expiration
#define TIMER_SIGNAL SIGUSR1

#define MAX_WATCHED_FDS 8

static struct
{
  int fd;
  void (*cb)(void);
} _watched_fds[MAX_WATCHED_FDS] = {{-1, NULL}};

bool watch_fd_for_input(int fd, void (*callback)(void))
{
  uint16_t idx;

  if (fd < 0)
    return false;
  for (idx = 0; idx < MAX_WATCHED_FDS && _watched_fds[idx].fd != -1 &&
                _watched_fds[idx].fd != fd;
       ++idx)
    ;
  if (idx == MAX_WATCHED_FDS || _watched_fds[idx].fd == fd)
    return false;
  _watched_fds[idx].fd = fd;
  _watched_fds[idx].cb = callback;
  if (idx + 1 < MAX_WATCHED_FDS) // init next free slot
    _watched_fds[idx + 1].fd = -1;
  return true;
}

bool unwatch_fd_for_input(int fd)
{
  uint16_t idx;
  uint16_t last;

  if (fd < 0)
    return false;
  // find index of searched fd
  for (idx = 0; idx < MAX_WATCHED_FDS && _watched_fds[idx].fd != -1 &&
                _watched_fds[idx].fd != fd;
       ++idx)
    ;
  // check if actually registered
  if (idx == MAX_WATCHED_FDS || _watched_fds[idx].fd == -1)
    return false;
  // find index of last registered fd
  for (last = idx;
       last + 1 < MAX_WATCHED_FDS && _watched_fds[last + 1].fd != -1; ++last)
    ;
  // replace fd to remove entry by last entry
  _watched_fds[idx] = _watched_fds[last];
  // nullify last unused entry
  _watched_fds[last].fd = -1;
  _watched_fds[last].cb = NULL;
  return true;
}

static void _sighandler(int sig, siginfo_t *info, void *ctx)
{
  TimerEvent_t *obj;

  (void)ctx;
  obj = (TimerEvent_t *)info->si_value.sival_ptr;
  if (sig != TIMER_SIGNAL || info->si_code != SI_TIMER)
  {
    fprintf(stderr, "Error: timer callback: assert failed\n");
    return;
  }
  obj->started = false;
  obj->callback(obj->context);
}

static void _timer_cleaner_register(timer_t t);

void platform_timer_init(void)
{
}

void platform_timer_add(timer_obj_t *tmr, uint16_t id, void (*cb)(void *),
                        void *cb_arg)
{
  (void)id;

  TimerInit(tmr, cb);
  TimerSetContext(tmr, cb_arg);
}

void platform_timer_set_duration(timer_obj_t *tmr, uint32_t duration)
{
  TimerSetValue(tmr, duration);
}

void platform_timer_start(timer_obj_t *tmr)
{
  TimerStart(tmr);
}

void platform_timer_stop(timer_obj_t *tmr)
{
  TimerStop(tmr);
}

void TimerInit(TimerEvent_t *obj, void (*callback)(void *))
{
  struct sigaction sigact = {};
  struct sigevent sevp = {};

  memset(&obj->duration, 0, sizeof(obj->duration));
  obj->callback = callback;
  obj->context = NULL;
  obj->started = false;

  sigact.sa_sigaction = &_sighandler;
  sigact.sa_flags = SA_SIGINFO | SA_RESTART;
  if (sigaction(TIMER_SIGNAL, &sigact, NULL) == -1)
  {
    fprintf(stderr, "Error: TimerInit(): sigaction() failed\n");
    return;
  }

  sevp.sigev_notify = SIGEV_SIGNAL;
  sevp.sigev_signo = TIMER_SIGNAL;
  sevp.sigev_value.sival_ptr = obj;
  if (timer_create(CLOCK_MONOTONIC, &sevp, &obj->timerid) == -1)
    fprintf(stderr, "Error: TimerInit(): timer_create() failed\n");
  _timer_cleaner_register(obj->timerid);
}

void TimerSetValue(TimerEvent_t *obj, uint32_t value)
{
  obj->duration.it_value.tv_sec = value / 1000;
  obj->duration.it_value.tv_nsec = (value % 1000) * 1000000;
}

void TimerStart(TimerEvent_t *obj)
{
  if (obj->started)
  {
    fprintf(stderr, "Warning: TimerStart(): timer already started\n");
    return;
  }
  if (timer_settime(obj->timerid, 0, &obj->duration, NULL) == -1)
    fprintf(stderr, "Error: TimerStart(): timer_settime() failed\n");
  else
    obj->started = true;
}

void TimerStop(TimerEvent_t *obj)
{
  struct itimerspec null_duration = {};

  if (timer_settime(obj->timerid, 0, &null_duration, NULL) == -1)
    fprintf(stderr, "Error: TimerStop(): timer_settime() failed\n");
  else
    obj->started = false;
}

void TimerSetContext(TimerEvent_t *obj, void *context)
{
  obj->context = context;
}

bool TimerIsStarted(TimerEvent_t *obj)
{
  return obj->started;
}

void TimerReset(TimerEvent_t *obj)
{
  obj->started = false;
  TimerStart(obj);
}

TimerTime_t TimerGetCurrentTime(void)
{
  struct timeval t;

  gettimeofday(&t, NULL);
  return t.tv_sec * 1000 + (TimerTime_t)(t.tv_usec / 1000);
}

void platform_error_handler(void)
{
}

void ENABLE_IRQ()
{
}

void DISABLE_IRQ()
{
}

void BEGIN_CRITICAL_SECTION()
{
}

void END_CRITICAL_SECTION()
{
}

void platform_print_hex_buffer(const uint8_t *buffer, uint16_t size)
{
  printf("Buf(%d):", size);
  for (uint16_t i = 0; i < size; ++i)
    printf("%02x", buffer[i]);
  printf("\n");
}

/**
 * For traces. Provides time.
 */
uint32_t platform_get_clock_ms(void)
{
  struct timespec spec;

  clock_gettime(CLOCK_MONOTONIC, &spec);

  return spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
}

void platform_hw_init(void)
{
  platform_timer_init();
}

void platform_configure_sleep_mode(void)
{
  printf("Configure sleep mode\n");
}

void platform_enter_low_power_ll(void)
{
  struct pollfd fds[MAX_WATCHED_FDS] = {};
  uint16_t nbr_fds;
  int ret;

  if (_watched_fds[0].fd == -1) // if no fd to watch, just pause()
  {
    pause();
    return;
  }
  for (nbr_fds = 0; nbr_fds < MAX_WATCHED_FDS && _watched_fds[nbr_fds].fd != -1;
       ++nbr_fds) // init poll structure
  {
    fds[nbr_fds].fd = _watched_fds[nbr_fds].fd;
    fds[nbr_fds].events = POLLIN;
  }
  ret = poll(fds, nbr_fds, 400); // -1 for blocking poll()
  if (ret <= 0)
  {
    if (ret == -1 && errno != EINTR)
      fprintf(stderr, "Error: poll() failed\n");
    return;
  }
  for (uint16_t i = 0; i < nbr_fds; ++i) // call callbacks
    if ((fds[i].revents & POLLIN) && _watched_fds[i].cb)
      _watched_fds[i].cb();
}

#define ERR_ENTROPY_SOURCE_FAILED -0x003C

int platform_entropy_hardware_poll(void *data, unsigned char *output,
                                   size_t len, size_t *olen)
{
  FILE *file;
  size_t read_len;
  ((void)data);

  *olen = 0;
  file = fopen("/dev/urandom", "rb");
  if (file == NULL)
    return ERR_ENTROPY_SOURCE_FAILED;
  read_len = fread(output, 1, len, file);
  if (read_len != len)
  {
    fclose(file);
    return ERR_ENTROPY_SOURCE_FAILED;
  }
  fclose(file);
  *olen = len;
  return 0;
}

/*
 * Functions to free the allocated timers when the program exits.
 * This is not very useful, but allows to keep a clean valgrind.
 */

#define MAX_CLEANABLE_TIMERS 32
static timer_t _timers[MAX_CLEANABLE_TIMERS] = {};
static size_t _nb_timers = 0;

static void _timer_cleaner(void)
{
  for (size_t i = 0; i < _nb_timers; ++i)
    timer_delete(_timers[i]);
}

static void _timer_cleaner_register(timer_t t)
{
  if (_nb_timers == 0)
    atexit(&_timer_cleaner);
  else if (_nb_timers >= MAX_CLEANABLE_TIMERS)
    return;
  _timers[_nb_timers++] = t;
}

bool time_as_formatted_string(char *str, size_t str_size, const char *format)
{
  time_t t;
  struct tm *tmp;

  t = time(NULL);
  tmp = localtime(&t);
  if (tmp == NULL)
  {
    perror("localtime");
    return false;
  }

  if (strftime(str, str_size, format, tmp) == 0)
  {
    fprintf(stderr, "strftime returned 0");
    return false;
  }
  return true;
}

void bin_to_hex(const uint8_t *bin, size_t bin_size, uint8_t *hex)
{
  for (size_t i = 0; i < bin_size; i++)
  {
    sprintf((char *)hex + i * 2, "%.2X", bin[i]);
  }
}

uint32_t platform_get_current_time(void)
{
  return (uint32_t)TimerGetCurrentTime();
}

uint32_t
platform_get_elpased_time(uint32_t past)
{
  return (platform_get_current_time() - past);
}