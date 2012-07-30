////////////////////////////////////////////////////////////////////////////////
// 					    __         __             __						  //
//					   |__  |\ |  | __  |  |\ |  |__						  //
//					   |__  | \|  |__|  |  | \|  |__						  //
//																			  //
////////////////////////////////////////////////////////////////////////////////
//      																	  //
//		engine_client.c														  //
//																			  //
//		Version 1.4															  //
//      																	  //
//      Copyright 2010 Matteo Micheletti									  //
//      																	  //
//      This program is free software; you can redistribute it and/or modify  //
//      it under the terms of the GNU General Public License as published by  //
//      the Free Software Foundation; either version 2 of the License, or	  //
//      (at your option) any later version.									  //
//      																	  //
//      This program is distributed in the hope that it will be useful,		  //
//      but WITHOUT ANY WARRANTY; without even the implied warranty of		  //
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		  //
//      GNU General Public License for more details.						  //
//      																	  //
//      You should have received a copy of the GNU General Public License	  //
//      along with this program; if not, write to the Free Software			  //
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,			  //
//      MA 02110-1301, USA.													  //
//																			  //
////////////////////////////////////////////////////////////////////////////////



#include "engine_client.h"


/*
 * ---------------------------- SEND SOCKET TYPE -------------------------------
 * 
 * Informa il server di essere un Client, inviandogli l'informazione attraverso 
 * una stringa.
 *
 */


int send_socket_type(int socket)
{
	int byte_tot = 0, byte_count, byte, * array_value;
	char * socket_type = (char *){"Client"};
	
	byte_tot = strlen(socket_type);
	array_value = calloc(2, sizeof(int));
	array_value[1] = byte_tot;
	
	byte_count = 0;
	while(byte_count < 2 *sizeof(int))
		if((byte = write(socket, array_value, 2 * sizeof(int) - byte_count)) > 0){
			syslog(LOG_DEBUG, "Write socket type len con successo\n");
			byte_count += byte;
		}
		else if(errno != EINTR){
			perror("Client socket type len ");
			return -1;
		}
	byte_tot *= sizeof(char);
	byte_count = 0;
	while(byte_count < byte_tot)
		if((byte = write(socket, socket_type, byte_tot - byte_count)) > 0){
			syslog(LOG_DEBUG, "Write socket type con successo\n");
			byte_count += byte;
			socket_type += byte;
		}
		else if(errno != EINTR){
			perror("Client socket type");
			return -1;
		}
	free(array_value);
	return 0;
}


/*
 * ------------------------------ SEND PASSWORD --------------------------------
 * 
 * Tenta di effetturare il login verso il Server tramite l'invio di una password.
 *
 */


int send_password(int socket)
{
	int byte_tot = 0, byte_count, byte, * array_value;
	char *password_copy, *password = calloc(DIM_NAME, sizeof(char));
	
	printf("Inserisci la password:");
	scanf("%s", password);
	byte_tot = strlen(password);
	array_value = calloc(2, sizeof(int));
	array_value[1] = byte_tot;
	
	byte_count = 0;
	while(byte_count < 2 *sizeof(int))
		if((byte = write(socket, array_value, 2 * sizeof(int) - byte_count)) > 0){
			syslog(LOG_DEBUG, "Write password len con successo\n");
			byte_count += byte;
		}
		else if(errno != EINTR){
			perror("Client password len ");
			return -1;
		}
	byte_tot *= sizeof(char);
	byte_count = 0;
	
	password_copy = password;
	while(byte_count < byte_tot)
		if((byte = write(socket, password, byte_tot - byte_count)) > 0){
			syslog(LOG_DEBUG, "Write password con successo\n");
			byte_count += byte;
			password += byte;
		}
		else if(errno != EINTR){
			perror("Client password");
			return -1;
		}
	password = password_copy;
	free(password);
	free(array_value);
	return 0;
}


/*
 * -------------------------- RECEIVE SOCKET REPORT ----------------------------
 * 
 * Attende la risposta del Server riguardo il suo ultimo tentativo di login;
 * se il responso è affermativo, parte la comunicazione con il Server.
 *
 */


int receive_socket_report(int socket)
{
	int byte_tot = 0, byte_count, byte, * array_value;
	char * esito, * esito_copy;
	
	array_value = calloc(2, sizeof(int));
	byte_count = 0;
	while(byte_count < 2 *sizeof(int))
		if((byte = read(socket, array_value, 2 * sizeof(int) - byte_count)) > 0){
			syslog(LOG_DEBUG, "Read esito len con successo\n");
			byte_count += byte;
		}
		else if(errno != EINTR){
			perror("Client esito len ");
			return -1;
		}
	if(!array_value[no]){
		byte_tot = array_value[yes] * sizeof(char);
		esito = calloc(byte_tot, sizeof(char));
		esito_copy = esito;
		byte_count = 0;
		while(byte_count < byte_tot)
			if((byte = read(socket, esito, byte_tot - byte_count)) > 0){
				syslog(LOG_DEBUG, "Read esito con successo\n");
				byte_count += byte;
				esito += byte;
			}
			else if(errno != EINTR){
				perror("Client esito");
				return -1;
			}
		esito = esito_copy;
		printf("esito %s\n", esito);
		if(strcmp(esito, "Yes") == 0)
			client_autenthicated = true;
		else{
			close(socket);
			free(esito);
			return -1;
		}
		free(esito);
	}
	free(array_value);
	return 0;
}


