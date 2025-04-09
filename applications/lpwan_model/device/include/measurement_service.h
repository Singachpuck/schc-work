#ifndef MEASUREMENT_SERVICE_LPWAN
#define MEASUREMENT_SERVICE_LPWAN

    typedef struct temp_t {
      float value;
    } temp_t;

    temp_t get_temp();
#endif