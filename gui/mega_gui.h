/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gui.c
 * Copyright (C) simone serafini 2010 <astro.simone@gmail.com>
 * 				 matteo micheletti 2010 <kingletti88@gmail.com>
 * 
 * guiHome is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * guiHome is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



// INCLUSIONS

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
#include <semaphore.h>	/*	getione semafori*/
#include <fcntl.h>			/*	open	O_APPEND	*/
#include <syslog.h>			/*	file log	*/
#include <ctype.h>			/*	isupper	isdigit	*/
#include <sys/wait.h>		/*	waitpid	*/
#include <stdio_ext.h>	/*	fpurge	*/
#include <sys/ioctl.h>	/*	ioctl	*/
#include <linux/if.h>		/*	struct ifconf	*/
#include <sys/stat.h>		/*	stat	*/
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <glib/gprintf.h>///////
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gtk/gtk.h>
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



//	CONSTANTS

#define _GNU_SOURCE
#define CC_NUM 		11
#define MAX_TRY 	100
#define MAX_ROOMS 	16
#define OFFSET		150
#define VERSION "1.5"
/* abilita/disabilita visualizzazione dei valori dei semafori e del mutex	*/
#define debug_sem 0

#define DIM_PROMT 80
#define DIM_STR 160
#define DIM_NAME 		50
#define DIM_ROOM 		20
#define END_ARRAY		-21

#define SECOND_FOR_SAVE_STRUCT 10
#define TEMPO_CLIENT 1	/*	tempo per controllo socket in secondi	*/
#define	WARNING	"WARNING: the hvac hasn't enough power, it's strongly\nrecommend to turn off unuseful lights\n"
#define CONFIG_FILE "home.conf"
#define HISTORY_FILE "history.txt"
#define USER_MANUAL_FILE "user_manual.txt"
#define SOURCE_PROGRAM "engine server.c"

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

#define CLOSE "closed"
#define OPEN "opened"
#define OFF "off"
#define ON "on"
#define NOBODY "nobody"
#define SOMEBODY "somebody"

#define PRIMA_IFACE "eth0"
#define ALT1_IFACE "wlan0"



// ENUMERATIONS

enum {INT_ORIZ,META_VERT,QUART_ORIZ,QUART_VERT,OTTAV_VERT};
enum {door, hvac, hvac_pw, weather, ih, eh, it, et, ft, mt, dt, name, pr, li, wi};
enum {radio, watt, percento, gradi};
enum {false, true};
enum {no, yes};
enum {off, on};
enum {closed, opened};
enum {winter, summer};
enum {INT, STRING, DOUBLE};



// GLOBALLY UTILIZED VARIABLES


int fd = -1, config_file_done = -1, margin = 5, n_li = 0, n_wi = 0, if_room = 0,
	num_select = -1;
gint int_tmp_timer,ext_tmp_timer,flr_tmp_timer,int_hum_timer,ext_hum_timer,draw_timer,prt_roms_timer,gen_wth_pw_timer;
gint context_id;
char *radio_names[4][2], *filename;
double dvalue = -1.0;
GtkWidget *vbox, **widget_li, **widget_wi, *table;
GtkWidget *input_mask,*entry_ip,*entry_door, *entry_pwd;
gboolean *nm_drawed,*pw_drawed;
int *values,initialized;
int id_client; // Descrittore di File sul mio HD ( client)
int id_server; // Descrittore di File sull'HD dell'altra macchina( client)
int gui_connected;
int _try,_DOOR;
char * _IP;
char * password;


// STRUCTS

/*
 * Struct contenente i valori che serviranno alla gui per disegnare 
 * le stanze della home
 * 
 */

typedef struct graphic_room {
	
	int init;
	int gr_light_dim;		//float?	//lato del quadrato che conterrà la luce
	int gr_window_width;	//float? 	//larghezza della singola finestra
	int gr_width;			//float?	//larghezza della room
	int gr_heigth;			//float?	//altezza della rooom
	int gr_TYPE;						//layout della room,uno dei tipi di enum
	int gr_pointx1;						
	int gr_pointy1;						//punto in alto a sinistra della room 
	int gr_pointx2;						//(iniziale della linea, per piu di 1 stanza)
	int gr_pointy2;						//e punto finale della linea(per piu di 1 stanza)
	int pw_bak;
	char nm_bak[DIM_NAME];
										
										
}g_room;

g_room * g_rooms;

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

struct_value * cc, * struct_li, * struct_wi, gen_room[2];



//FUNCTIONS

void menuitem_response(gchar*);
int connection(char *,int);
int tryToReconnect(int);
int receiveMessage(int*);
int reconnect(void);