/*
 * --------------------------- SEND AUTENTICATION ------------------------------
 * 
 * Utilizza la funzione SEND SOCKET TYPE per l'autenticazione col Server.
 *
 */


int send_autentication(int socket){
	register int esito;
	if(!send_socket_type(socket))
		send_password(socket);
	esito = receive_socket_report(socket);
	return esito;
}

/*
 * ----------------------------------- MAIN ------------------------------------
 * 
 * Vengono creati il socket e i thread che girano in concorrenza.
 *
 */

int main(int argc, char** argv)
{
	
	optimal_temp		= 0;
	wrong_weather		= 0;
	switch_weather		= false;
	_bool 				= false;
	_warning			= false;
	load_data			= false;
	client_autenthicated= false;
	dehum_Effect		= 0.001;
	temp_Effect 		= 0.003;
	c_h_mult 			= 0.02;
	_time 				= 0;
	__time 				= 0;
	
	_IP = calloc(16,sizeof(char));

	int i,j;
	

	signal(SIGPIPE,SIG_IGN);
	

	for ( i = 1; i < argc; i++)
	{
		j = 0;

		if (strcmp(argv[i], "man") == 0)
		{
			printManualOnShell("./engine_client.c");

			if(argc <= i + 1)
				exit(0);
		}
		else if (strcmp(argv[i], "usman") == 0)
		{
			printUserManual("./engine_client.c","./user_manual_engine.txt");
			
			if(argc <= i + 1)
				exit(0);
		}
		else if (strcmp(argv[i], "help") == 0)
		{
			printHelp();

			if(argc <= i + 1)
				exit(0);
		}

	}
	
	do
	{
		printf("\nInserire un indirizzo ip valido:\n");
		
		fflush(stdout);
	
		while(scanf("%s",_IP) < 0)
		{
			printf("\nInserire un indirizzo ip valido:\n");
			
			fflush(stdout);
		}
		
		printf("Inserire una porta valida:\n");
		
		fflush(stdout);
	
		while(scanf("%d",&PORT) < 0)
		{
			printf("Inserire una porta valida:\n");
			
			fflush(stdout);
		}
	}
	while(createSocketByIP(_IP,PORT) < 0);
	
	
		
	pthread_mutex_init(&mutex_id,NULL);

	globalValues = calloc(MAX_GV_LENGHT, sizeof(int));
	globalValues[MAX_GV_LENGHT - 1] = END_ARRAY;

	pthread_t treads[N_THREADS];
	void * ret_val;
	
	
	pthread_create(&(treads[0]),NULL,processCheckRugTemp,NULL);
	pthread_create(&(treads[1]),NULL,processCheckLight,NULL);
	pthread_create(&(treads[2]),NULL,processCheckTemp,NULL);

	for(i = 0; i < N_THREADS; i++)
	{
		pthread_join(treads[i], &ret_val);
		
		if(ret_val != 0)
		{
			perror("\nerror \n");
			return 0;
		}
	}

	pthread_mutex_destroy(&mutex_id);
	
	free(globalValues);

	return 0;
}


/*
 * ----------------------------------- HELP ------------------------------------
 * 
 * Stampa l'aiuto di questo programma.
 *
 */


int printHelp()
{
	printf("\nEngine version 1.4\n\n\n");
	printf("\nThis program purpose is to manage an house whit a domotic\n");
	printf("system.\n\n\n");
	printf("\nThank you for using this program, here is the help:\n");
	printf("\nUsage:\n");
	printf("\n./path/engine [OPTION]\n");
	printf("\nhelp:	prints this help\n");
	printf("\nman:	print a detailed manual that explain this program work\n");
	printf("\nusman:	same as 'man', but writes the manual in a txt FILE\n");
	printf("	you can find into your current directory.\n");
	printf("\n\nReport bugs to: kingletti88@gmail.com\n\n");

	return 0;
}


/*
 * ---------------------------------- MANUAL -----------------------------------
 * 
 * PARAMETRI:
 * 			
 * 		filename	:char*  path dove risiede questo programma
 * 
 * Stampa su shell i commenti del codice.
 *
 */


int printManualOnShell(char * filename)
{

	FILE * fSource = fopen(filename, "r"); 

	if (fSource == NULL)
	{
		printf("\nError opening the file, code: ");
		exit(1);
	}
	else
	{
		char c;
		int bool = 0;

		while(!feof(fSource))
		{
			if(fscanf(fSource,"%c", &c) != EOF && c == '/')
			{
				if(fscanf(fSource,"%c", &c) != EOF && c == '*')
				{
					while(fscanf(fSource,"%c", &c) != EOF && !bool)
					{
						if(c == '*')
						{
							if(fscanf(fSource,"%c", &c) != EOF && c == '/')
								bool = 1;
							else
								printf("%c", c);
						}
						else
							printf("%c", c);
					}
					
					bool = 0;
					printf("\n");
				}
			}
		}
		
		fclose(fSource);
		
		printf("\n\n");
	}
	
	return 0;
}


