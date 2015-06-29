#ifndef SCREEN_H
#define SCREEN_H

void screen_setup (int prio);
void screen_refresh (void);

void screen_clear (void);
void screen_printxy (int x, int y, const char* txt);
int screen_getchar (void);

void set_enableBumpers(int value);
void set_enableSonars(int value);
void printBumpers();
void printSonars();
void printTitle();


#endif
