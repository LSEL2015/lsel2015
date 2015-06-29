#ifndef TESTS_H
#define TESTS_H

int run_testMotors(int direction); //Ejecuta el test correspondiente a moverse 
																	//a velocidad baja, media y alta durante 3 segundos hacia adelante y hacia atrás.

int run_testBumpers(int direct, int hits); //Ejecuta test Bumpers.
																					// El robot cambia de dirección tantas veces como choques se indiquen (hits) 
																					//indicádole una dirección inicial (direction).

#endif