/*
 * ------------------------------ USER'S MANUAL --------------------------------
 * 
 * PARAMETRI:
 * 
 *		source		:char*	file da cui legge il testo da formattare
 * 		destination	:char*	file su cui scriverà il testo formattato
 * 
 * Stampa su un file i commenti del codice.
 *
 */


int printUserManual(char * source,char * destination)
{

	FILE * fSource,* fDestination;

	fSource = fopen(source, "r");
	fDestination = fopen(destination, "w");

	if (fSource == NULL || fDestination == NULL)
	{
		perror("open source or destination");
		exit(1);
	}
	else
	{
		char c;
		int bool = 0;
					
		while(!feof(fSource))
		{
			if(fscanf(fSource,"%c", &c) != EOF && c == '/')
			{
				if(fscanf(fSource,"%c", &c) != EOF && c == '*')
				{
					while(fscanf(fSource,"%c", &c) != EOF && !bool)
					{
						if(c == '*')
						{
							if(fscanf(fSource,"%c", &c) != EOF && c == '/')
								bool = 1;
							else
								fprintf(fDestination,"%c", c);
						}
						else
							fprintf(fDestination,"%c", c);
					}
					
					bool = 0;
					fprintf(fDestination,"\n");
				}
			}
		}
		
		fclose(fSource);
		fclose(fDestination);
		
		printf("%s salvato con successo\n", destination);
	}
	
	return 0;
}


/*
 * ------------------------------- DETAILED HOME -------------------------------
 * 
 * Stampa la Struct_Home in modo dettagliato, compresi i nomi dei parametri, per
 * una più facile lettura.
 *
 */


int printDetailedHome()
{
	
	if(home.door == closed)
		printf("Door: closed\n");
	else
		printf("Door: opened\n");
	if(home.hvac == off)
		printf("Hvac: off\n");
	else
		printf("Hvac: on\n");
	if(home.weather == winter)
		printf("Weather: winter\n");
	else
		printf("Weather: summer\n");
		
	printf("Hvac power: %d W\n", (home.hvac_power));
	printf("Internal umidity: %d %%\n", home.internal_humidity);
	printf("External umidity: %d %%\n", home.external_humidity);
	printf("Internal heat: %3.2f °\n", (home.internal_temp / 10.0));
	printf("External heat: %3.2f °\n", (home.external_temp / 10.0));
	printf("Floor heat: %3.2f °\n", (home.floor_temp / 10.0));
	printf("Min heat: %3.2f °\n", (home.min_temp / 10.0));
	printf("Desired heat: %3.2f °\n", (home.desired_temp / 10.0));

	int r = 0;

	while(r < home.n_room)
	{
		int w = 0,l = 0;

		printf("Room: %s\n", home.rooms[r].name);

		if(home.rooms[r].presence == no)
			printf("	Human presence: no\n");
		else
			printf("	Human presence: yes\n");

		printf("	Room energy: %d\n", home.rooms[r].power);

		while(w < home.rooms[r].n_windows)
		{
			if(home.rooms[r].windows[w] == closed)
				printf("	Window %d is closed\n", (w + 1));
			else
				printf("	Window %d is opened\n", (w + 1));
			w++;
		}

		while(l < home.rooms[r].n_lights)
		{
			if(home.rooms[r].lights[l] == off)
				printf("	Light %d is off\n", (l + 1));
			else
				printf("	Light %d is on for the %d %%\n", (l + 1), home.rooms[r].lights[l]);
			l++;
		}

		r++;
	}

	return 0;
}

/*
 * ----------------------------------- HOME ------------------------------------
 * 
 * Stampa i valori della Struct_Home.
 *
 */


int printHome()
{
	printf("%d ", home.door);
	printf("%d ", home.hvac);
	printf("%d ", home.weather);
	printf("%d ", home.hvac_power);
	printf("%d ", home.internal_humidity);
	printf("%d ", home.external_humidity);
	printf("%d ", home.internal_temp);
	printf("%d ", home.external_temp);
	printf("%d ", home.floor_temp);
	printf("%d ", home.min_temp);
	printf("%d ", home.desired_temp);
	printf("%d ", home.n_room);

	int r = 0;

	while(r < home.n_room)
	{
		int w = 0,l = 0;

		printf("%d ", home.rooms[r].presence);
		printf("%d ", home.rooms[r].power);
		printf("%d ", home.rooms[r].n_windows);

		while(w < home.rooms[r].n_windows)
		{
			printf("%d ", home.rooms[r].windows[w]);
			w++;
		}

		printf("%d ", home.rooms[r].n_lights);

		while(l < home.rooms[r].n_lights)
		{
			printf("%d ", home.rooms[r].lights[l]);
			l++;
		}

		r++;
	}

	return 0;
}

/*
 * ---------------------------- GLOBAL ARRAY LENGHT ----------------------------
 * 
 * Computa la lunghezza dell'array globale contenente i valori da passare nel 
 * socket.
 *
 */


int getGlobalValuesLenght()
{

	int gVlenght;

	for(gVlenght = 0; globalValues[gVlenght] != END_ARRAY; gVlenght++)
		;

	gVlenght++;

	return gVlenght;
}


