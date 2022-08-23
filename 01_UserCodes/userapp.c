
#include "userapp.h"

#include <stddef.h>
#include <stdio.h>

#include "HWconfig.h"
#include "dbug.h"
#include "main.h"
#include "sysutils.h"
#include "blink.h"

typedef double scale_t;

typedef struct{
	scale_t tmpPositions;
	scale_t countScaleL;
	scale_t countScaleR;
	scale_t fixScaleL;
	scale_t fixScaleR;
	user_timer_t timeMoveL;
	user_timer_t timeMoveR;
}scale_drive_t;

static scale_drive_t ScaleDrive = {
		 .tmpPositions=0.0
		,.countScaleL=0.0
		,.countScaleR=0.0
		,.fixScaleL=0.0
		,.fixScaleR=0.0
		,.timeMoveL=0
		,.timeMoveR=0
};

typedef enum{
	INIT_MATCH=0,

	WAIT_COMMAND,

	MOVING_TO_POINT_L,
	SWITCH_L,
	PREWAIT_SWITCH_L,
	WAIT_SWITCH_L,

	MOVING_TO_POINT_R,
	SWITCH_R,
	PREWAIT_SWITCH_R,
	WAIT_SWITCH_R,

	MOVING_TO_CENTER,

	SWITCHES_ERROR
}match_drive_t;

#if DBUG>=1
const char* szMatch[]={ "INIT_MATCH"
				,"WAIT_COMMAND"
				,"MOVING_TO_POINT_L"
				,"SWITCH_L"
				,"PREWAIT_SWITCH_L"
				,"WAIT_SWITCH_L"
				,"MOVING_TO_POINT_R"
				,"SWITCH_R"
				,"PREWAIT_SWITCH_R"
				,"WAIT_SWITCH_R"
				,"MOVING_TO_CENTER"
				,"SWITCHES_ERROR"
};
#endif



typedef enum{
	SWITCHES_OPEN=0,
	SWITCH_L_CLOSE=(1<<0),
	SWITCH_R_CLOSE=(1<<1),
	ERROR_SWITCH =( SWITCH_R_CLOSE | SWITCH_L_CLOSE )
}swithes_t;

swithes_t getStateSwitches(){
	swithes_t result = 0;
	if (  HAL_GPIO_ReadPin( fromMX(SWITCH_L_IN) )  ==  SWICH_CLOSE_PIN_STATE  ){
		result |= SWITCH_L_CLOSE;
	}
	if (  HAL_GPIO_ReadPin( fromMX(SWITCH_R_IN) )  ==  SWICH_CLOSE_PIN_STATE  ){
		result |= SWITCH_R_CLOSE;
	}
	return result;
}

typedef enum{
	FALSE = (1==0),
	TRUE = (1==1)
}bool_t;

bool_t isChangeDirect = FALSE ;
typedef enum{
	LEFT = 0,
	RIGHT
}direct_t;

// Вспомогательные функции

static direct_t isDirectMove = 0 ;

bool_t isStartMovedL(){
	if ((isChangeDirect) && isDirectMove == LEFT ){
		isDirectMove = RIGHT;
		HAL_Delay(200);
		isChangeDirect = FALSE;
		return TRUE;
	}
	else
	return FALSE;
}
bool_t isStartMovedR(){
	if ((isChangeDirect) && isDirectMove == RIGHT ){
		isDirectMove = LEFT;
		HAL_Delay(200);
		isChangeDirect = FALSE;
		return TRUE;
	}
	else
	return FALSE;
}

void HAL_GPIO_EXTI_Callback(uint16_t aPin){
	if (aPin == CHANGE_DIRECT_BUTTON_Pin){
		isChangeDirect = TRUE;
		while(HAL_GPIO_ReadPin( fromMX(CHANGE_DIRECT_BUTTON) )  == RESET ){
		}

	}
}

// управление драйверами

void stopAllDrivers(){
	setBlinkPeriod(127);
	HAL_GPIO_WritePin( fromMX(DRIVER_L_EN) , DRIVER_OFF_STATE  );
	HAL_GPIO_WritePin( fromMX(DRIVER_R_EN) , DRIVER_OFF_STATE  );
}

void moveOneStepL(){
	HAL_GPIO_WritePin( fromMX(DRIVER_L_EN) , DRIVER_ON_STATE  );
	ScaleDrive.tmpPositions -= SCALE_STEP_L;

}

void moveOneStepR(){
	HAL_GPIO_WritePin( fromMX(DRIVER_R_EN) , DRIVER_ON_STATE  );
	ScaleDrive.tmpPositions += SCALE_STEP_R;
}

static double abs(double arg){
	if ( arg >= 0 ){
		return  (arg);
	}
	else{
		return (-arg);
	}
}

