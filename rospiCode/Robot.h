

void initRobot(void);			//Inicia la conexión con el robot, y habilita los motores
													//Nota: tarda como mucho 2 segundos en habilitar los motores, opciones:
													//  - Contador externo
													//  - Utilizar getMotorState() para comprobar si está habilitado (1) o no (0)

void closeRobot(void);		//Cierra la conexión con el robot



int getSonar(int sonar);	//Devuelve la distancia medida en cualquiera de los 16 sonar

unsigned char getFrontBumpers();		//Devuelve el estado de los bumpers delanteros (bits menos significativos, orden del manual)

unsigned char getRearBumpers();		//Devuelve el estado de los bumpers traseros (bits menos significativos, orden del manual)

int getMotorState();			//Indica si el motor está habilitado (1) o no (0);

int getXPos();						//Devuelve la posición X del robot (mm)

int getYPos();						//Devuelve la posición Y del robot (mm)

int getOrientation();			//Devuelve la orientación del robot (º)



void setRotVel(int i);		//Establece la velocidad de giro del robot (º/s)

void setVel(int i);				//Establece la velocidad del robot (mm/s)

void setVelM(int i);				//Establece la velocidad maxima del robot (mm/s)

void setAccel(int i);			//Establece la aceleración del robot (mm/s2)

void setRotAccel(int i);		//Establece la aceleración del giro del robot (º/s2)

void stop(void);					//Detiene el movimiento del robot inmediatamente. No deshabilita los motores.

void refresh_sonar(void);	//Acualiza estado de los sonars

unsigned char getCompass(void); //Devuelve estado brújula interna

void midi(unsigned char* arg);			//Reproduce el argumento midi (más info en el manual, "BUZZER SOUNDS")