/*
 * ------------------------------ GET ROOM NAME --------------------------------
 * 
 * PARAMETRI:
 * 
 * 		n_room 		:int	numero della stanza
 * 		exact_value :int	n° di caratteri che formano il nome della stanza 
 * 
 * 
 * Inizializza nella struttura il nome di una determinata stanza.
 *
 */

int getRoomName(int n_room, int exact_value)
{
	int nDati,success = -1;
	_try = 0;

	if ((nDati = read(id_client,home.rooms[n_room].name,exact_value)) < 0)
			tryToReconnect(success);
	else
		;

	return 0;
}


/*
 * ------------------------------- POWER NEEDED --------------------------------
 * 
 * Controlla quanti watt sono necessari al funzionamento degli apparati 
 * elettrici.
 *
 */


int checkPowerNeeded()
{

	int i,j,powNeed;

	if(home.hvac == off)
		powNeed = 0;
	else
		powNeed = home.hvac_power;

	for(i = 0; i < home.n_room; i++)
	{
		home.rooms[i].power = 0;
		  
		for(j = 0; j < home.rooms[i].n_lights; j++)
		{
			if(home.rooms[i].lights[j])
			{
				double fract = home.rooms[i].lights[j] / 100.0;

				home.rooms[i].power += (LIGHT_WATT * fract);
				powNeed += (LIGHT_WATT * fract);
			}
		}
	}
	
	return powNeed;
}


/*
 * ------------------------------- REDUCE POWER --------------------------------
 * 
 * Chiamata quando il consumo di energia eccde quello consentito; controlla 
 * quanti watt sono strettamente necessari al funzionamento degli apparati 
 * elettrici, in modo da ridurre il consumo di quelli secondari.
 *
 */


int reducePowerNeeded()
{
	int i,j;
	
	if(home.hvac == on)
	{
		home.hvac = off;		
		home.hvac_power = 0;
	}

	for(i = 0; i < home.n_room; i++)
	{
			home.rooms[i].power = 0;
					  
			for(j = 0; j < home.rooms[i].n_lights; j++)
			{
				if(home.rooms[i].lights[j])
					home.rooms[i].lights[j] = off;
			}
	}

	__time = 10;
	
	return 0;

}


/*
 * ----------------------------- SET GLOBAL ARRAY ------------------------------
 * 
 * PARAMETRI:
 * 
 * 		values: int* array in input.
 * 
 * 
 * Setta i valori delle Struct standard_room e standard_home ricavandoli da un
 * array in input.
 *
 */

int setValues(int * values)
{

	int i = 0;

	home.door 				= values[i++];
	home.hvac				= values[i++];
	home.weather			= values[i++];
	home.hvac_power			= values[i++];
	home.internal_humidity	= values[i++];
	home.external_humidity	= values[i++];
	home.internal_temp		= values[i++];
	home.external_temp		= values[i++];
	home.floor_temp			= values[i++];
	home.min_temp			= values[i++];
	home.desired_temp		= values[i++];
	home.n_room				= values[i++];
		

	int r = 0;

	while(r < home.n_room)
	{
		int w = 0,l = 0;

		home.rooms[r].presence = values[i++];
		home.rooms[r].power = values[i++];
		home.rooms[r].n_windows = values[i++];

		while(w < home.rooms[r].n_windows)
		{
			home.rooms[r].windows[w] = values[i++];
			w++;
		}

		home.rooms[r].n_lights = values[i++];

		while(l < home.rooms[r].n_lights)
		{
			home.rooms[r].lights[l] = values[i++];
			l++;
		}

		r++;
	}

	return 0;
}

/*
 * ------------------------ GLOBAL ARRAY INIZIALIZATION ------------------------
 * 
 * Legge i valori contentuti nelle Struct standard_room e standard_home e li 
 * utilizza per inizializzare un array di interi di scope globale, che verrà 
 * utilizzato nel Socket per la comunicazione dei cambiamenti di Stato della 
 * casa al Server.
 * Al termine dell'array è stato appeso un valore negativo ( -21 ) utile alla
 * determinazione della sua lunghezza.
 *
 */

int initializeGlobalValues()
{
	int i = 0;

	globalValues[i++] = 	home.door;
	globalValues[i++] =		home.hvac;
	globalValues[i++] =		home.weather;
	globalValues[i++] =		home.hvac_power;
	globalValues[i++] =		home.internal_humidity;
	globalValues[i++] =		home.external_humidity;
	globalValues[i++] =		home.internal_temp;
	globalValues[i++] =		home.external_temp;
	globalValues[i++] =		home.floor_temp;
	globalValues[i++] =		home.min_temp;//mod//
	globalValues[i++] =		home.desired_temp;//mod//
	globalValues[i++] =		home.n_room;
		

	int r = 0;

	while(r < home.n_room)
	{
		int w = 0,l = 0;
		
		
		globalValues[i++] = home.rooms[r].presence;
		globalValues[i++] = home.rooms[r].power;
		globalValues[i++] = home.rooms[r].n_windows;

		while(w < home.rooms[r].n_windows)
		{
			globalValues[i++] = 	home.rooms[r].windows[w];
			w++;
		}

		globalValues[i++] = home.rooms[r].n_lights;

		while(l < home.rooms[r].n_lights)
		{
			globalValues[i++] =	home.rooms[r].lights[l];
			l++;
		}

		r++;
	}

	globalValues[i++] = END_ARRAY;

	return 0;

}


