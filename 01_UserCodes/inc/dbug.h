/*
 * dbug.h
 *
 *  Created on: Apr 21, 2021
 *      Author: me
 */

#ifndef MYDBUG_H_
#define MYDBUG_H_

#include "HWconfig.h"
#include <stdio.h>

#ifdef DBUG

#ifdef DBUG_HUART

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

int __io_putchar(int ch);

#endif


#define dbug(...) printf(__VA_ARGS__)

void flushDbugUart();

#define dbugnl(...) {\
					dbug("\n\r");\
					if (DBUG>=2){\
						dbug("[ms:%lu]: ",HAL_GetTick());\
					}\
					dbug(__VA_ARGS__);\
					dbug("\n\r");\
					/*flushDbugUart();*/\
					}
#if DBUG>=4 /*аппаратные драйверы*/
#define dbug4(...) dbug(__VA_ARGS__)
#define dbugnl4(...) dbugnl(__VA_ARGS__)
#else
#define dbug4(...)
#define dbugnl4(...)

#endif


#define dbugw(...) {dbug("Что-то не то!: ");dbugnl(__VA_ARGS__);}

#define dbugerr(...) 	{\
						dbugnl("!err: ");\
							if (DBUG>=2){\
								dbug("фаил: %s, строка %d, ",__FILE__, __LINE__);\
							}\
						dbug(__VA_ARGS__);\
						if (DBUG>=5) while(1==1);\
						}

void outputDbugStr(const char* str,const size_t i);

#define dbugClear() dbug("%c[2J%c[H\r\n\n\n\n\n\n\n\n\n\n\n",27,27)


#else	//debug

#define dbug(...)
#define dbugnl(...)
#define dbugerr(...)
#define dbugw(...)

#endif	//debug

#endif /* DBUG_H_ */
