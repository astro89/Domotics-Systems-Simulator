#define _SVID_SOURCE

#include <stdio.h>			/*	printf	scanf	sprintf	sscanf	perror	getchar	EOF	FILENAME_MAX	*/
#include <stdlib.h>			/*	exit	atoi	*/
#include <errno.h>			/*	nomi simbolici codici di errore:	errno	EBUSY	*/
#include <string.h>			/*	strlen	strcmp	strcpy	strcat	*/
#include <pthread.h>		/* 	gestione thread	*/
#include <unistd.h>			/*	sleep read closed	*/
#include <signal.h>			/*	signal	SIGINT	*/
#include <sys/types.h>	/*	tipi dei dati di sistema	*/
#include <sys/socket.h>	/*	socket	*/
#include <netinet/in.h> /*	inet - ip4	*/
#include <arpa/inet.h>	/*	inet_aton	*/
#include <semaphore.h>	/*	getione semafori	*/
#include <fcntl.h>			/*	open	O_APPEND	*/
#include <syslog.h>			/*	file log	*/
#include <ctype.h>			/*	isupper	isdigit	*/
#include <stdio_ext.h>	/*	fpurge	*/
#include <sys/ioctl.h>	/*	ioctl	*/
#include <linux/if.h>		/*	struct ifconf	*/
#include <sys/stat.h>		/*	stat	*/
#include <math.h>
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gtk/gtk.h>

/*	Constants	*/

#define VERSION "1.5"
/* abilita/disabilita visualizzazione dei valori dei semafori e del mutex	*/
#define debug_sem 0
#define debug_socket 0
#define debug 0

#define DIM_PROMT 80
#define DIM_STR 160
#define DIM_NAME 		50
#define DIM_ROOM 		16
#define END_ARRAY		-21

#define PORT 			8080
#define SECOND_FOR_SAVE_STRUCT 10
#define TEMPO_CLIENT 1	/*	tempo per controllo socket in secondi	*/
#define	WARNING	"WARNING: the hvac hasn't enough power, it's strongly\nrecommend to turn off unuseful lights\n"
#define HISTORY_FILE "history.txt"
#define USER_MANUAL_FILE "user_manual.txt"
#define SOURCE_PROGRAM "engine_server.c"

#define DOOR "door"
#define HVAC "hvac"
#define WEATHER "weather"
#define HVAC_POWER "hvac-power"
#define INT_HUMIDITY "internal-humidity"
#define EXT_HUMIDITY "external-humidity"
#define INT_TEMP "internal-temp"
#define EXT_TEMP "external-temp"
#define FLOOR_TEMP "floor-temp"
#define MIN_TEMP "min-temp"
#define DES_TEMP "des-temp"

#define PRESENCE "presence"
#define POWER "power"
#define WINDOW "window"
#define LIGHT "light"
#define WINTER "winter"
#define SUMMER "summer"

#define CLOSE "close"
#define OPEN "open"

#define CLOSED "closed"
#define OPENED "opened"
#define OFF "off"
#define ON "on"
#define NOBODY "nobody"
#define SOMEBODY "somebody"

/*	Enumerations	*/

enum {false, true};
enum {no, yes};
enum {off, on};
enum {closed, opened};
enum {winter, summer};
enum {INT, STRING, DOUBLE};

/*	Gui define	*/
#define CC_NUM 11
enum {door, hvac, hvac_pw, weather, ih, eh, it, et, ft, mt, dt,
	name, pr, li, wi, iface};
enum {radio, watt, percento, gradi};

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

/*
 * Il numero massimo di luci e finestre è uguale alla metà
 * del massimo numero di stanze.
 * Tale limitazione è stata ritenuta sufficientemente ragionevole.
 */

typedef struct stardard_room {
	char name[DIM_NAME];
	int number;
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
	int des_temp;
	int n_room;
	std_room rooms[DIM_ROOM];
}home;

typedef struct
{
	int tipo;
	union{
		int int_value;
		double double_value;
		char string_value[DIM_NAME];
		char * char_value;
	}union_value;
}struct_value;

void socket_create(char * ip);
int reconnect(int *);
int window_iface(void);
