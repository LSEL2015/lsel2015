#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#define BUF_LENGTH 256
#define MAX_COMMAND 5

void init_connection(void);	//Inicia el proceso de conexión

void close_connection(void);	//Cierra la conexión

int ready;										//Variable booleana, 1 si hay conexión (en caso de caída, se tomarán medidas a niveles superiores).
															//Nota: sin probar, por ahora no la usamos

char* sendBuff[MAX_COMMAND]; 	//Cada uno de los punteros lleva a una cadena de char, el primero es la longitud, luego los datos:
															//Ejemplo: (1, 0)-> Se envía el paquete (250, 251, 3, 0, 0, 0). 
															//Construcción del paquete: (cabecera, tamaño datos + checksum, datos, checksum)

int nCommand;

char* SIP;

void execTX(void);	//Ejecuta fsm_fire (ver fsm.c) de la máquina de estados TX
void execRX(void);	//Ejecuta fsm_fire de la máquina de estados RX

#endif