void userapp(){

	static user_timer_t TimeToTail,StartMoveTime,TimeToCenter;

	static match_drive_t MatchDrive=0;
	match_drive_t setNewState(match_drive_t aNewState){
		return (MatchDrive = aNewState);
	}

	#if DBUG>=1
	static match_drive_t memMatchDrive=-1;
	if (memMatchDrive!=MatchDrive){
		dbugnl("Новое состояние автомата %s",szMatch[MatchDrive]);
		HAL_Delay(200);
		memMatchDrive = MatchDrive;
	}
	#endif



	switch(MatchDrive){

	case INIT_MATCH:{
		resetUserTimer(&StartMoveTime);
		ScaleDrive.tmpPositions = 0.0 ;
		setNewState(WAIT_COMMAND);
	}
	break;

	case WAIT_COMMAND:{
		static int c;
		if ((++c&511)==511){
			dbug(".");
		}
		resetUserTimer(&StartMoveTime);
		stopAllDrivers();
		if (isStartMovedL()){
			setBlinkPeriod(60);
			dbugnl("Начато движение в лево");
			setNewState(MOVING_TO_POINT_L);
		}
		if (isStartMovedR()){
			setBlinkPeriod(60);
			dbugnl("Начато движение в право");
			setNewState(MOVING_TO_POINT_R);
		}
		//todo так-же возможен вариант ассинхронной смены направления, но необходимы доп проверки, в условии такой задачи не стояло
	}
	break;

	case MOVING_TO_POINT_L:{//LEFT
		if ( getUserTimer(&StartMoveTime) > TIMEOUT_MOVE_L){
			stopAllDrivers();
			dbugerr("время движения в лево слишком большое! останов!");
		}else
		if ( getStateSwitches() == SWITCHES_OPEN ){
			moveOneStepL();
		}else
		if ( getStateSwitches() == SWITCH_L_CLOSE ){
			ScaleDrive.fixScaleL = ScaleDrive.tmpPositions;
			setNewState( PREWAIT_SWITCH_L );
		}else{
			setNewState( SWITCHES_ERROR );
		}

	}
	break;

	case PREWAIT_SWITCH_L:{
		stopAllDrivers();
		setBlinkPeriod(260);
		resetUserTimer(&TimeToTail);
		setNewState( WAIT_SWITCH_L );
	}
	break;

	case WAIT_SWITCH_L:{
		if (  getUserTimer(&TimeToTail)  >  CONTACT_BOUNCE_MSEC  ){
			if ( getStateSwitches() != SWITCH_L_CLOSE ){
				//todo прошел импульс от концевика, но далее он разомкнулся, не критично, сообщить о необходимости чистки или замены
				dbugw("сбой концевика L");
			}
			//todo можно замерить время обратного хода в R сторону
			resetUserTimer(&TimeToCenter);
			setNewState( MOVING_TO_CENTER );
		}
	}
	break;

	case MOVING_TO_POINT_R:{//RIGHT
		if ( getUserTimer(&StartMoveTime) > TIMEOUT_MOVE_R){
			stopAllDrivers();
			dbugerr("время движения в право слишком большое! останов!");
		}else
		if ( getStateSwitches() == SWITCHES_OPEN ){
			moveOneStepR();
		}else
		if ( getStateSwitches() == SWITCH_R_CLOSE ){
			ScaleDrive.fixScaleR = ScaleDrive.tmpPositions;
			setNewState( PREWAIT_SWITCH_R );
		}else{
			setNewState( SWITCHES_ERROR );
		}

	}
	break;

	case PREWAIT_SWITCH_R:{
		stopAllDrivers();
		setBlinkPeriod(260);
		resetUserTimer(&TimeToTail);
		setNewState( WAIT_SWITCH_R );
	}
	break;

	case WAIT_SWITCH_R:{
		if ( getUserTimer(&TimeToTail) > CONTACT_BOUNCE_MSEC ){
			if ( getStateSwitches() != SWITCH_R_CLOSE ){
				//todo прошел импульс от концевика, но далее он разомкнулся, не критично, сообщить о необходимости чистки или замены
				dbugw("сбой концевика R");
			}
			//todo можно замерить время обратного хода в L сторону
			resetUserTimer(&TimeToCenter);
			setNewState( MOVING_TO_CENTER );
		}
	}
	break;

	case MOVING_TO_CENTER:{//CENT
		if (  ( getStateSwitches() != SWITCHES_OPEN )  &&  ( getUserTimer(&TimeToCenter) > DEPARTURE_TIME_MSEC )  ){
			setNewState( SWITCHES_ERROR );
		}
		if ( ScaleDrive.tmpPositions < 0 ) {
			if ( abs(ScaleDrive.tmpPositions) < SCALE_STEP_L ){
				setNewState(WAIT_COMMAND);
			}else{
				moveOneStepR();
			}
		}else
		if ( ScaleDrive.tmpPositions > 0 ) {
			if ( abs(ScaleDrive.tmpPositions) < SCALE_STEP_R ){
				setNewState(WAIT_COMMAND);
			}else{
				moveOneStepL();
			}

		}else{
			setNewState(WAIT_COMMAND);
		}
	}
	break;

	case SWITCHES_ERROR:{
		stopAllDrivers();
		dbugw("ошибка концевиков!");
		//todo что-то надо делать! куда-то выводить, сигналить
		while(TRUE){
		}
	}
	break;

	default:
		dbugerr("сбой состояния автомата управления приводом!");
	}

	if (  ( getStateSwitches() == ERROR_SWITCH )  &&  ( getUserTimer(&StartMoveTime) > DEPARTURE_TIME_MSEC )  ){
		dbug("\n\rВ строке %d, событие: ",__LINE__);
		setNewState( SWITCHES_ERROR );
	}

}
