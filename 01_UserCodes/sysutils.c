#include <stddef.h>

#include "sysutils.h"
/*
 * @return - system HAL time miliSeconds
 */

size_t getTime(){
	return HAL_GetTick();
}

void resetUserTimer(user_timer_t* aTimer){
	*aTimer = getTime();
}

size_t getUserTimer(user_timer_t* aTimer){
	return (getTime() - *aTimer );
}