/*
 * --------------------------- CHECKS THE DEW POINT ----------------------------
 * 
 * Controlla il livello della temperatura di riugiada, e si preoccupa di tenere
 * la temperatuera del pavimento minore di questa.
 * 
 * La percezione della temperatura cambia in base all'unidità interna; il molti-
 * plicatore varia tra:
 * 
 * - 0.9 ( se l'umidità interna è dello 0% )
 * 
 * - 1.1 ( se l'umidità interna è del 100% ).
 *
 */


void * processCheckRugTemp(void * arg) {

	while (true) {
		
		sleep(2);

		pthread_mutex_lock(&mutex_id);
		
		printf("\nPROCESS 1: CHECKS THE DEW POINT\n\n");

		if(receiveMessage(globalValues) == 0)
		{

			int tempRug =
			home.internal_temp - (((100 - home.internal_humidity) / 5) * 10);

			if(home.floor_temp <= tempRug && __time == 0)
			{
				if(home.hvac == on)
				{
					if(home.internal_humidity > 30)
					{
						if(checkPowerNeeded() <  MAX_WATT)
						{
							double decr = home.hvac_power * dehum_Effect;
	
							home.internal_humidity -= decr;
						}
					}
					else
					{
						if(checkPowerNeeded() <  MAX_WATT)
						{
							double decr = home.hvac_power * temp_Effect;
	
							home.internal_temp -= decr;
						}
					}
				}
				else if(_time == 0)
				{
					home.hvac = on;
					home.hvac_power = 1000;
				}
			}

			if (initializeGlobalValues() != 0)
				perror("\nNot initialized, error code \n");

			sendMessage(globalValues);

		}

		printDetailedHome();

		printf("\n------------------------------------------------\n");

		if(_time > 0)
			_time--;
		if(__time > 0)
		__time--;
		
		pthread_mutex_unlock(&mutex_id);
		
		sleep(2);
		
	}

	pthread_exit(0);
}


/*
 * -------------------------- CHECKS THE ILLUMINATION --------------------------
 * 
 * Il livello di illuminazione è indicato attraverso il campo lux relativo alla
 * singola stanza, ed è calcolato in questo modo:
 * 
 * - 0 <= lux <= 2000 
 * 
 * - limite inferiore ( tutte le luci spente, tutte le finestre chiuse) = 0
 * 
 * - limite superiore ( tutte le luci accese, tutte le finestre aperte) = 2000
 * 
 * 
 * Incremento/decrenmento di lux dato dall'apertura/chiusura di una finestra:
 * 
 * - 2000 / ( n° finestre + n° luci )
 * 
 * Incremento/decrenmento di lux dato dall'accensione/spegnimento di una luce:
 * 
 * - 2000 / ( n° finestre + n° luci ) * (1 - ( potenza singola luce / 100.0 )) 
 * 
 * 
 * Controlla che nella casa ci sia sufficiente illuminazione nelle stanze in cui
 * sono presenti delle persone; se il tempo è bello apre le finestre, mentre se 
 * è brutto apre le luci ( è possibile, in questo caso, che non venga raggiunto
 * il minimo livello di illuminazione, ma questo caso è da imputare ad una scor-
 * retta progettazione dell' impianto di illuminzazione, e quindi non gestibile 
 * in questa sede).
 *
 */


