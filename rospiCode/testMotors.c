/**************************************************************************|
|	This tests allows the robot to walk forward and backward at different  |
|	speed, for a time slice of 3 seconds.								   |
|																		   |
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "fsm.h"
#include <unistd.h>
#include "Robot.h"
#include "TX_RX.h"
#include "tests.h"

/* TEST_MOTORS STATES */
#define IDLE 0
#define VELMIN_1 1
#define VELMED_1 2
#define VELMAX 3
#define VELMED_2 4
#define VELMIN_2 5

/* VARIABLES */

int speeds [6] = {0, 150, 400, 700, 400, 150};
int direction;
int timer;
int initMotores;
int endTest = 0;
/* -1 MARCHA ATRÁS
   0 STOP
   1 HACIA DELANTE
*/

/* INPUT FUNCTIONS */

static int checkInitMotores(fsm_t *this) {
	if(initMotores == 1 || initMotores == -1) {
		direction = initMotores;
		return 1;
	}
	else {
		return 0;
	}
}

static int checkNoInitMotores(fsm_t *this){
	if(initMotores == 0){
		return 1;
	}
	else{
		return 0;
	}
}

static int checkTimer(fsm_t *this){
	if(timer <= 0){
		timer = 30;
		return 1;
	}
	else{
		timer--;
		return 0;
	}
}

/* OUTPUT FUNCTIONS */

static void doNothing(fsm_t *this) {
}

static void move(fsm_t *this){
int speed = direction*speeds[this->current_state];
printf("Speed: %d", speed);
setVel(speed);

/*
	MOVIMIENTO DE LOS MOTORES
*/
}

static void stopM(fsm_t *this){
	direction = 0;
	setVel(0);
	endTest = 1;
	usleep(10000);
}

/* TABLA DE TRANSICCIONES */
static fsm_trans_t tt_testMotores[] = {
	{IDLE, checkNoInitMotores, IDLE, doNothing} ,
	{IDLE, checkInitMotores, VELMIN_1, move} ,
	{VELMIN_1, checkTimer, VELMED_1, move} ,
	{VELMED_1, checkTimer, VELMAX, move} ,
	{VELMAX, checkTimer, VELMED_2, move} ,
	{VELMED_2, checkTimer, VELMIN_2, move} ,
	{VELMIN_2, checkTimer, IDLE, stopM} ,
	{-1, NULL, -1, NULL} ,
};

int run_testMotors(int direct){

	/*
	ANTES DE NADA HAY QUE HACER LA SINCRONIZACIÓN
	*/
	initMotores = direct;
	timer = 30;
	endTest = 0;
	fsm_t *motoresFsm = fsm_new(tt_testMotores);
	motoresFsm->current_state = 0;
	
	while (endTest == 0) {
		execRX();
		
		printf(">> Init Motores: %d\t", initMotores);
		printf("Estado: %d\t", motoresFsm->current_state);
		printf("Contador = %d", timer);
		
		fsm_fire(motoresFsm);
		printf("\n");

		execTX();
	}
	
	return 0;
		
}
//EOF

