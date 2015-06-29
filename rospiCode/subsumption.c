#include "Robot.h"
#include "tasks.h"
#include "TX_RX.h"

#define BEHAVIOURS 3

#define BUMPERS_PRIORITY 0
#define SONARS_PRIORITY 1
#define GOAHEAD_PRIORITY 2

#define VELOCIDAD 350
#define SONAR_PROXIMITY_LIMIT 500

static int activation_table[6];
int endSub = 0;

/* MAYOR PRIORITY 0
   MINOR PRIORITY 2 */

void executeBehaviours();
void evaluateBumpers();
void evaluateSonars();

void runUntilObject(){
	
	endSub = 0;
	activation_table[4] = VELOCIDAD;
	activation_table[5] = 1;
	refresh_sonar();
	executeBehaviours();
	
	int i;
	
	for(i = 1; i < BEHAVIOURS*2; i=i+2){
		if(activation_table[i] == 1){
			setVel(activation_table[i-1]);
			break;
		}	
	}
}




void executeBehaviours(){
	
	activation_table[1] = 0;
	activation_table[3] = 0;
	
	
	evaluateBumpers();
	evaluateSonars();
}

void evaluateBumpers(){
	
	int frontBumpers;
	/*int rearBumpers;*/
	
	frontBumpers = getFrontBumpers();
	/*rearBumpers = getRearBumpers();*/
	
	if(frontBumpers != 0){
		activation_table[0] = -VELOCIDAD;
		activation_table[1] = 1;
	}
	else{
		activation_table[1] = 0;
	}
	/* if(rearBumpers != 0){
	   activation_table[0] = VELOCIDAD;
	   activation_table[1] = 1;
	   }
	   else{
	   activation_table[1] = 0;
	   }*/
}

void evaluateSonars(){
  
	int sonars[4];
	int i;
	
	for(i = 2; i < 6; i++){
		sonars[i-2] = getSonar(i);
		/*printf("%d \n", sonars[i-2]);*/
	}
	
	for(i = 0; i < 4; i++){
		if(sonars[i] <= SONAR_PROXIMITY_LIMIT){
			activation_table[2] = 0;
			activation_table[3] = 1;
		}
	}
}

