#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Robot.h"
#include "TX_RX.h"

//Puntero a cada variable del SIP
#define SIP_X 4
#define SIP_Y 6
#define SIP_ANGLE 8
#define SIP_RBUMPERS 13
#define SIP_FBUMPERS 14
#define SIP_FLAGS 17
#define SIP_COMPASS 19
#define SIP_SONAR 20

//Sonar info
unsigned short int sonars[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};


/*
	Añade el siguiente comando a enviar, hasta un máximo de MAX_COMMAND
*/
int send(unsigned char *arg) {
	unsigned char* p;
	int length = (*arg) + 1;
	int i = 0;

	if (nCommand >= MAX_COMMAND) {
		return -1;
	}

	p = sendBuff[nCommand];//Prepara la zona de memoria del buffer del emisor
	while ((i++) < length) {
	  *p++ = *arg++;
	}
	/*p = sendBuff[nCommand-1];
	while(*p){
		printf(" %d ", *p++);
	}
	printf("\n");*/
	nCommand++;
	return 1;
}

/*Inicio de conexión*/
void initRobot(){
	int i;
	
	init_connection();	

	sendBuff[0] = (char *) malloc (MAX_COMMAND*BUF_LENGTH);

	for (i = 1; i < MAX_COMMAND; i++) {
		sendBuff[i] = sendBuff[i-1] + BUF_LENGTH;
	}
}


static unsigned short int get_int(char* a){
  return ((*(a+1))<<8 | (*a));
}

/*Cierre de conexión*/
void closeRobot(void){
	close_connection();
}


/*Devuelve lectura del sonar indicado*/
int getSonar(int sonar){
	return sonars[sonar];
}

/*Devuelve el estado del motor*/

int getMotorState(){
 return (*(SIP + SIP_FLAGS))&(0x01);
}

/*Devuelve coordenada X*/
int getXPos(){
	return get_int(SIP + SIP_X);
}

/*Devuelve coordenada Y*/
int getYPos(){
	return get_int(SIP + SIP_Y);
}

/*Devuelve orientación en grados*/
int getOrientation(){
	return get_int(SIP + SIP_ANGLE);
}

/*Devuelve estado de todos los bumpers delanteros*/
unsigned char getFrontBumpers(){
	return ((*(SIP + SIP_FBUMPERS))>>1)&(0x1f);
}

/*Devuelve estado de todos los bumpers traseros*/
unsigned char getRearBumpers(){
	return ((*(SIP + SIP_RBUMPERS))>>1)&(0x1f);
}

/*Devuelve estado de la brújula interna*/
unsigned char getCompass(){
	return *(SIP + SIP_COMPASS);
}

/*
	Actualiza las medidas del sónar con el último SIP. Debe ejecutarse con cada SIP recibido.
*/
void refresh_sonar(){
	unsigned char* pSonar = SIP+SIP_SONAR;
	int sonar_count = *pSonar;
	int number_sonar, i;
	for(i=0;i<sonar_count;i++){
		number_sonar = *(pSonar+1+i*3);
		sonars[number_sonar] = get_int(pSonar+2+i*3);
	}
}





//Parámetros de movimiento

void setRotVel(int i){
	unsigned char command[5] = {4, 21, 59, 0, 0};

	
	if(i < 0){
		command[2] = 27;
		i = i*(-1);
	}
	
	command[3] = (unsigned char)(i%256);
	command[4] = (unsigned char)(i/256);

	send(command);
}

/*Actualiza velocidad de movimiento*/
void setVel(int i){
	unsigned char command[5] = {4, 11, 59, 0, 0};


	if(i < 0){
		command[2] = 27;
		i = i*(-1);
	}
	
	command[3] = (unsigned char)(i%256);
	command[4] = (unsigned char)(i/256);

	send(command);
}

/*Actualiza velocidad máxima*/
void setVelM(int i){
	unsigned char command[5] = {4, 6, 59, 0, 0};


	if(i < 0){
		command[2] = 27;
		i = i*(-1);
	}
	
	command[3] = (unsigned char)(i%256);
	command[4] = (unsigned char)(i/256);

	send(command);
}

/*Actualiza aceleración*/
void setAccel(int i){
	unsigned char command[5] = {4, 5, 59, 0, 0};

	if(i < 0){
		command[2] = 27;
		i = i*(-1);
	}
	
	command[3] = (unsigned char)(i%256);
	command[4] = (unsigned char)(i/256);

	send(command);
}

/*Actualiza velocidad de rotación*/
void setRotAccel(int i){
	unsigned char command[5] = {4, 23, 59, 0, 0};

	if(i < 0){
		command[2] = 27;
		i = i*(-1);
	}

	command[3] = (unsigned char)(i%256);
	command[4] = (unsigned char)(i/256);

	send(command);
}

/*Velocidad = 0*/
void stop(void){
	unsigned char command[5] = {4, 29, 59, 1, 0};
	send(command);
}


//EXTRAS
//Sin probar
void midi(unsigned char* arg){
	int i;
	int length = *(arg++)+1;
	unsigned char* command = (unsigned char*)malloc(length);
	unsigned char* command_c = command;
	*command++ = length;
	*command++ = 15;
	for (i = 0; i < length-2; i++){
		*command++ = *arg++;
	}		
	send(command_c);
}

