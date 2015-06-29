#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Robot.h"
#include "TX_RX.h"
#include "screen.h"
#include "tasks.h"
#include "interp.h"
#include "tests.h"
#include <pthread.h>
#include "subsumption.h"


int salir = 0;
int direct = 0;
int hits = 0;

//Variable that selects which test/function to execute
int extFunc = -1;
pthread_mutex_t cerrojo_extFunc; //Mutex for extFunc

/* Lock/unlock 'cerrojo_extFunc' mutex */
static void set_extFunc(int func2exe) {
	pthread_mutex_lock(&cerrojo_extFunc);
	extFunc = func2exe;
	pthread_mutex_unlock(&cerrojo_extFunc);
}

/* Get 'cerrojo_extFunc' mutex state */
static int get_extFunc() {
	int temp = -1;
	pthread_mutex_lock(&cerrojo_extFunc);
	temp = extFunc;
	pthread_mutex_unlock(&cerrojo_extFunc);
	return temp;
}

static int rotateC(char* arg){
	setRotVel(atoi(arg));
	return 0;
}

static int velC(char* arg){
	setVel(atoi(arg));
	return 0;
}


static int setAccC(char* arg){
	setAccel(atoi(arg));
	return 0;
}


static int setVelC(char* arg){
	setVelM(atoi(arg));
	return 0;
}

static int testMotorsC(char* arg){
	set_extFunc(0);
	direct = (atoi(arg));
	return 0;
}

static int testBumpersC(char* arg){
	set_extFunc(1);
	direct = ((atoi(arg)) == 1)*2 - 1;
	hits = (atoi(arg+1));
	return 0;
}

static int setSubsumption(char* arg){
	set_extFunc(2);
	return 0;
}

static int enableSonarsC(char* arg){
	if (atoi(arg) == 0){
		set_enableSonars(0);
		screen_clear();
	} else {
		set_enableSonars(1);
	}
	return 0;
}

static int enableBumpersC(char* arg){
	if (atoi(arg) == 0){
		set_enableBumpers(0);
		screen_clear();
	} else {
		set_enableBumpers(1);
	}
	return 0;
}

static int radiocontrol(char* arg){
	char input = 'n';
	int v = 0;
	int w = 0;

	printf("  Velocidad +100mm/s: w\n  Velocidad -100mm/s: s\n  Giro +15º/s: a\n  Giro -15º/s: d\n");

	while(input != 'q'){
		if(read(0, &input, 1) > 0) {
			if(input=='w') {
				v = v + 100;
			}
			if(input=='s') {
				v = v - 100;
			}
			if(input=='a') {
				w = w + 15;
			}
			if(input=='d') {
	       			w = w - 15;
			}
			
			if(input == 'e') {
				v = 0;
				w = 0;
			}
			setVel(v);
			setRotVel(w);
		}
	}
	return 0;
}

static int quitTest(char* arg) {
	setVel(0);
	set_extFunc(-1);
	return 0;
}


static void *robot(void* arg){
	//Primero añadimos los comandos al intérprete
	interp_addcmd ("setvel", setVelC, "Set the maximum Vel (mm/s) parameter");
	interp_addcmd ("setacc", setAccC, "Set the maximum robot acelleration");
	interp_addcmd ("rotate", rotateC, "Makes the robot rotate");
	interp_addcmd ("vel", velC, "Set the robot movement velocity");
	interp_addcmd ("radiocontrol", radiocontrol, "Simple movement interface");
	interp_addcmd ("testMotors", testMotorsC, "Execute testMotors");
	interp_addcmd ("testBumpers", testBumpersC, "Execute testBumpers");	
	interp_addcmd ("subsumption", setSubsumption, "Execute subsumption.c");
	interp_addcmd ("q", quitTest, "If executing test, exit from test");
	interp_addcmd ("sonars", enableSonarsC, "Enable (1) or disable (0) sonar view");
	interp_addcmd ("bumpers", enableBumpersC, "Enable (1) or disable (0) bumpers view");

	int counter = 0;
	
	while (salir == 0) {
		/* READ SIP PKG */
		execRX();
		if (counter < 15) {
			counter++;
		} else {
			refresh_sonar();
		}
		
		
		/* TESTS AND AI FUNCTIONS */
		switch(get_extFunc()) {
		case 0:
			run_testMotors(direct);
			sleep(1);
			run_testMotors(direct*(-1));
			set_extFunc(-1); 
			break;
		case 1:
			run_testBumpers(direct, hits);
			set_extFunc(-1); 
			break;
		case 2:
			runUntilObject();
			break;
		default:
			break;
		}
		
		/* GENERATE AND SEND PKG */
		execTX();
		
	}
}



int main (){
		
//Initialization
	task_setup ();
	screen_setup (2);
	initRobot();
	
//New thread; it executes real-time tasks
	task_new ("robot", robot, 200, 200, 3, 1024);
	
	interp_run ();
	closeRobot();
	
	sleep(1);
	return 0;
}