void * processCheckLight(void * arg)
{

	while (true) {
		
		sleep(2);

		pthread_mutex_lock(&mutex_id);

		printf("\nPROCESS 2: CHECKS THE ILLUMINATION\n\n");

		if(receiveMessage(globalValues) == 0)
		{

			int i,noOne = 0;
			int * lux = calloc(home.n_room, sizeof(int));

			if(home.weather == winter)
				home.door = closed;

			for(i = 0; i < home.n_room; i++)
			{
				int j;
				float percent;
				float unitIncr=(home.rooms[i].n_windows+home.rooms[i].n_lights)?
				2000/(home.rooms[i].n_windows+home.rooms[i].n_lights): 0;
				
				for(j = 0; j < home.rooms[i].n_lights;j++)
				{
					if(home.rooms[i].lights[j])
					{
						percent = home.rooms[i].lights[j] / 100.0;
						lux[i] += (unitIncr * percent);
					}
				}

				for(j = 0; j < home.rooms[i].n_windows;j++)
				{
					if(home.rooms[i].windows[j] == opened)
						lux[i] += unitIncr;
				}
				
				if(home.rooms[i].presence == yes)
				{
					if (lux[i] <= 700 && __time == 0)
					{
						if(home.weather == summer)
						{
							for(j=0; lux[i]<=700 && j<home.rooms[i].n_windows;j++)
							{
								if(home.rooms[i].windows[j] == 0)
								{
									home.rooms[i].windows[j] = 1;
									lux[i] += unitIncr;
									_time = 10;
									home.hvac = off;
									home.hvac_power = 0;
								}
							}
						}
						else
						{
							for(j=0; lux[i]<=700 && j<home.rooms[i].n_lights;j++)
							{
								if(home.rooms[i].lights[j] == 0)
								{
									if(checkPowerNeeded() <= (MAX_WATT - LIGHT_WATT))
									{
										if((lux[i] + unitIncr) <= 700)
										{
											home.rooms[i].lights[j] = 100;
											lux[i] += unitIncr;
										}
										else
										{
											percent = (700 - lux[i]) / unitIncr;
											
											home.rooms[i].lights[j] = 100 * percent + 3;
											lux[i] = 701;
										}
									}
									else
									{
										reducePowerNeeded();
										
										break;
									}
								}
								else
								{
									percent =
									1 - ( home.rooms[i].lights[j]/100.0 );

									if(checkPowerNeeded() <=
									   ( MAX_WATT - LIGHT_WATT * percent))
									{
										lux[i] += (unitIncr * percent);
										home.rooms[i].lights[j] = 100;
									}
									else
									{
										reducePowerNeeded();

										break;
									}
								}
							}
						}
					}
					else if (lux[i] > 700 && lux[i] < 1600  && __time == 0)
					{
						if(checkPowerNeeded() > MAX_WATT)
							reducePowerNeeded();
					}

					else
					{
						if(home.weather == summer)
						{
							for(j=0; lux[i]>=1600 && j<home.rooms[i].n_lights;j++)
							{
								percent = home.rooms[i].lights[j] / 100.0;
								lux[i] -= (unitIncr * percent);
								home.rooms[i].lights[j] = 0;
							}
							
							if(checkPowerNeeded() > MAX_WATT)
								reducePowerNeeded();
						}
						else
						{
							for(j=0; lux[i]>=1600 && j<home.rooms[i].n_windows;j++)
							{
								if(home.rooms[i].windows[j])
								{
									home.rooms[i].windows[j] = 0;
									lux[i] -= unitIncr;
								}
							}
						}
					}
				}
				else
				{
					for(j = 0; j < home.rooms[i].n_lights;j++)
						home.rooms[i].lights[j] = 0;
						
					if(home.weather == winter)
					{
						for(j = 0; j < home.rooms[i].n_windows;j++)
							home.rooms[i].windows[j] = 0;
					}

					if(checkPowerNeeded() > MAX_WATT)
						reducePowerNeeded();
					
					noOne++;
				}
			}

			if(noOne == home.n_room)
				home.door = closed;

			if (initializeGlobalValues() != 0)
				perror("\nNot initialized, error code \n");
				
			checkPowerNeeded(); 
				
			free(lux);

			sendMessage(globalValues);
		}

		printDetailedHome();

		printf("\n------------------------------------------------\n");

		if(_time > 0)
			_time--;
			
		if(__time > 0)
			__time--;
			
		pthread_mutex_unlock(&mutex_id);
		
		sleep(2);
		
	  }
	  pthread_exit(0);
}


/*
 * -------------------------- CHECKS THE TEMPERATURE ---------------------------
 * 
 * Controlla la temperatura interna della casa e la temperaura del pavimento, 
 * comparandole con quella esterna.
 * 
 * Se la temperatura interna è troppo bassa, la incrementa attraverso l'hvac,
 * influenzando di ritorno anche la temperatura del pavimento: nel caso contario
 * si comporta, com'è intuibile, in maniera speculare.
 * 
 * L'effetto dell'hvac cambia in base alla scelta del "weather" eseguita dall'u-
 * tente: se impostato du "summer" l'hvac ha un effetto refrigerante, altrimen-
 * ti (impostazione:"winter") ha un effetto riscaldante. E' ovvio, dato che la 
 * semi-automaticità è il primo obiettivo di un impianto domotico, che nel caso
 * l'utente abbia operato un ascelta "errata" ( temperatura esterna più rigida 
 * di quella interna e weather impostato a summer, e viceversa) il sistema in-
 * vertirà in maniera del tutto autonoma il weather stesso.
 * 
 */


