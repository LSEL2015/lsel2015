#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include "screen.h"
#include "tasks.h"
#include "Robot.h"

#define TS 4 /* TITLE Y SIZE OF THE ROSPI APPLICATION */
#define SS 20 /* SONAR ZONE SIZE */
#define BS 10 /* BUMPER ZONE SIZE */

#define N_BUMPERS 5 /* NUMBER OF BUMPERS */
#define N_SONARS 16 /* NUMBER OF SONARS */

int enableSonars = 0;
int enableBumpers = 0;

static void *refresh_screen (void* arg) {
	struct timeval next_activation;
	struct timeval now, timeout, rtime;
	
	gettimeofday (&next_activation, NULL);
	while (1) {
		struct timeval *period = task_get_period (pthread_self());
		timeval_add (&next_activation, &next_activation, period);
		gettimeofday (&now, NULL);
		timeval_sub (&timeout, &next_activation, &now);
		timeval_sub (&rtime, period, &timeout);
		task_register_time (pthread_self(), &rtime);
		select (0, NULL, NULL, NULL, &timeout);
		
		printTitle();
		
		if(enableSonars == 1) {
			printSonars();
		}
		if(enableBumpers == 1) {
			printBumpers();
		}
		
		screen_refresh();
	}
}


static struct termios oldtc, newtc;
static char* screen;
static int columns;
static int lines;

static pthread_t t_screen;
static pthread_mutex_t m_scr;

static char*
scr (int x, int y) {
	if (x >= columns)
		x = columns - 1;
	if (y >= lines)
		y = lines - 1;
	return screen + y * (columns + 1) + x;
}

static
int getenv_int (const char *var, int defval) {
	char* val = getenv (var);
	return val? atoi(val) : defval;
}

void
screen_setup (int prio)
{
	columns = getenv_int ("COLUMNS", 120);
	lines = getenv_int ("LINES", (TS+SS+BS)*2) / 2;
	
	mutex_init (&m_scr, prio);
	screen = (char *) malloc ((columns + 1) * lines);
	screen_clear ();
	
	printf ("\e[2J\e[%d;1f", lines + 1);
	fflush (stdout);
	
	tcgetattr(0, &oldtc);
	newtc = oldtc;
	newtc.c_lflag &= ~ICANON;
	newtc.c_lflag |= ECHO;
	tcsetattr(0, TCSANOW, &newtc);
	
	t_screen = task_new ("screen", refresh_screen, 500, 500, 1, 1024);
}

void
screen_refresh (void) {
  int y;

  printf ("\e7\e[?25l");

  pthread_mutex_lock (&m_scr);
  for (y = 0; y < lines; ++y) {
    printf ("\e[%d;1f%s", y+1, scr(0,y));
  }
  pthread_mutex_unlock (&m_scr);

  printf ("\e8\e[?25h");
  fflush (stdout);
}

void
screen_clear (void)
{
  int y;

  pthread_mutex_lock (&m_scr);
  memset (screen, ' ', (columns + 1) * lines);
  for (y = 0; y < lines; ++y)
    *scr(columns, y) = '\0';
  pthread_mutex_unlock (&m_scr);
}

void
screen_printxy (int x, int y, const char* txt)
{
  char* p = scr(x,y); 
  pthread_mutex_lock (&m_scr);
  while (*txt) {
    *p++ = *txt++;
  }
  pthread_mutex_unlock (&m_scr);
}

int
screen_getchar (void)
{
  fd_set rds;
  struct timeval t = {0, 0};
  int ch = 0;
  
  FD_ZERO (&rds);
  FD_SET (0, &rds);
  pthread_mutex_lock (&m_scr);
  if (select (1, &rds, NULL, NULL, &t) > 0) {
    ch = getchar();
  }
  pthread_mutex_unlock (&m_scr);
  return ch;
}

