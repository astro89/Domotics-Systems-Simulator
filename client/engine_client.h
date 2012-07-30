#define _SVID_SOURCE

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>			/*	isupper	isdigit	*/
#include <math.h>
#include <syslog.h>


// Constants

#define DIM_NAME 		50
#define DIM_ROOM 		20
#define MAX_WATT		3000
#define WATT_INCR		100
#define LIGHT_WATT		60
#define MAX_EXCURSION	50
#define N				2
#define END_ARRAY		-21
#define LOCAL_HOST		"127.0.0.1"
#define MAX_GV_LENGHT	(12 + DIM_ROOM * ( DIM_ROOM + 4) + 1)
#define N_THREADS		3
#define MAX_TRY			100


// Enumerations

enum{false,true};
enum{off,on};
enum{closed,opened};
enum{winter,summer};
enum{Integer,String};
enum{no,yes};

// Functions

int parse_input(char *);
int write_output(char *);
int setValues(int *);
int initializeGlobalValues();
int getGlobalValuesLenght();
void * processCheckRugTemp(void *);
void * processCheckLight(void *);
void * processCheckTemp(void *);
int printHome();
int printDetailedHome();
int printHelp();
int getRoomName(int,int);
void sendMessage(int *);
int receiveMessage(int *);
int createSocketByAddress(char*, int);
int createSocketByIP(char*, int);
int printManualOnShell(char *);
int printUserManual(char *,char *);
int tryToReconnect(int);
void closeSocket();
int checkPowerNeeded();
int reducePowerNeeded();
int difference(int,int);


// Globally utilized values

int * globalValues;
int id_client; // Descrittore di File sul mio HD ( client)
int id_server; // Descrittore di File sull'HD dell'altra macchina( client)
extern int errno;
pthread_mutex_t mutex_id;

int _time,__time,_try,_bool,_warning,load_data,optimal_temp,wrong_weather,switch_weather,client_autenthicated;
int maxEscursion;
float dehum_Effect; // moltiplicatore umidità
float temp_Effect; // moltiplicatore temperatura
float c_h_mult; // moltiplicatore per aumentare/diminuire la temperatura interna


char * _IP;
int PORT;

/*
 * Il numero massimo di luci e finestre è uguale alla metà
 * del massimo numero di stanze.
 * Tale limitazione è stata ritenuta sufficientemente ragionevole.
 */


typedef struct standard_room {
	char name[DIM_NAME];
	int presence;
	int power;
	int n_windows;
	int n_lights;
	int windows[(DIM_ROOM)/2];
	int lights[(DIM_ROOM)/2];
}std_room;

struct standard_home {
	int door;
	int hvac;
	int hvac_power;
	int weather;
	int internal_humidity;
	int external_humidity;
	int internal_temp;
	int external_temp;
	int floor_temp;
	int min_temp;
	int desired_temp;
	int n_room;
	std_room rooms[DIM_ROOM];
}home;
