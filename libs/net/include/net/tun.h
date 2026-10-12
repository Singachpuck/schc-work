#ifndef NET_TUN_H_
#define NET_TUN_H_

#include <stddef.h>

int create_tun(const char *requested_name, char *actual_name,
               size_t actual_name_len);

#endif /* NET_TUN_H_ */

