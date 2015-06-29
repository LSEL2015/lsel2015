#include "fsm.h"
#include "TX_RX.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART

#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>


static fsm_t* TX_fsm;
static fsm_t* RX_fsm;
static int descUart;
static int initR = 0;
static int sync0R = 0;
static int sync1R = 0;
static int helloR = 0;
static int reload_RX = 0;
static int closeAll = 0;

int ready = 0;								//Variable booleana, 1 si hay conexión (en caso de caída, se tomarán medidas a niveles superiores).

char* sendBuff[MAX_COMMAND]; 	//Cada uno de los punteros lleva a una cadena de char, el primero es la longitud, luego los datos:
															//Ejemplo: (1, 0)-> Se envía el paquete (250, 251, 3, 0, 0, 0). 
															//Construcción del paquete: (cabecera, tamaño datos + checksum, datos, checksum)

static char recBuffer[BUF_LENGTH];
char* SIP = recBuffer;



int nCommand = 0;


enum tx_state {
	IDLE_TX,
	SYNC0_TX,
	SYNC1_TX,
	SYNC2_TX,
	OPEN_TX,
	MOTORS_TX,
	SEND_TX
};

enum rx_state {
	IDLE_RX,
	SYNC0_RX,
	SYNC1_RX,
	HELLO_RX,
	RECEIVE_RX
};




void close_connection(void){
	closeAll = 1;
}

static unsigned short int get_int(unsigned char* a){
	char c1, c2;
	unsigned short int result;

	c1 = *a;
	c2 = *(a+1);
	result = c1*256 + c2;
	return result;
}

static unsigned short int calcCheckSum(char* buff){
	int i;
	unsigned char n;
	unsigned short int c = 0;
	i = 3;
	n = buff[2] - 2;
	while (n > 1) {
		c += ((unsigned char)buff[i]<<8) | (unsigned char)buff[i+1];
		//c = c & 0xffff;
		n -= 2;
		i += 2;
	}
	if (n > 0)
		c = c ^ (int)((unsigned char) buff[i]);
	return c;
}



static void sendSync0(){
	int ret = 0;	
	unsigned char word[6] = {250, 251, 3, 0, 0, 0};
	ret = write(descUart, word, 6);
	//printf("Envio sync0\n");
}


static void sendSync1(){
	int ret = 0;
	unsigned char word[6] = {250, 251, 3, 1, 0, 1};
	ret = write(descUart, word, 6);
	//printf("Envio sync1\n");
}

static void sendSync2(){
	int ret = 0;	
	unsigned char word[6] = {250, 251, 3, 2, 0, 2};
	ret = write(descUart, word, 6);
	//printf("Envio sync2\n");
}

static void reloadRX(){
	reload_RX = 1;
}

static void sendOpen(){
	int ret = 0;
	unsigned char word[9] = {250, 251, 6, 1, 59, 1, 0, 2, 59};
	ret =	write(descUart, word, 9);
	//printf("Envio Open\n");
}

static void sendHabilitateMotors(){
	int ret = 0;	
	unsigned char word[9] = {250, 251, 6, 4, 59, 1, 0, 5, 59};
	ret =	write(descUart, word, 9);
}

static void setReady(){
	ready = 1;
}

static void doNothing(){
	
}

static void sendCommand(){
	unsigned char buff[BUF_LENGTH];
	int i, j, z;
	int length;
	unsigned short int checkS;
	unsigned char rotate[9] = {250, 251, 6, 21, 59, 30, 0, 51, 59};
	int ret = 0;
	buff[0] = 250;
	buff[1] = 251;
	

	//printf(" nCommand: %d\n", nCommand);
	for(i = 0; i < nCommand; i++){
		length = *sendBuff[i];
		buff[2] = length + 2;
		for(j = 0; j < length; j++)
			buff[j+3] = *(sendBuff[i] + j + 1);

		checkS = calcCheckSum(buff);

		buff[length+3] = (char)(checkS/256);
		buff[length+4] = (char)(checkS%256);

/*
		for(z = 0; z < length+5; z++){
			printf(" %d ", buff[z]);
		}
		printf("\n");
*/
		ret = write(descUart, buff, (length+5));

		//Espera mínima en el envío de comandos consecutivos según el manual: 5 ms - utilizamos 10 ms
		usleep(10000);
	}
	nCommand = 0;
}

static void sendPulse(){
	int ret = 0;		
	unsigned char word[6] = {250, 251, 3, 0, 0, 0};
	ret = write(descUart, word, 6);
}

static void sendClose(){
	int ret = 0;	
	unsigned char word[9] = {250, 251, 6, 2, 59, 1, 0, 3, 59};

	printf("Ok\n");
	ret = write(descUart, word, 9);
	reload_RX = 1;
	ready = 0;
	close(descUart);
}

/////////////////////////////////////////////


static int init(){
	return initR;
}

static int Sync0received(){
	int i = sync0R;
	//printf("Sync0received: %d\n", i);
	sync0R = 0;
	return i;
}

static int Sync1received(){
	int i = sync1R;
	//printf("Sync1received: %d\n", i);
	sync1R = 0;
	return i;
}

static int helloReceived(){
	int i = helloR;
	helloR = 0;
	return i;
}

static int haveCommand(){
	return nCommand;
}

static int closeC(){
	return closeAll;
}

static int checkContador(){//REVISAR
	return 0;
}

/////////////////////////////RXRXRXRXRXRXRXRXRX


static int always(){
	return 1;
}
 

