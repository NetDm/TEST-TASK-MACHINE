#ifndef __SYSUTILS___
#define __SYSUTILS___

#include <stddef.h>

size_t getTime();

typedef size_t user_timer_t;

void resetUserTimer(user_timer_t* aTimer);
size_t getUserTimer(user_timer_t* aTimer);

#endif