void * processCheckTemp(void * arg) {

	while (true) {
		
		sleep(2);

		pthread_mutex_lock(&mutex_id);

		printf("\nPROCESS 3: CHECKS THE TEMPERATURE\n\n");

		if(receiveMessage(globalValues) == 0)
		{
				
			optimal_temp = (difference(home.desired_temp,home.min_temp))/2;
			
			// comunque la temperatura esterna influenza quella interna
			int change = (home.external_temp > home.internal_temp) ? 
						 (difference(home.internal_temp,home.external_temp)) * c_h_mult 
						 : (-(difference(home.internal_temp,home.external_temp))) * c_h_mult;//inverted
			syslog(LOG_INFO,"change: %d\n",change);
			
			home.internal_temp += change;
			home.floor_temp +=  change/2;
			
			//la temperatura interna è inferiore a quella minima
			if(home.internal_temp <= home.desired_temp && __time == 0)
			{
				if(home.internal_temp < home.min_temp)
				{
					if(home.weather == winter)
					{
						if(home.hvac == on)
						{
							int int_temp_bk = home.internal_temp;
							
							int incr = home.hvac_power * temp_Effect;
							syslog(LOG_INFO,"incr: %d\n",incr);
		
							home.internal_temp += incr;
							if(home.floor_temp <= home.min_temp)
								home.floor_temp +=  incr/2;
							else
								home.floor_temp +=  incr/3;//
										
							if (home.internal_temp == int_temp_bk)
								_bool = true;
							else
								_bool = false;
							
							if(checkPowerNeeded() < ( MAX_WATT + WATT_INCR))
								home.hvac_power += WATT_INCR;
							else
								reducePowerNeeded();
						}
						else
						{
							home.hvac = on;
							home.hvac_power = 1000;
						}
					}
					else
					{
						wrong_weather++;
						
						if(wrong_weather > 5)
						{
							switch_weather = true;
							wrong_weather = 0;
						}
					}
				}
				//caso in cui sto tra la min e la desired temp
				else if((difference(home.internal_temp,(home.min_temp + optimal_temp)) < optimal_temp)
						 && _time == 0)
				{
					if(home.hvac == off)
					{
						home.hvac = on;
						home.hvac_power = 1000;
					}
					else
					{
						if(home.internal_temp < (home.min_temp + optimal_temp))
						{
							int incr = home.hvac_power * temp_Effect;
							
							home.internal_temp += incr;
							if(home.floor_temp <= (home.min_temp + optimal_temp))
								home.floor_temp +=  incr/2;
							else
								home.floor_temp +=  incr/3;
		
							if(checkPowerNeeded() < ( MAX_WATT + WATT_INCR))
								home.hvac_power += WATT_INCR;
							else
								reducePowerNeeded();
						}
						else
						{
							int decr = home.hvac_power * temp_Effect;
							
							home.internal_temp -= decr;
							if(home.floor_temp >= (home.min_temp - optimal_temp))
								home.floor_temp -=  decr/2;
							else
								home.floor_temp -=  decr/3;
		
							if(checkPowerNeeded() < ( MAX_WATT + WATT_INCR))
								home.hvac_power += WATT_INCR;
							else
								reducePowerNeeded();
						}
					}
				}
				else
				{
					home.hvac = off;
					home.hvac_power = 0;
				}
			}
			//caso in cui è troppo caldo
			else if (__time == 0)
			{
				if(home.weather == summer)
				{
					if(home.hvac == on)
					{
						int int_temp_bk = home.internal_temp;
						
						int decr = home.hvac_power * temp_Effect;
						
						home.internal_temp -= decr;
						if(home.floor_temp >= (home.min_temp - optimal_temp))
							home.floor_temp -=  decr/2;
						else
							home.floor_temp -=  decr/3;
						
						if (home.internal_temp == int_temp_bk)
							_bool = true;
						else
							_bool = false;

						if(checkPowerNeeded() < ( MAX_WATT + WATT_INCR))
							home.hvac_power += WATT_INCR;
						else
							reducePowerNeeded();
					}
					else if(_time == 0)
					{
						home.hvac = on;
						home.hvac_power = 1000;
					}
				}
				else
				{
					wrong_weather++;
					
					if(wrong_weather > 5)
					{
						switch_weather = true;
						wrong_weather = 0;
					}
				}
			}
			if(__time > 0)
			{
				if(_bool == true)
					_warning = true;
			}
			else
				_warning = false;
				
			if(switch_weather == true)
			{
				if(home.weather == winter)
					home.weather = summer;
				else
					home.weather = winter;
					
				switch_weather = false;
			}

			if (initializeGlobalValues() != 0)
				perror("\nNot initialized, error code \n");

			sendMessage(globalValues);
			
		}

	    printDetailedHome();
	   

	    printf("\n------------------------------------------------\n");

	    if(_time > 0)
			_time--;
			
		if(__time > 0)
			__time--;
			
		
		pthread_mutex_unlock(&mutex_id);
		
		sleep(2);
		//fflush(stdout);

	}
	pthread_exit(0);
}


/*
 * ------------------------------ CLOSES SOCKET --------------------------------
 * 
 * Chiude il socket attivo.
 *
 */


void closeSocket()
{
	close(id_client);
}


/*
 * -------------------- CREATES SOCKET BY ADDRESS --------------------
 * 
 * PARAMETRI:
 * 
 * 		Destinazione	:char*	URL http
 * 		Porta			:int	porta
 * 
 * Apre un socket (ossia, in sostanza, un canale di comunicazione) identificato 
 * da una Porta e un indirizzo HTTP, in cui i dati possono transitare in entram-
 * bi i sensi, ma in modo alternato ( a senso unico alternato ).
 *
 */


int createSocketByAddress(char* Destinazione, int Porta)
{
	struct sockaddr_in temp;
	struct hostent *h;

	//Tipo di indirizzo
	temp.sin_family=AF_INET;
	temp.sin_port=htons(Porta);
	h=gethostbyname(Destinazione);

	//Creazione socket.
	id_client = socket(AF_INET,SOCK_STREAM,0);

	//Connessione del socket.
	if((id_server = connect(id_client, (struct sockaddr*) &temp,
							sizeof(temp))) < 0)
		exit(1);

	return 0;
}