/*
Sirve para recibir los paquetes: comprueba cabeceras y checksum.
Si no son correctos, descarta el paquete y espera al siguiente (hasta que recibe
otra vez 250 - 251)
*/
static int read_pkg(){
  //printf("Empezamos la recepción\n");
	unsigned char header_0 = 0xFA;
	unsigned char header_1 = 0xFB;
	unsigned short int checksum = 0;
	unsigned char rx_buffer[BUF_LENGTH];
	int i;
	
	
	int leidos = 0;
	int contador = 0;
	

	int pLength = BUF_LENGTH;
	while(contador < pLength){
	  	if((leidos = read(descUart, (void*)(rx_buffer+contador), 1)>0)){
			contador = leidos + contador;
			//printf("Contador: %d  Leidos: %d\nVariable: %d\n", contador, leidos, recBuffer[contador]);
			//printf("Contador: %d  Leidos: %d\nVariable: %d\n", contador, leidos, recBuffer[contador]);
			//printf(" %d\n ", rx_buffer[contador-1]);
		}
		
		
		if(contador == 1){
			if(*(rx_buffer) != header_0){
				//printf("Error en la cabecera");
				return -1;
			}
		}
		if(contador == 2){
			if(*(rx_buffer+1) != header_1){
				//printf("Error en la cabecera");
				return -1;
			}
		}
		if(contador == 3){
		        pLength = *(rx_buffer+2) + 3;
		        //printf("pLength: %d %d\n", pLength, recBuffer[2]);
		}
		
	}
	checksum = get_int(rx_buffer+pLength-2);
	if(checksum != calcCheckSum(rx_buffer)){
	  //printf(" %d\n",calcCheckSum(rx_buffer));
		//printf("Error en el checksum\n");
		//return -1;
	}

	recBuffer[0] = pLength - 2;
	for (i = 0; i < recBuffer[0]; i++){
		recBuffer[i+1] = rx_buffer[i+3];
	}

	//printf("hahala\n");
	
	return pLength;
}



//init() es la misma

static int readSync0(){
	int leidos;
	//printf("ok readSync0\n");
	if(((leidos = read_pkg())!=6) | (recBuffer[1]!=0)){
	  //printf("Error al recibirla sync0\n");
	  return 0;
	}
	return 1;
}

static int readSync1(){
	int leidos;
	//printf("ok readSync1\n");

	//Comprueba por encima si es un sync0
	if(((leidos = read_pkg())!=6) | (recBuffer[1]!=1)){
		return 0;
	}

	return 1;
}

static int readHello(){
	int leidos;
	if((leidos = read_pkg())!=36)
		return 0;

	return 1;
}

static int readSIP(){
	int leidos;
	if ((leidos = read_pkg())>0)
		return 1;
	return 0;
}

static int reload(){
	return reload_RX;
}


/////////////////////////////

static void okSync0(){
	sync0R = 1;
	//printf("ACTUALIZO sync0\n");
}

static void okSync1(){
	sync1R = 1;
	//printf("ACTUALIZO sync1\n");
}

static void okHello(){
	helloR = 1;
}

static void resetRX(){
	reload_RX = 0;
}

void execTX(){
  //	printf("ESTADO %d",*TX_fsm->current_state);
	fsm_fire(TX_fsm);
}

void execRX(){
	fsm_fire(RX_fsm);
}


struct fsm_trans_t tt_TX[] = {
	{IDLE_TX, init, SYNC0_TX, sendSync0} ,
	{SYNC0_TX, Sync0received, SYNC1_TX, sendSync1} ,
	{SYNC0_TX, checkContador, IDLE_TX, reloadRX} ,
	{SYNC1_TX, Sync1received, SYNC2_TX, sendSync2} ,
	{SYNC1_TX, checkContador, IDLE_TX, reloadRX} ,
	{SYNC2_TX, helloReceived, OPEN_TX, sendOpen} ,
	{OPEN_TX, always, MOTORS_TX, sendHabilitateMotors} ,
	{MOTORS_TX, always, SEND_TX, setReady} ,
	{SEND_TX, haveCommand, SEND_TX, sendCommand} ,
	{SEND_TX, closeC, IDLE_TX, sendClose} ,
	{SEND_TX, always, SEND_TX, sendPulse} ,
	{-1, NULL, -1, NULL} ,
};


struct fsm_trans_t tt_RX[] = {
	{IDLE_RX, init, SYNC0_RX, doNothing} ,
	{SYNC0_RX, readSync0, SYNC1_RX, okSync0} ,
	{SYNC0_RX, reload, IDLE_RX, resetRX} ,
	{SYNC1_RX, readSync1, HELLO_RX, okSync1} ,
	{SYNC1_RX, reload, IDLE_RX, resetRX} ,
	{HELLO_RX, readHello, RECEIVE_RX, okHello} ,
	{RECEIVE_RX, readSIP, RECEIVE_RX, doNothing} ,
	{RECEIVE_RX, reload, IDLE_RX, resetRX} ,
	{-1, NULL, -1, NULL} ,
};

/*

*/
void init_connection(void){
	//Descriptor de fichero para lectura/escritura en el puerto serie
	descUart = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NONBLOCK); 
	if (descUart == -1){
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}
	//Configuración del puerto serie (velocidad de transmisión, control de flujo,...)
	struct termios options;
	tcgetattr(descUart, &options);
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = IXOFF;
	options.c_lflag = 0;

	tcflush(descUart, TCIFLUSH);
	tcsetattr(descUart, TCSANOW, &options);

	initR = 1;


	TX_fsm = fsm_new(tt_TX);
	RX_fsm = fsm_new(tt_RX);
	//printf("Iniciando conexion\n");
}