void printBumpers(){		
	
	int x, y;		
	int k = TS+SS;		
	int frontBumpers[5];		
	int rearBumpers[5];		
	int front, rear;		
	
	front = getFrontBumpers();		
	rear = getRearBumpers();		
	
	for(x = 0; x < 5; x++){	
		y = ((front >> x) & 1);		
		frontBumpers[x] = y;		
		
		y = ((rear >> x) & 1);		
		rearBumpers[x] = y;		
		
	}		
	
	for(y = 0; y < 5; y++){		
		for(x = 0; x < 5; x++){		
			if(frontBumpers[x] == 0){		
				screen_printxy(6*x, y+k, " ");		
				screen_printxy(6*x+1, y+k, "_");		
				screen_printxy(6*x+2, y+k, "_");		
				screen_printxy(6*x+3, y+k, "_");		
				screen_printxy(6*x+4, y+k, "_");		
				screen_printxy(6*x+5, y+k, " ");		
			}		
			else{		
				screen_printxy(6*x, y+k, " ");		
				screen_printxy(6*x+1, y+k, " ");		
				screen_printxy(6*x+2, y+k, " ");		
				screen_printxy(6*x+3, y+k, " ");		
				screen_printxy(6*x+4, y+k, " ");		
				screen_printxy(6*x+5, y+k, " ");		
			}		
		}		
	}		
	
	for(y = 0; y < 5; y++){		
		screen_printxy(6*x, y+k, " ");		
		screen_printxy(6*x+1, y+k, " ");		
		screen_printxy(6*x+2, y+k, " ");		
		screen_printxy(6*x+3, y+k, " ");		
		screen_printxy(6*x+4, y+k, " ");		
		screen_printxy(6*x+5, y+k, " ");		
	}		
	
	for(y = 0; y < 5; y++){		
		for(x = 0; x < 5; x++){		
			if(rearBumpers[x] == 0){		
				screen_printxy(6*(x+6), y+k, " ");		
				screen_printxy(6*(x+6)+1, y+k, "_");		
				screen_printxy(6*(x+6)+2, y+k, "_");		
				screen_printxy(6*(x+6)+3, y+k, "_");		
				screen_printxy(6*(x+6)+4, y+k, "_");		
				screen_printxy(6*(x+6)+5, y+k, " ");		
			}		
			else{		
				screen_printxy(6*(x+6), y+k, " ");		
				screen_printxy(6*(x+6)+1, y+k, " ");		
				screen_printxy(6*(x+6)+2, y+k, " ");		
				screen_printxy(6*(x+6)+3, y+k, " ");		
				screen_printxy(6*(x+6)+4, y+k, " ");		
				screen_printxy(6*(x+6)+5, y+k, " ");		
			}		
		}		
	}                         
	screen_printxy(0,y+k+1, "^^^      FRONT BUMPERS      ^^^ || ^^^       REAR BUMPERS       ^^^");
}		

void printSonars(){		
	int sonar[N_SONARS];		
	int i;		
	for(i = 0; i < N_SONARS; i++) {		
		sonar[i] = getSonar(i);		
	}		
	
	int x, y;
	screen_printxy(0,TS, "))))   FRONT SONAR   ((((");
	for(y = 0; y < 8; y++) {
		for(x= 0; x < 50; x++) {		
			if(sonar[y]/100 > x)		
				screen_printxy(x, y+TS+1, "|");		
			else		
				screen_printxy(x, y+TS+1, " ");		
		}
	}
	screen_printxy(0,y+TS+1, "))))   REAR SONAR   ((((");
	for(y = 9; y < N_SONARS+2 ; y++) {
		for(x= 0; x < 50; x++) {		
			if(sonar[y-1]/100 > x)		
				screen_printxy(x, y+TS+1, "|");		
			else		
				screen_printxy(x, y+TS+1, " ");		
		}
	}
}

void set_enableBumpers(int value) {
	enableBumpers = value;
}


void set_enableSonars(int value) {
	enableSonars = value;
}

void printTitle() {
	screen_printxy(0,0, "********************************************************************************");
	screen_printxy(0,1, "*                          ROSPI APPLICATION MANAGER                           *");
	screen_printxy(0,2, "********************************************************************************");
	screen_printxy(0,3, "                                                                                ");	
}






#ifdef TEST

int
main ()
{
	screen_init (1);
	return 0;
}

#endif