/*
 * ------------------------------ RECONNECTION ---------------------------------
 * 
 * PARAMETRI:
 * 
 * 		success	:int	valore passato alla funzione da @sendMessage()
 * 				e @receiveMessage() per la riconnessione
 * 
 * 
 * Prova a riconnetersi al server per un massimo di MAX_TRY volte.
 *
 */


int tryToReconnect(int success)
{

	printf("\n\nConnection is fallen, trying to reconnect:   ");

	while(success < 0 && _try < MAX_TRY)
	{
		printf("%c",8);
		
		success = createSocketByIP(_IP,PORT);
		_try++;
		
		sleep(1);
		
	}

	if(_try == MAX_TRY)
	{
		perror("\n\nConnection timeout, error");
		exit(1);
	}

	return 0;
}


/*
 * --------------------------- CREATES SOCKET BY IP ----------------------------
 * 
 * PARAMETRI:
 * 
 * 		IP	:char*	ip
 * 		Porta	:int	porta
 *  
 * 
 * Apre un socket (ossia, in sostanza, un canale di comunicazione) identificato 
 * da una Porta e un indirizzo IP, in cui i dati possono transitare in entrambi 
 * i sensi, ma in modo alternato (a senso unico alternato).
 *
 */


int createSocketByIP(char* IP, int Porta)
{
	struct sockaddr_in client_address;
	struct in_addr server_addr;

	inet_aton(IP, &server_addr);

	//Tipo di indirizzo
	client_address.sin_family=AF_INET;
	client_address.sin_addr = server_addr;
	client_address.sin_port=Porta;

	//Creazione socket.
	id_client = socket(AF_INET,SOCK_STREAM,0);

	//Connessione del socket.
	if((id_server = connect(id_client, (struct sockaddr*) &client_address,
						  sizeof(client_address))) < 0)
	{
		if((_try % 8) == 0 || (_try % 8) == 4)
			printf("|");
		else if((_try % 8) == 1 || (_try % 8) == 5)
			printf("/");
		else if((_try % 8) == 2 || (_try % 8) == 6)
			printf("-");
		else if((_try % 8) == 3 || (_try % 8) == 7)
			printf("\\");
					
		fflush(stdout);
	}
	if(!client_autenthicated)
	{
		if(send_autentication(id_client))
			return -1;
	}
	return id_server;
}


/*
 * ----------------------------------- SEND ------------------------------------
 * 
 * PARAMETRI:
 * 
 * 		values	:int*	array che conterrà i valori della Home
 * 
 * 
 * Scrive dati sul socket: tali dati provengono dalla lettura dell'aray globale 
 * contenente i valori correnti delle Struct standard_room e standard_home.
 *
 */


void sendMessage(int * values)
{

	int i,exact_send = 0,success = -1;
	_try = 0;
	
	int * exact_value = calloc(2, sizeof(int));
	
	for(i = 0; values[i] != END_ARRAY; i++, exact_send++)
	  ;

	exact_send++;
	
	exact_value[0] = (_warning)?-1:_warning;
	exact_value[1] = exact_send;
	
	if(exact_value[0] == false)
	{
		if ((write(id_client,&exact_value[0],sizeof(int))) < 0)
			tryToReconnect(success);
		else if ((write(id_client,&exact_value[1],sizeof(int))) < 0)
			tryToReconnect(success);
		else if ((write(id_client,values,(exact_value[1] * sizeof(int)))) < 0)
			tryToReconnect(success);
	}
	else
	{
		if ((write(id_client,&exact_value[0],sizeof(int))) < 0)
			tryToReconnect(success);
	}

}


/*
 * ---------------------------------- RECEIVE ----------------------------------
 * 
 * PARAMETRI:
 * 
 * 		values	:int*	array contenente i valori della Home
 * 
 * 
 * Legge dati dal Socket: tali dati verranno poi utilizzati per aggiornare l'ar-
 * ray globale con gli eventuali nuovi valori immessi manualmente tramite il 
 * Terminale.
 *
 */

int receiveMessage(int * values)
{

	int nDati,success = - 1;
	_try = 0;
	int * exact_value = calloc(2, sizeof(int));

	if((nDati = read(id_client,exact_value,2 * sizeof(int))) < 0)
		tryToReconnect(success);
        
    else if(exact_value[0] == 0)
    {
		if ((nDati = read(id_client,values,(exact_value[1] * sizeof(int)))) < 0)
			tryToReconnect(success);
		else
		{
			setValues(values);

			if (initializeGlobalValues() != 0)
			{
				perror("Not Inizialized, error code ");
				exit(1);
			}
		}
	}
	else if(exact_value[0] > 0)
		getRoomName((exact_value[0] - 1),exact_value[1]);
	
	
	return exact_value[0];
	
 }


/*
 * --------------------------------- DIFFERENCE --------------------------------
 * 
 * Differenza positiva tra due numeri.
 * 
 */


int difference(int a,int b)
{
	
	if( a >= 0 && b >= 0)
		return (a > b) ? (a - b) : (b - a);
	else if(a < 0 && b < 0)
		return (abs(a) > abs(b)) ? (abs(a) - abs(b)) : (abs(b) - abs(a));
	else
		return (a >= 0 && b < 0) ? (a + abs(b)) : (abs(a) + b);
	
	return -1;
}
