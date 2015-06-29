/* TEST BUMPERS */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "fsm.h"
#include "Robot.h"
#include "TX_RX.h"
#include "tests.h"

/* ESTADOS */
#define IDLE 0
#define FORWARD_MOVE 1
#define REAR_MOVE 2

/* VARIABLES */
int inicio, stopB;
int contador, direction;
int velocity[3] = {0, 400, -400};
int endTestB = 0;
int nHits = 0;

/* INPUT FUNCTIONS */

static int checkNoInicio(fsm_t *this){
	if(inicio == 0)
		return 1;
	else{
		return 0;
	}
// printBumpers();
}

static int checkInicioPositivo(fsm_t *this){
	if(inicio == 1)
		return 1;
	else{
		return 0;
	}
//printBumpers();
}

static int checkInicioNegativo(fsm_t *this){
	if(inicio == -1)
		return 1;
	else{
		return 0;
	}
//printBumpers();
}

static int checkNoFrontBumpers(fsm_t *this){
	// printBumpers();
	char bumpers = getFrontBumpers();
	int i;
	for (i = 5; i > 0; --i) {
		if(((bumpers >> i) & 1))
			return 0;
	}
	return 1;
	
}

static int checkFrontBumpers(fsm_t *this){
	//printBumpers();
	char bumpers = getFrontBumpers();
	int i;
	for (i = 5; i > 0; --i) {
		if(((bumpers >> i) & 1))
			return 1;
	}
	return 0;
}

static int checkNoRearBumpers(fsm_t *this){
	//printBumpers();
	char bumpers = getRearBumpers();
	int i;
	for (i = 5; i > 0; --i) {
		if(((bumpers >> i) & 1))
			return 0;
	}
	return 1;
}

static int checkRearBumpers(fsm_t *this){
	// printBumpers();
	char bumpers = getRearBumpers();
	int i;
	for (i = 5; i > 0; --i) {
		if(((bumpers >> i) & 1))
			return 1;
	}
	return 0;
	
}

static int checkStopOrCounter(fsm_t *this){
	//printBumpers();
	if((stopB == 1 ) || (contador == nHits)) {
		endTestB = 1;
		return 1;
	}
	else {
		return 0;
	}
}



/* OUTPUT FUNCTIONS */

static void doNothing(fsm_t *this){
}

/* checkCounter */
static void checkCounter(fsm_t *this){
	if(contador < nHits) contador++;
}

/*static void move(fsm_t *this){
  direction = velocity[this->current_state];
  vel(direction);
  printf("%d %d\n", this->current_state, direction);
  }*/

/* TABLA DE TRANSICIONES */
static fsm_trans_t tt_testBumpers[] = {
	{IDLE, checkNoInicio, IDLE, doNothing},
	{IDLE, checkInicioPositivo, FORWARD_MOVE, doNothing},
	{IDLE, checkInicioNegativo, REAR_MOVE, doNothing},
	{FORWARD_MOVE, checkNoFrontBumpers, FORWARD_MOVE, doNothing},
	{FORWARD_MOVE, checkStopOrCounter, IDLE, doNothing},
	{FORWARD_MOVE, checkFrontBumpers, REAR_MOVE, checkCounter},
	{REAR_MOVE, checkNoRearBumpers, REAR_MOVE, doNothing},
	{REAR_MOVE, checkStopOrCounter, IDLE, doNothing},
	{REAR_MOVE, checkRearBumpers, FORWARD_MOVE, checkCounter},
	{-1, NULL, -1, NULL},
};

int run_testBumpers(int direct, int hits) {
	
	inicio = direct; 
	stopB = 0;
	contador = 0; 
	direction = direct;
	nHits = hits;
	
	
	fsm_t *testBumpers = fsm_new(tt_testBumpers);
	testBumpers->current_state = 0;
	endTestB = 0;
	
	while(endTestB == 0) {
		execRX();
		
		//printf("Inicio: %d\tStop: %d\tEstado: %d\tContador: %d\n", inicio, stop, testBumpers->current_state, contador);
		fsm_fire(testBumpers);
		
		direction = velocity[testBumpers->current_state];                                   
		inicio = 0;        
		setVel(direction);                                                          
		printf("%d %d\n", testBumpers->current_state, direction);     

		execTX();
		
	}
	
	return 0;
}
//EOF
