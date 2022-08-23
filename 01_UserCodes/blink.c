/*
 * blink.c
 *
 *  Created on: Jun 24, 2022
 *      Author: me
 */

#include "main.h"
#include "dbug.h"
#include <stdio.h>

static int blinkPeriod = 50;

void blinkThread(){
	  static int c;
	  if ( c++ >= blinkPeriod ){
		  c = 0;
		  HAL_GPIO_TogglePin( fromMX(BLINK) );
		  dbug("`");
	  }
}

void setBlinkPeriod(int aP){
	blinkPeriod = aP;
}
