/* 	engine server.c
 *
 *	Copyright 2010 Simone Serafini 1.5
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *	MA 02110-1301, USA.
*/

#include "engine_server.h"


extern int errno;
int command, sub_command, number_command, room_command,
read_write = 1, connection = 0, iface_state = 0, gui_connection = 0, 
server_id, client_id, gui_id, if_warning = 0;
FILE * history_file;
struct ifconf iflist;
struct ifreq  ifreq_flags;
char cret[DIM_NAME], buffer[4096];
pthread_t iserver, iterminal, iclient, itry;
sem_t sem_server, sem_client;
struct_value value;

/*	Gui global variables */

int fd = -1, margin = 5, ili = li, iwi = wi, iiface = iface,
	config_file_finish = -1, config_file_done = -1,
	config_file_load = -1, config_file_home = -1, config_file_room = -1,
	n_li = 0, n_wi = 0, if_room = 0, general, entro = 0, n_iface = 0, num_select = -1, scelta = -1;
struct_value * cc;
char *radio_names[4][2], *filename;
GtkWidget *name_room, *presence[2], **widget_li, **widget_wi, *widget_iface,
	*table, *status_home, *status_room, *status_iface, *status_general;

/* FUNCTION:	get_pwd
 * PARAMETERS:	puntatore a char
 *RITORNO:	puntatore a char
 *DESCRIZIONE:	Restituisce la stringa con la cartella locale.
*/

char * get_pwd(char * path)
{
	strcat(strcpy(path, getenv("PWD")), "/");
	return path;
}

/* FUNCTION:	print_help
 * PARAMETERS:	void
 *RITORNO:	void
 *DESCRIZIONE:	Scrive su stdout l'help del server.
*/
void print_help(char * path_program)
{
	printf("Engine Server version %s\n", VERSION);
	printf("\nThis program purpose is to manage an house whit a domotic\n");
	printf("system.\n");
	printf("Thank you for using this program, here is the help:\n");
	printf("\nUsage:\n");
	printf("%s [OPTION...] \n", path_program);
	printf("\nApplication options:\n");
	printf("help:	prints this help\n");
	printf("man:	print a detailed manual that explain this program work\n");
	printf("usman:	same as 'man', but writes the manual in a txt FILE\n");
	printf("\tyou can find into your current directory.\n\n");
	printf("Report bugs to: astro.simone@gmail.com\n");
}

/* FUNCTION:	print_user_manual
 * PARAMETERS:	puntatore a char, puntatore a char
 *RITORNO:	int
 *DESCRIZIONE:	Scrive su destination o stdout i commenti di source.
*/
int print_user_manual(char * source, char * destination)
{
	FILE * fSource, * fDestination;
	char c;
	int bool;
	fSource = fopen(source, "r");
	if(destination != NULL){
		fDestination = fopen(destination, "w");
		if(fDestination == NULL){
			perror("open destination");
			exit(errno);
		}
	}
	else
		fDestination = stdout;
	if(fSource == NULL){
		perror("open source");
		exit(errno);
	}
	else{
		bool = 0;
		while(!feof(fSource)){
			if(fscanf(fSource,"%c", &c) != EOF && c == '/'){
				if(fscanf(fSource,"%c", &c) != EOF && c == '*'){
					while(fscanf(fSource,"%c", &c) != EOF && !bool){
						if(c == '*'){
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
		printf("\n%s salvato con successo\n", destination);
	}
	return 0;
}

/* FUNCTION:	save_struct
 * PARAMETERS:	void
 *RITORNO:	int
 *DESCRIZIONE:	Salva su file il file di configurazione della casa.
*/
int save_struct(void)
{
	FILE * f;
	char path_pwd[DIM_STR];
	register int incrR = 0, incrW = 0, incrL = 0;
	char * win = ",window";
	char * lig = ",light";
	if((f = fopen(strcat(strcpy(path_pwd, filename), ".bk"), "w")) == NULL)
		printf("\nErrore nell'apertura del file\n");
	else{
		fprintf(f, "%s %d\n%s %d\n%s %d\n%s %d\n%s %d\n%s %d\n%s %2.2f\n%s %2.2f\n%s %2.2f\n%s %2.2f\n%s %2.2f%s",
				  DOOR, home.door ,
				  HVAC, home.hvac ,
				  HVAC_POWER, home.hvac_power ,
				  WEATHER, home.weather ,
				  INT_HUMIDITY, home.internal_humidity ,
				  EXT_HUMIDITY, home.external_humidity ,
				  INT_TEMP, ((double)home.internal_temp)/10 ,
				  EXT_TEMP, ((double)home.external_temp)/10 ,
				  FLOOR_TEMP, ((double)home.floor_temp)/10 ,
				  MIN_TEMP, ((double)home.min_temp)/10 ,
				  DES_TEMP, ((double)home.des_temp)/10 ,";");
		while(incrR < home.n_room){
			fprintf(f,"\n%s{presence %d",
					  home.rooms[incrR].name, home.rooms[incrR].presence);
			incrW = 0;
			incrL = 0;
			while (incrW < home.rooms[incrR].n_windows){
				fprintf(f, "%s %d",
						  win, home.rooms[incrR].windows[incrW]);
				incrW++;
			}
			while (incrL < home.rooms[incrR].n_lights){
				fprintf(f, "%s %d",
						  lig, home.rooms[incrR].lights[incrL]);
				incrL++;
			}
			incrR++;
			fprintf(f, "%s", ",}");
		}
		fprintf(f, "%s\n", ".");
		fclose(f);
	}
	return 0;
}

/* FUNCTION:	process_exit
 * PARAMETERS:	int
 *RITORNO:	void
 *DESCRIZIONE:	Effettua le operazioni necessarie prima di terminare il processo.
*/
void process_exit(int status)
{
	sem_destroy(&sem_server);
	sem_destroy(&sem_client);
	if(!config_file_finish)fclose(history_file);
	if(!config_file_finish)save_struct();
	syslog(LOG_DEBUG, "The end\n");
	printf("\n");
	exit(status);
}

/* FUNCTION:	print_thread_name
 * PARAMETERS:	pthread_t
 *RITORNO:	puntatore a char
 *DESCRIZIONE:	Stampa i nomi dei thread in base al propio id.
*/
char * print_thread_name(pthread_t th)
{
	if (th == iserver)
		return "Server";
	else if (th == iterminal)
		return "Terminal";
	else if (th == iclient)
		return "Client";
	else if (th == itry)
		return "Try";
	return "Error";
}

/* FUNCTION:	print_semaphore
 * PARAMETERS:	void
 *RITORNO:	void
 *DESCRIZIONE:	Stampa i valori correnti dei semafori utilizzati.
*/
void print_semaphore(void)
{
	int val_server, val_client;
	sem_getvalue(&sem_server, &val_server);
	printf("\t%s: Server %d\n", print_thread_name(pthread_self()), val_server);
	sem_getvalue(&sem_client, &val_client);
	printf("\t%s: Client %d\n", print_thread_name(pthread_self()), val_client);
}

/* FUNCTION:	usage
 * PARAMETERS:	void
 *RITORNO:	void
 *DESCRIZIONE:	Spiega come formare un patten corretto per il terminale.
*/
void usage(void)
{
	printf("------------USAGE--------------\n");
	printf("set|get WHAT [VALUE]\n");
	printf("Possible value of the WHAT:\n");
	printf(" - d or door\n");
	printf(" - h or hvac\n");
	printf(" - hpw or hvac-power\n");
	printf(" - we or weather\n");
	printf(" - ih or internal-humidity\n");
	printf(" - eh or external-humidity\n");
	printf(" - it or internal-temp\n");
	printf(" - et or external-temp\n");
	printf(" - ft or floor-temp\n");
	printf(" - mt or min-temp\n");
	printf(" - dt or des-temp\n");
	printf(" - name of room.OBJECT in:\n");
	printf("   - pr or presence\n");
	printf("   - pw or power\n");
	printf("   - wi or windows.NUMBER\n");
	printf("   - li or lights.NUMBER\n");
	printf("printhd per print home's data\n");
	printf("start per start comunication\n");
	printf("stop per stop comunication\n");
	printf("exit per uscire\n\n");
}

/* FUNCTION:	check_name_room
 * PARAMETERS:	string
 *RITORNO:	int
 *DESCRIZIONE:	Controlla se nella struttura ci sia una stanza con il
 *             nome name_room ritornando il numero della stanza.
*/
int check_name_room(char * name_room)
{
	register int i;
	for(i = 0;i < home.n_room; i++)
		if(strcmp(home.rooms[i].name, name_room) == 0)
			return home.rooms[i].number;
	return -1;
}

/* FUNCTION:	chaeck_done_iface
 * PARAMETERS:	puntatore a GtkWidget
 *RITORNO:	int
 *DESCRIZIONE:	Controlla se l'interfaccia selezionate si UP.
*/
int check_done_iface(GtkWidget * parent)
{
	if(scelta >= 0 && scelta < n_iface){
		gtk_main_quit();
		gtk_widget_destroy(parent);
		return 0;
	}
	gtk_widget_grab_focus(widget_iface);
	return -1;
}

/* FUNCTION:	count_argc
 * PARAMETERS:	string
 *RITORNO:	int
 *DESCRIZIONE:	Conta quanti argomenti vengono passati al terminale e li
 *             restituisce.
*/
int count_argc(char cmd[DIM_PROMT])
{
	register int i, j = -1, argc = 0, flag;
	for(i = 0;i < strlen(cmd);){
		i = j+1;
		flag = 0;
		if(cmd[i] != ' '){
			for(j = i+1;!flag && j < strlen(cmd);j++)
				if(cmd[j] == ' '){
					argc++;
					flag = 1;
				}
		}
		else
			j++;
	}
	if(cmd[strlen(cmd)-1] != ' ')
		argc++;
	return argc;
}

/* FUNCTION:	check_number_windows
 * PARAMETERS:	int, int
 *RITORNO:	int
 *DESCRIZIONE:	Controlla se la stanza n_room ha la finestra number_windows.
*/
int check_number_windows(int number_windows, int n_room)
{
	if(number_windows < home.rooms[n_room].n_windows)
		return number_windows;
	else
		return -1;
}

/* FUNCTION:	check_number_lights
 * PARAMETERS:	int, int
 *RITORNO:	int
 *DESCRIZIONE:	Controlla se la stanza n_room ha la luce number_lights .
*/
int check_number_lights(int number_lights, int n_room)
{
	if(number_lights < home.rooms[n_room].n_lights)
		return number_lights;
	else
		return -1;
}

/* FUNCTION:	precheck_number
 * PARAMETERS:	puntatore a int, puntatore a char, puntatore a char
 *RITORNO:	int
 *DESCRIZIONE:	parsa una parte della stringa: il numero di finestra o di luce.
*/
int precheck_number(int * i, char cmd[DIM_PROMT], char cmd_quattro[DIM_PROMT])
{
	register int j;
	for (j = 0;j < strlen(cmd_quattro); j++) {
		cmd_quattro[j] = ' ';
	}
	j = 0;
	do{
		if((cmd[*i] != ' ') && (cmd[*i] != '\n') && (cmd[*i] != '\0')){
			if(isdigit(cmd[*i])){
				cmd_quattro[j] = cmd[*i];
				j++;
			}
		}
		(*i)++;
	}while ((((*i)-1) >= 0) && ((*i) < strlen(cmd)) && (cmd[(*i)-1] != ' ')
		&& (cmd[(*i)-1] != '\n') && (cmd[(*i)-1] != '\0'));

	cmd_quattro[j] = '\0';
	return atoi(cmd_quattro);
}

/* FUNCTION:	parse_object_room
 * PARAMETERS:	puntatore a int, puntatore a char, puntatore a char, int
 *RITORNO:	int
 *DESCRIZIONE:	parsa una parte della stringa: oggetto della richiesta,
 *             fissata la stanza.
*/
int parse_object_room(int * i, char cmd[DIM_PROMT], char cmd_tre[DIM_PROMT], int n_room)
{
	register int j, status;
	char cmd_quattro[DIM_PROMT];
	for (j = 0;j < strlen(cmd_tre); j++)
		cmd_tre[j] = ' ';
	j = 0;
	do{
		if((cmd[*i] != '.') && (cmd[*i] != ' ') && (cmd[*i] != '\n') && (cmd[*i] != '\0')){
			if (isalpha(cmd[*i]) && isupper(cmd[*i])){
				cmd[*i] += 32;
			}
			cmd_tre[j] = cmd[*i];
			j++;
		}
		(*i)++;
	}while ((((*i)-1) >= 0) && ((*i) < strlen(cmd)) && (cmd[(*i)-1] != '.')
		&& (cmd[(*i)-1] != ' ') && (cmd[(*i)-1] != '\n') && (cmd[(*i)-1] != '\0'));

	cmd_tre[j] = '\0';

	if((strcmp(cmd_tre, PRESENCE) && (strcmp(cmd_tre, "pr"))) == 0){
		syslog(LOG_DEBUG, "presence");
		sub_command = 1;
	}
	else if((strcmp(cmd_tre, POWER) && (strcmp(cmd_tre, "pw"))) == 0){
			syslog(LOG_DEBUG, "power");
			sub_command = 2;
	}
	else if((strcmp(cmd_tre, WINDOW) && (strcmp(cmd_tre, "wi"))) == 0){
			syslog(LOG_DEBUG, "window");
			sub_command = 3;
			if (cmd[(*i)-1] == '.'){
				syslog(LOG_DEBUG, "%c", cmd[(*i)-1]);
				status = precheck_number(i, cmd, cmd_quattro);
				number_command = check_number_windows(status, n_room);
				if (number_command < 0)
					return -1;
				else
					syslog(LOG_DEBUG, "%d",number_command);
			}
			else{
				printf("\n");
				usage();
				return -1;
			}
	}
	else if((strcmp(cmd_tre, LIGHT) && (strcmp(cmd_tre, "li"))) == 0){
		syslog(LOG_DEBUG, "light");
		sub_command = 4;
		if (cmd[(*i)-1] == '.'){
			syslog(LOG_DEBUG, "%c", cmd[(*i)-1]);
			status = precheck_number(i, cmd, cmd_quattro);
			number_command = check_number_lights(status, n_room);
			if (number_command < 0)
				return -1;
			else
				syslog(LOG_DEBUG, "%d",number_command);
		}
		else{
			printf("\n");
			usage();
			return -1;
		}
	}
	else if((strcmp(cmd_tre, "name")) == 0){
		syslog(LOG_DEBUG, "name");
		sub_command = 5;
	}
	else{
		printf("\n");
		usage();
		return -1;
	}
	return 0;
}

/* FUNCTION:	parse_what
 * PARAMETERS:	puntatore a int, puntatore a char, puntatore a char
 *RITORNO:	int
 *DESCRIZIONE:	parsa una parte della stringa: dati generali della casa o
 *             nome stanza.
*/
int parse_what(int * i, char cmd[DIM_PROMT], char cmd_due[DIM_PROMT])
{
	register int j, n_room, status;
	char cmd_tre[DIM_PROMT];
	for (j = 0;j < strlen(cmd_due); j++)
		cmd_due[j] = ' ';
	j = 0;
	do{
		if((cmd[*i] != '.') && (cmd[*i] != ' ') && (cmd[*i] != '\n') && (cmd[*i] != '\0')){
			if (isalpha(cmd[*i]))
				cmd[*i] = tolower(cmd[*i]);
			cmd_due[j] = cmd[*i];
			j++;
		}
		(*i)++;
	}while ((((*i)-1) >= 0) && ((*i) < strlen(cmd)) && (cmd[(*i)-1] != '.')
		&& (cmd[(*i)-1] != ' ') && (cmd[(*i)-1] != '\n') && (cmd[(*i)-1] != '\0'));

	cmd_due[j] = '\0';

	if (((strcmp(cmd_due, DOOR)) && (strcmp(cmd_due, "d"))) == 0){
		syslog(LOG_DEBUG, DOOR);
		command = 1;
	}
	else if (((strcmp(cmd_due, HVAC)) && (strcmp(cmd_due, "h"))) == 0){
		syslog(LOG_DEBUG, HVAC);
		command = 2;
	}
	else if (((strcmp(cmd_due, HVAC_POWER)) && (strcmp(cmd_due, "hpw"))) == 0){
		syslog(LOG_DEBUG, HVAC_POWER);
		command = 3;
	}
	else if ((strcmp(cmd_due, WEATHER) && (strcmp(cmd_due, "we"))) == 0){
		syslog(LOG_DEBUG, WEATHER);
		command = 4;
	}
	else if ((strcmp(cmd_due, INT_HUMIDITY) && (strcmp(cmd_due, "ih"))) == 0){
		syslog(LOG_DEBUG, INT_HUMIDITY);
		command = 5;
	}
	else if ((strcmp(cmd_due, EXT_HUMIDITY) && (strcmp(cmd_due, "eh"))) == 0){
		syslog(LOG_DEBUG, EXT_HUMIDITY);
		command = 6;
	}
	else if ((strcmp(cmd_due, INT_TEMP) && (strcmp(cmd_due, "it"))) == 0){
		syslog(LOG_DEBUG, INT_TEMP);
		command = 7;
	}
	else if ((strcmp(cmd_due, EXT_TEMP) && (strcmp(cmd_due, "et"))) == 0){
		syslog(LOG_DEBUG, EXT_TEMP);
		command = 8;
	}
	else if ((strcmp(cmd_due, FLOOR_TEMP) && (strcmp(cmd_due, "ft"))) == 0){
		syslog(LOG_DEBUG, FLOOR_TEMP);
		command = 9;
	}
	else if ((strcmp(cmd_due, MIN_TEMP) && (strcmp(cmd_due, "mt"))) == 0){
		syslog(LOG_DEBUG, MIN_TEMP);
		command = 11;
	}
	else if ((strcmp(cmd_due, DES_TEMP) && (strcmp(cmd_due, "dt"))) == 0){
		syslog(LOG_DEBUG, DES_TEMP);
		command = 12;
	}
	else{
		n_room = check_name_room(cmd_due);
		if (n_room >= 0){
			command = 'r';
			room_command = n_room;
			syslog(LOG_DEBUG, "%s", cmd_due);
			if (cmd[(*i)-1] == '.'){
				syslog(LOG_DEBUG, "%c", cmd[(*i)-1]);
				status = parse_object_room(i, cmd, cmd_tre, n_room);
				if (status < 0)
					return status;
			}
			else
				return -1;
		}
		else
			return -1;
	}
	return 0;
}

/* FUNCTION:	parse_value
 * PARAMETERS:	puntatore a int, puntatore a char, puntatore a char
 *RITORNO:	double
 *DESCRIZIONE:	parsa una parte della stringa: per andare a modificare
 *             i valori della casa.
*/
double parse_value(int  * i, char cmd[DIM_PROMT], char cmd_cinque[DIM_PROMT])
{
	register int j, punto = -1;
	register double val;
	for (j = 0;j < strlen(cmd_cinque); j++)
		cmd_cinque[j] = ' ';
	j = 0;
	do{
		if ((cmd[*i] != ' ') && (cmd[*i] != '.') && (cmd[*i] != '\n') && (cmd[*i] != '\0')){
			cmd_cinque[j] = cmd[*i];
			j++;
			if(punto >= 0)
				punto++;
		}
		if(cmd[*i] == '.')
			punto = 0;
		(*i)++;
	}while ((((*i)-1) >= 0) && ((*i) < strlen(cmd)) && (cmd[(*i)-1] != ' ')
		&& (cmd[(*i)-1] != '\n') && (cmd[(*i)-1] != '\0'));

	cmd_cinque[j] = '\0';
	if (((strcmp(cmd_cinque, "\0")) && (strcmp(cmd_cinque, " "))
	&& (strcmp(cmd_cinque, ""))) == 0)
		return -1;
	if (((strcmp(cmd_cinque, WINTER)) && (strcmp(cmd_cinque, SUMMER))
	&& (strcmp(cmd_cinque, OFF)) && (strcmp(cmd_cinque, ON))
	&& (strcmp(cmd_cinque, CLOSE)) && (strcmp(cmd_cinque, OPEN))
	&& (strcmp(cmd_cinque, NOBODY)) && (strcmp(cmd_cinque, SOMEBODY))) == 0){
		if (((strcmp(cmd_cinque, WINTER)) && (strcmp(cmd_cinque, OFF))
		&& (strcmp(cmd_cinque, CLOSE)) && (strcmp(cmd_cinque, NOBODY))) == 0)
			val = 0.0;
		if (((strcmp(cmd_cinque, SUMMER)) && (strcmp(cmd_cinque, ON))
		&& (strcmp(cmd_cinque, OPEN)) && (strcmp(cmd_cinque, SOMEBODY))) == 0)
			val = 1.0;
	}
	else{
		val = atoi(cmd_cinque);
		if(punto >= 0)
			for(j = 0; j < punto; j++)
				val /= 10;
	}
	return val;
}

/* FUNCTION:	parse_type
 * PARAMETERS:	puntatore a int, puntatore a char, puntatore a char, puntatore a char
 *RITORNO:	int
 *DESCRIZIONE:	parsa una parte di stringa: comandi base e get/set.
*/
int parse_type(int * i, char cmd[DIM_PROMT], char cmd_uno[DIM_PROMT], char cmd_due[DIM_PROMT])
{
	char cmd_cinque[DIM_PROMT];
	register int j;
	double status;
	for(j = 0;j < strlen(cmd_uno); j++){
		cmd_uno[j] = ' ';
		cmd_due[j] = ' ';
	}
	j = 0;
	do{
		if ((cmd[*i] != ' ') && (cmd[*i] != '\n') && (cmd[*i] != '\0')){
			if (isalpha(cmd[*i]))
				cmd[*i] = tolower(cmd[*i]);
			cmd_uno[j] = cmd[*i];
			j++;
		}
		(*i)++;
	}while ((((*i)-1) >= 0) && ((*i) < strlen(cmd)) && (cmd[(*i)-1] != ' ')
		&& (cmd[(*i)-1] != '\n') && (cmd[(*i)-1] != '\0'));

	cmd_uno[j] = '\0';
	if (strcmp(cmd_uno, "exit") == 0){
		syslog(LOG_INFO, "exit\n");
		command = 'e';
	}
	else if (strcmp(cmd_uno, "printhd") == 0){
		syslog(LOG_INFO, "printhd\n");
		command = 'p';
	}
	else if (strcmp(cmd_uno, "start") == 0){
		syslog(LOG_INFO, "start\n");
		command = 's';
	}
	else if (strcmp(cmd_uno, "stop") == 0){
		syslog(LOG_INFO, "stop\n");
		command = 't';
	}
	else if (strcmp(cmd_uno, "get") == 0){
		syslog(LOG_INFO, "get");
		status = parse_what(i, cmd, cmd_due);
		if (status < 0)
			return status;
	}
	else if (strcmp(cmd_uno, "set") == 0){
		syslog(LOG_INFO, "set");
		status = parse_what(i, cmd, cmd_due);
		if (status < 0)
			return status;
		status = parse_value(i, cmd, cmd_cinque);
		if (status < 0){
			return status;
		}
		if (command != 'r')
			command *= 10;
		else
			command = 'R';

		sub_command *= 10;

		if (command == 70 || command == 80 || command == 90 || command == 110 || command == 120){
			value.union_value.double_value = status;
			value.tipo = DOUBLE;
			syslog(LOG_INFO, "Value = %2.2f", status);
		}
		else if (sub_command != 50){
			value.union_value.int_value = status;
			value.tipo = INT;
			syslog(LOG_INFO, "Value = %d", (int)status);
		}
		else{
			strcpy(value.union_value.string_value, cmd_cinque);
			value.tipo = STRING;
			syslog(LOG_INFO, "Value = %s\n", value.union_value.string_value);
		}
	}
	else{
		return -1;
	}
	return 0;
}

/* FUNCTION:	parse
 * PARAMETERS:	int, puntatore a char
 *RITORNO:	int
 *DESCRIZIONE:	parsa la stringa.
*/
int parse(int argc, char cmd[DIM_PROMT])
{
	int i = 0;
	char cmd_uno[DIM_PROMT];
	char cmd_due[DIM_PROMT];
	int status = parse_type(&i, cmd, cmd_uno, cmd_due);
	if (status < 0)
		return status;
	return 0;
}

/* FUNCTION:	readline
 * PARAMETERS:	puntatore a char, int
 *RITORNO:	int
 *DESCRIZIONE:	Legge un char alla volta e lo inserisce nella stringa da parsare
 *             nel terminale.
*/
int readline(char *word, int len)
{
	int ch, cur = 0, i;
	__fpurge(stdin);
	printf("%s:@>",print_thread_name(pthread_self()));
	while ((ch = getchar()) == '\n')
		printf("%s:@>",print_thread_name(pthread_self()));
	while (ch != '\n' && ch != EOF){
		if(isalpha(ch) || isdigit(ch) || ch == ' ' || ch == '.' || ch == '-' || ch == '_'){
			if (cur < len-1){
				word[cur] = ch;
				cur++;					
			}
		}
		fflush(NULL);
		ch = getchar();
	}
	word[cur] = '\0';
	for(i = 0; word[i] != '\0'; i++)
		if(fputc(word[i], history_file) == EOF)
			printf("\n\nfputc error\n");
	if(fputc('\n', history_file) == EOF)
		printf("\n\nfputc error\n");
	return 0;
}

/* FUNCTION:	get_values
 * PARAMETERS:	void
 *RITORNO:	puntatore a int
 *DESCRIZIONE:	Scambia i dati della struttura in uscita in un array.
*/
int * get_values(void)
{
	register int i = 0, r = 0;
	int * values = malloc((12+home.n_room*(2+DIM_ROOM)+1)*sizeof(int));
	values[i++] = home.door;
	values[i++] = home.hvac;
	values[i++] = home.weather;
	values[i++] = home.hvac_power;
	values[i++] = home.internal_humidity;
	values[i++] = home.external_humidity;
	values[i++] = home.internal_temp;
	values[i++] = home.external_temp;
	values[i++] = home.floor_temp;
	values[i++] = home.min_temp;
	values[i++] = home.des_temp;
	values[i++] = home.n_room;
	while(r < home.n_room)
	{
		int w = 0,l = 0;
		values[i++] = home.rooms[r].presence;
		values[i++] = home.rooms[r].power;
		values[i++] = home.rooms[r].n_windows;
		while(w < home.rooms[r].n_windows)
		{
			values[i++] = home.rooms[r].windows[w];
			w++;
		}
		values[i++] = home.rooms[r].n_lights;
		while(l < home.rooms[r].n_lights)
		{
			values[i++] = home.rooms[r].lights[l];
			l++;
		}
		r++;
	}
	values[i] = END_ARRAY;
	return values;
}

/* FUNCTION:	set_values
 * PARAMETERS:	puntatore a int
 *RITORNO:	int
 *DESCRIZIONE:	Scambia i dati della struttura in entrata da un array.
*/
int set_values(int * values)
{
	register int i = 0, r;
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
	home.des_temp			= values[i++];
	
	i++;	//	home.n_room è di sola lettura
	r = 0;
	while(r < home.n_room)
	{
		int w = 0,l = 0;
		home.rooms[r].presence = values[i++];
		home.rooms[r].power = values[i++];
		i++;	//	home.rooms[r].n_windows è di sola lettura
		while(w < home.rooms[r].n_windows)
		{
			home.rooms[r].windows[w] = values[i++];
			w++;
		}
		i++;	//	home.rooms[r].n_lights è di sola lettura
		while(l < home.rooms[r].n_lights)
		{
			home.rooms[r].lights[l] = values[i++];
			l++;
		}
		r++;
	}
	return 0;
}

/* FUNCTION:	receive_password
 * PARAMETERS:	int, puntatore a char
 *RITORNO:	int
 *DESCRIZIONE:	Riceve password di sicurezza.
*/
int receive_password(int socket, char * socket_type)
{
	int byte_tot = 0, byte_count, byte, * array_value;
	char * password_proposta, * password_proposta_copy;
	
	array_value = calloc(2, sizeof(int));
	byte_count = 0;
	while(byte_count < 2 *sizeof(int))
		if((byte = read(socket, array_value, 2 * sizeof(int) - byte_count)) > 0){
			syslog(LOG_DEBUG, "Read password len con successo\n");
			byte_count += byte;
		}
		else if(errno != EINTR){
			perror("Server password len ");
			return -1;
		}
	if(!array_value[no]){
		byte_tot = array_value[yes] * sizeof(char);
		password_proposta = calloc(byte_tot, sizeof(char));
		password_proposta_copy = password_proposta;
		byte_count = 0;
		while(byte_count < byte_tot)
			if((byte = read(socket, password_proposta, byte_tot - byte_count)) > 0){
				syslog(LOG_DEBUG, "Read password con successo\n");
				byte_count += byte;
				password_proposta += byte;
			}
			else if(errno != EINTR){
				perror("Server password");
				return -1;
			}
		password_proposta = password_proposta_copy;
		syslog(LOG_INFO, "passord %s\n", password_proposta);
		if(strcmp(password_proposta, "Client") == 0 &&
			strcmp(socket_type, "Client") == 0)
			client_id = socket;
		else if(strcmp(password_proposta, "Gui") == 0  &&
				strcmp(socket_type, "Gui") == 0)
			gui_id = socket;
		else{
			free(password_proposta);
			return -1;
		}
		free(password_proposta);
	}
	free(array_value);
	return 0;
}

/* FUNCTION:	receive_socket_type
 * PARAMETERS:	int
 *RITORNO:	int
 *DESCRIZIONE:	Riceve il tipo del socket che si è collegato: Client o Gui.
*/
int receive_socket_type(int socket)
{
	int byte_tot = 0, byte_count, byte, * array_value;
	char * socket_type, *socket_type_copy;
	
	array_value = calloc(2, sizeof(int));
	byte_count = 0;
	while(byte_count < 2 *sizeof(int))
		if((byte = read(socket, array_value, 2 * sizeof(int) - byte_count)) > 0){
			syslog(LOG_DEBUG, "Read socket type len con successo\n");
			byte_count += byte;
		}
		else if(errno != EINTR){
			perror("Server socket type len ");
			return -1;
		}
	if(!array_value[no]){
		byte_tot = array_value[yes] * sizeof(char);
		socket_type = calloc(byte_tot, sizeof(char));
		socket_type_copy = socket_type;
		byte_count = 0;
		while(byte_count < byte_tot)
			if((byte = read(socket, socket_type, byte_tot - byte_count)) > 0){
				syslog(LOG_DEBUG, "Read socket type con successo\n");
				byte_count += byte;
				socket_type += byte;
			}
			else if(errno != EINTR){
				perror("Server socket type");
				return -1;
			}
		socket_type = socket_type_copy;
		syslog(LOG_INFO, "socket type %s\n", socket_type);
		if(strcmp(socket_type, "Client") == 0){
			free(array_value);
			return receive_password(socket, "Client");
		}
		else if(strcmp(socket_type, "Gui") == 0){
			free(array_value);
			return receive_password(socket, "Gui");
		}
		free(socket_type);
	}
	free(array_value);
	return -1;
}

/* FUNCTION:	send_socket_report
 * PARAMETERS:	int, int
 *RITORNO:	int
 *DESCRIZIONE:	Invia l'esito della connessione al Client o alla Gui.
*/
int send_socket_report(int socket, int esito)
{
	int byte_tot = 0, byte_count, byte, * array_value;
	char * socket_report = (!esito)? (char *){"Yes"}: (char *){"No"};
	
	byte_tot = strlen(socket_report);
	array_value = calloc(2, sizeof(int));
	array_value[1] = byte_tot;
	
	byte_count = 0;
	while(byte_count < 2 *sizeof(int))
		if((byte = write(socket, array_value, 2 * sizeof(int) - byte_count)) > 0){
			syslog(LOG_DEBUG, "Write socket report len con successo %d\n", byte_tot);
			byte_count += byte;
		}
		else if(errno != EINTR){
			perror("Server socket report len ");
			return -1;
		}
	byte_tot *= sizeof(char);
	byte_count = 0;
	while(byte_count < byte_tot)
		if((byte = write(socket, socket_report, byte_tot - byte_count)) > 0){
			syslog(LOG_DEBUG, "Write socket report con successo\n");
			byte_count += byte;
			socket_report += byte;
		}
		else if(errno != EINTR){
			perror("Server socket report");
			return -1;
		}
	free(array_value);
	return 0;
}

/* FUNCTION:	receive_autentication
 * PARAMETERS:	int
 *RITORNO:	int
 *DESCRIZIONE:	Riceve dati per autenticare il socket.
*/
int receive_autentication(int socket){
	register int esito;
	esito = receive_socket_type(socket);
	syslog(LOG_DEBUG, "esito type %d\n", esito);
	send_socket_report(socket, esito);
	return esito;
}

/* FUNCTION:	socket_accept
 * PARAMETERS:	puntatore a int
 *RITORNO:	int
 *DESCRIZIONE:	Server accetta client dal socket.
*/
int socket_accept(int * socket)
{
	do{
		*socket = accept(server_id, NULL, NULL);
	}while(*socket < 0 &&(
	errno == EINTR ||	/*	Interrupted function call	*/
	errno == EAGAIN ||	/*	Socket non_block	*/
	errno == ENETDOWN ||	/*	Cannot send after transport endpoint shutdown	*/
	errno == EPROTO ||	/*	Protocol error	*/
	errno == ENOPROTOOPT ||	/*	Protocol not available	*/
	errno == EHOSTDOWN ||	/*	Host is down	*/
	errno == ENONET ||	/*	Machine is not on the network	*/
	errno == EHOSTUNREACH ||	/*	No route to host	*/
	errno == EOPNOTSUPP ||	/*	Operation not supported on transport endpoint	*/
	errno == ENETUNREACH));	/*	Network is unreachable	*/
	if(*socket < 0){
			perror("accept");
		return -1;
	}
	else
		syslog(LOG_DEBUG, " accept\n");
	return 0;
}

/* FUNCTION:	send_room_name
 * PARAMETERS:	int, puntatore a int
 *RITORNO:	int
 *DESCRIZIONE:	Invia il nome della stanza n_room a un determinato socket.
*/
int send_room_name(int n_room, int * socket)
{
	register int i;
	int byte_tot = 0, byte_count = 0, byte, * array_value;
	char * name_room, * name_room_backup;
	for(i = 0; i < strlen(home.rooms[n_room].name); i++);

	byte_tot = i;
	array_value = calloc(2, sizeof(int));
	array_value[no] = n_room + 1;
	array_value[yes] = byte_tot;

	byte_tot *= sizeof(int);
	byte_count = 0;
	while(byte_count < 2 *sizeof(int))
		if((byte = write(*socket, array_value, 2 * sizeof(int) - byte_count)) > 0){
			syslog(LOG_DEBUG, "Write length name room con successo\n");
			byte_count += byte;
		}
		else if(errno != EINTR){
			perror("Server write length name room");
			reconnect(socket);
			return -1;
		}
	byte_tot /= sizeof(int);
	name_room = calloc(byte_tot, sizeof(char));
	strcpy(name_room, home.rooms[n_room].name);
	name_room_backup = name_room;
	byte_tot *= sizeof(char);
	byte_count = 0;
	while(byte_count < byte_tot)
		if((byte = write(*socket, name_room, byte_tot - byte_count)) > 0){
			syslog(LOG_DEBUG, "Write name room con successo\n");
			byte_count += byte;
			name_room += byte;
		}
		else if(errno != EINTR){
			perror("Server write name room");
			reconnect(socket);
			return -1;
		}
	name_room = name_room_backup;
	free(array_value);
	free(name_room);
	return 0;
}

/* FUNCTION:	send_len_array
 * PARAMETERS:	puntatore a int
 *RITORNO:	int
 *DESCRIZIONE:	Invia tramite socket quanti bytes invierà.
*/
int send_len_array(int * byte_tot)
{
	int * array_value;
	array_value = calloc(2, sizeof(int));
	array_value[yes] = *byte_tot;

	if(write(client_id, array_value, 2 * sizeof(int)) <= 0){
		reconnect(&client_id);
		return -1;
	}
	else if(debug_socket)
		syslog(LOG_DEBUG, "Write: numero interi con successo: %d\n", array_value[yes]);
	free(array_value);
	return 0;
}

/* FUNCTION:	wait_reconnect
 * PARAMETERS:	puntatore a void
 *RITORNO:	puntatore a void
 *DESCRIZIONE:	Thread che per riconnettere un client precedentemente
 * 				perso.
*/
void * wait_reconnect(void * arg)
{
	register int i;
	int temp_id;
	char socket_name[DIM_NAME];
	while(true){
		if(!socket_accept(&temp_id)){
				if(receive_autentication(temp_id)){
					close(temp_id);
					syslog(LOG_DEBUG, " close\n");
					syslog(LOG_DEBUG,"Connect not success\n");
				}
				else{
					strcpy(socket_name, (temp_id == client_id)? "Client": "Gui");
					syslog(LOG_DEBUG,"Connect %s success\n", socket_name);
					printf("\n\n%s: Connect %s success\n", print_thread_name(iserver), socket_name);
					printf("%s:@>",print_thread_name(iterminal));
					fflush(stdout);
					for(i = 0; i < home.n_room; i++)
						while(send_room_name(i, &temp_id) < 0);
					(temp_id  == client_id)? (connection = 1): (gui_connection = 1);
				}
		}
	}
	pthread_exit(0);
}

/* FUNCTION:	reconnect
 * PARAMETERS:	puntatore a int
 *RITORNO:	int
 *DESCRIZIONE:	Server resta in attesa quando si accorge che il client si è
 *             disconnesso.
*/
int reconnect(int * socket)
{
	if(socket == &client_id){
		connection = 0;
		printf("\n\n%s: Connect lost Client, try reconnect\n",
			 print_thread_name(pthread_self()));
		printf("%s:@>",print_thread_name(iterminal));
		fflush(stdout);
	}
	else{
		gui_connection = 0;
		printf("\n\n%s: Connect lost Gui, try reconnect\n",
			 print_thread_name(pthread_self()));
		printf("%s:@>",print_thread_name(iterminal));
		fflush(stdout);
	}
	return 0;
}

/* FUNCTION:	send_array
 * PARAMETERS:	puntatore a int, puntatore a int
 *RITORNO:	int
 *DESCRIZIONE:	Invia tramite socket da array.
*/
int send_array(int * copy_value, int * byte_tot)
{
	int * copy_value_backup, byte_count = 0, byte;
	copy_value_backup = copy_value;
	(*byte_tot) *= sizeof(int);
	
	while(byte_count < (*byte_tot))
		if((byte = write(client_id, copy_value, (*byte_tot) - byte_count)) > 0){
			if(debug_socket)syslog(LOG_DEBUG, "Write con successo\n");
			byte_count += byte;
			copy_value += byte;
		}
		else if(errno != EINTR){
			reconnect(&client_id);
			return -1;
		}
	copy_value = copy_value_backup;
	return 0;
}

/* FUNCTION:	receive_len_array
 * PARAMETERS:	puntatore a int
 *RITORNO:	int
 *DESCRIZIONE:	Riceve tramite socket quanti bytes riceverà.
*/
int receive_len_array(int * byte_tot)
{
	int ret;
	if(debug)syslog(LOG_INFO,"Entro receive len\n");
	if((ret = read(client_id, byte_tot, sizeof(int))) < 0){
		if(debug_socket)syslog(LOG_DEBUG,"errno %d\n", errno);
		perror("read len 1");
		reconnect(&client_id);
		return -1;
	}else if(ret == 0){
		if(debug_socket)syslog(LOG_DEBUG,"ret 0\n");
		reconnect(&client_id);
		return -1;
	}else if(*byte_tot == 0){
		if_warning = 1;
		if((ret = read(client_id, byte_tot, sizeof(int))) < 0){
			if(debug_socket)syslog(LOG_DEBUG,"errno %d\n", errno);
			perror("read len 2");
			reconnect(&client_id);
			return -1;
		}else if(ret == 0){
			if(debug_socket)syslog(LOG_DEBUG,"ret 0\n");
			reconnect(&client_id);
			return -1;
		}
		else if(debug_socket)
			syslog(LOG_DEBUG, "Read: numero interi con successo: %d\n", *byte_tot);
	}else if(if_warning){
		printf("\n\n%s\n", WARNING);
		printf("%s:@>",print_thread_name(iterminal));
		fflush(stdout);
		*byte_tot = 0;
		if_warning = 0;
	}
	if(debug)syslog(LOG_INFO,"Fine receive len\n");
	return 0;
}

/* FUNCTION:	receive_array
 * PARAMETERS:	puntatore a int, puntatore a int
 *RITORNO:	int
 *DESCRIZIONE:	Riceve tramite socket su array.
*/
int receive_array(int * copy_value, int * byte_tot)
{
	int * copy_value_backup, byte_count = 0, byte;
	if(debug)syslog(LOG_INFO,"Entro receive\n");
	copy_value_backup = copy_value;
	(*byte_tot) *= sizeof(int);
	if(*byte_tot > 0){
		while(byte_count < (*byte_tot)){
			if((byte = read(client_id, copy_value, (*byte_tot) - byte_count)) > 0){
				if(debug_socket)syslog(LOG_DEBUG, "Read con successo\n");
				byte_count += byte;
				copy_value += byte;
			}
			else if(byte < 0){
				if(errno != EINTR){
					reconnect(&client_id);
					return -1;
				}
			}
			else{
				if(debug_socket)syslog(LOG_DEBUG, "byte 0\n");
				reconnect(&client_id);
				return -1;
			}
		}
	}
	copy_value = copy_value_backup;
	if(debug)syslog(LOG_INFO,"Fine receive\n");
	return 0;
}

/* FUNCTION:	print_home
 * PARAMETERS:	void
 *RITORNO:	void
 *DESCRIZIONE:	Stampa su stdout i dati della casa.
*/
void print_home(void)
{
	register int r = 0, w, l;
	printf("Home\'s data:\n");
	printf("Door: %s\n", (home.door == closed)? CLOSED: OPENED);
	printf("Hvac: %s\n", (home.hvac == off)? OFF: ON);
	printf("Weather: %s\n", (home.weather == winter)? WINTER: SUMMER);
	printf("Hvac power: %d W\n", home.hvac_power);
	printf("Internal umidity: %d %%\n", home.internal_humidity);
	printf("External umidity: %d %%\n", home.external_humidity);
	printf("Internal heat: %2.2f °C\n", ((double)home.internal_temp)/10);
	printf("External heat: %2.2f °C\n", ((double)home.external_temp)/10);
	printf("Floor heat: %2.2f °C\n", ((double)home.floor_temp)/10);
	printf("Minimum heat: %2.2f °C\n", ((double)home.min_temp)/10);
	printf("Desired heat: %2.2f °C\n", ((double)home.des_temp)/10);
	while(r < home.n_room){
		w = 0, l = 0;
		printf("Room: %s\n", home.rooms[r].name);
		printf("	Human presence: %s\n", (home.rooms[r].presence == no)? NOBODY: SOMEBODY);
		printf("	Room power: %d W\n", home.rooms[r].power);
		while(w < home.rooms[r].n_windows){
			printf("	Window %d is %s\n", (w + 1), (home.rooms[r].windows[w] == closed)? CLOSED: OPENED);
			w++;
		}
		while(l < home.rooms[r].n_lights){
			printf("	Light %d is %s\n", (l + 1), (home.rooms[r].lights[l] == off)? OFF: ON);
			l++;
		}
		r++;
	}
}

/* FUNCTION:	refresh_gui
 * PARAMETERS:	puntatore a int, puntatore a int
 *RITORNO:	int
 *DESCRIZIONE:	Invia dati aggiornati alla gui.
*/
int refresh_gui(int * copy_value, int * byte_tot)
{
	int * array_value, * copy_value_backup, byte_count = 0, byte;
	array_value = calloc(2, sizeof(int));
	array_value[yes] = *byte_tot;
	(*byte_tot) *= sizeof(int);
	copy_value_backup = copy_value;
	
	if(write(gui_id, array_value, 2 * sizeof(int)) <= 0){
		reconnect(&gui_id);
		return -1;
	}
	else if(debug_socket)
		syslog(LOG_DEBUG, "Write gui: numero interi con successo: %d\n", array_value[yes]);
	free(array_value);
	
	while(byte_count < (*byte_tot))
		if((byte = write(gui_id, copy_value, (*byte_tot) - byte_count)) > 0){
			if(debug_socket)syslog(LOG_DEBUG, "Write gui con successo\n");
			byte_count += byte;
			copy_value += byte;
		}
		else if(errno != EINTR){
			reconnect(&gui_id);
			return -1;
		}
	copy_value = copy_value_backup;
	return 0;
}

/* FUNCTION:	esec_command
 * PARAMETERS:	void
 *RITORNO:	int
 *DESCRIZIONE:	Esegue il comando richiesto dal terminale o comunica con i
 *             client.
*/
int esec_command(void)
{
	register int i;
	int byte_tot = 0, * copy_value;
	switch (command){
		case 0:{
			if(debug)syslog(LOG_INFO, "Server ascolta socket\n");
			copy_value = get_values();
			for(i = 0; copy_value[i] != END_ARRAY; i++){
				byte_tot++;
			}
			byte_tot++;		//	terminatore dell'array
			if(gui_connection && iface_state && read_write)
				refresh_gui(copy_value, &byte_tot);
			if(connection && iface_state && read_write){
				if(send_len_array(&byte_tot) != 0)
					return 'c';
				if(send_array(copy_value, &byte_tot) != 0)
					return 'c';
				if(receive_len_array(&byte_tot) != 0)
					return 'c';
				if(receive_array(copy_value, &byte_tot) != 0)
					return 'c';

				set_values(copy_value);
				if(debug)syslog(LOG_INFO, "Struct modificata con successo\n");
			}
			free(copy_value);
			break;
		}
		case 1:{
			strcpy(cret, home.door == closed? CLOSED: OPENED);
			break;
		}
		case 2:{
			strcpy(cret, home.hvac == closed? OFF: ON);
			break;
		}
		case 3:{
			sprintf(cret, "%d W", home.hvac_power);
			break;
		}
		case 4:{
			strcpy(cret, home.weather == winter? WINTER: SUMMER);
			break;
		}
		case 5:{
			sprintf(cret, "%d %%", home.internal_humidity);
			break;
		}
		case 6:{
			sprintf(cret, "%d %%", home.external_humidity);
			break;
		}
		case 7:{
			sprintf(cret, "%2.2f °C", ((double)home.internal_temp/10));
			break;
		}
		case 8:{
			sprintf(cret, "%2.2f °C", ((double)(home.external_temp)/10));
			break;
		}
		case 9:{
			sprintf(cret, "%2.2f °C", ((double)(home.floor_temp)/10));
			break;
		}
		case 11:{
			sprintf(cret, "%2.2f °C", ((double)(home.min_temp)/10));
			break;
		}
		case 12:{
			sprintf(cret, "%2.2f °C", ((double)(home.des_temp)/10));
			break;
		}
		case 'r':{
			switch(sub_command){
				case 1:{
					strcpy(cret, home.rooms[room_command].presence == false? NOBODY: SOMEBODY);
					break;
				}
				case 2:{
					sprintf(cret, "%d W", home.rooms[room_command].power);
					break;
				}
				case 3:{
					if(number_command >= 0)
					strcpy(cret, home.rooms[room_command].windows[number_command] == closed? CLOSED: OPENED);
					else
						return -1;
					break;
				}
				case 4:{
					char temp[DIM_NAME];
					if(number_command >= 0){
						sprintf(temp, " %d", home.rooms[room_command].lights[number_command]);
						strcat(temp, " %");
						strcpy(cret, home.rooms[room_command].lights[number_command] == closed?
						OFF: temp);
					}
					else
						return -1;
					break;
				}
				case 5:{
					strcpy(cret, home.rooms[room_command].name);
					break;
				}
			}
			break;
		}
		case 10:{
			if(value.tipo == INT){
				home.door = value.union_value.int_value;
				value.union_value.int_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet door error value\n");
			break;
		}
		case 20:{
			if(value.tipo == INT){
				home.hvac = value.union_value.int_value;
				value.union_value.int_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet hvac error value\n");
			break;
		}
		case 30:{
			if(value.tipo == INT){
				home.hvac_power = value.union_value.int_value;
				value.union_value.int_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet hvac-power error value\n");
			break;
		}
		case 40:{
			if(value.tipo == INT){
				home.weather = value.union_value.int_value;
				value.union_value.int_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet weather error value\n");
			break;
		}
		case 50:{
			if(value.tipo == INT){
				home.internal_humidity = value.union_value.int_value;
				value.union_value.int_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet ih error value\n");
			break;
		}
		case 60:{
			if(value.tipo == INT){
				home.external_humidity = value.union_value.int_value;
				value.union_value.int_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet eh error value\n");
			break;
		}
		case 70:{
			if(value.tipo == DOUBLE){
				home.internal_temp = value.union_value.double_value*10;
				value.union_value.double_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet it error value\n");
			break;
		}
		case 80:{
			if(value.tipo == DOUBLE){
				home.external_temp = value.union_value.double_value*10;
				value.union_value.double_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet et error value\n");
			break;
		}
		case 90:{
			if(value.tipo == DOUBLE){
				home.floor_temp = value.union_value.double_value*10;
				value.union_value.double_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet ft error value\n");
			break;
		}case 110:{
			if(value.tipo == DOUBLE){
				home.min_temp = value.union_value.double_value*10;
				value.union_value.double_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet mt error value\n");
			break;
		}case 120:{
			if(value.tipo == DOUBLE){
				home.des_temp = value.union_value.double_value*10;
				value.union_value.double_value = -1;
				strcpy(cret, "ok");
			}
			else
				printf("\n\nSet dt error value\n");
			break;
		}
		case 'R':{
			switch(sub_command){
				case 10:{
					if(value.tipo == INT){
						home.rooms[room_command].presence = value.union_value.int_value;
						value.union_value.int_value = -1;
						strcpy(cret, "ok");
					}
					else
						printf("\n\nSet pr error value\n");
					break;
				}
				case 20:{
					if(value.tipo == INT){
						home.rooms[room_command].power = value.union_value.int_value;
						value.union_value.int_value = -1;
						strcpy(cret, "ok");
					}
					else
						printf("\n\nSet pw error value\n");
					break;
				}
				case 30:{
					if(value.tipo == INT){
						home.rooms[room_command].windows[number_command] = value.union_value.int_value;
						value.union_value.int_value = -1;
						strcpy(cret, "ok");
					}
					else
						printf("\n\nSet wi error value\n");
					break;
				}
				case 40:{
					if(value.tipo == INT){
						home.rooms[room_command].lights[number_command] = value.union_value.int_value;
						value.union_value.int_value = -1;
						strcpy(cret, "ok");
					}
					else
						printf("\n\nSet li error value\n");
					break;
				}
				case 50:{
					if(value.tipo == STRING){
						strcpy(home.rooms[room_command].name, value.union_value.string_value);
						for (i = 0; i < strlen(value.union_value.string_value);i++)
							value.union_value.string_value[i] = ' ';
						while(send_room_name(room_command, &client_id) < 0);
						if(gui_connection)
							while(send_room_name(room_command, &gui_id) < 0);
						strcpy(cret, "ok");
					}
					else
						printf("\n\nSet name error value\n");
					break;
				}
			}
			break;
		}
		case 'e':
			return 'e';
		case 's':{
			read_write = 1;
			strcpy(cret, "ok");
			break;
		}
		case 't':{
			read_write = 0;
			strcpy(cret, "ok");
			break;
		}
		case 'p':{
			print_home();
			strcpy(cret, "ok");
			break;
		}
		default:
			return -1;
	}
	return 0;
}

/* FUNCTION:	server
 * PARAMETERS:	puntatore a void
 *RITORNO:	puntatore a void
 *DESCRIZIONE:	Thread che serve il termiale e i client.
*/
void * server(void * arg)
{
	int status;
	while(true){
		sem_wait(&sem_client);

		if (debug_sem)print_semaphore();
		if(debug)syslog(LOG_INFO, "%s: in attesa di clienti...\n", print_thread_name(pthread_self()));

		if (debug_sem)print_semaphore();

		if (debug_sem)print_semaphore();
		sem_post(&sem_server);

		if (debug_sem)print_semaphore();
		sem_wait(&sem_client);		// new
		if(debug)syslog(LOG_INFO, "%s: Esegue comando...\n", print_thread_name(pthread_self()));
		
		if (debug_sem)print_semaphore();
		status = esec_command();
		if (debug_sem)print_semaphore();
		
		if (status < 0)
			strcpy(cret, strcpy(strcpy(cret, print_thread_name(pthread_self())), " Command not found...\n"));
		else if(status == 'c'){
			
		}
		sem_post(&sem_server);		// new
		if (debug_sem)print_semaphore();
		if(status != 'e')
			sem_wait(&sem_client);		// new
	}
	pthread_exit(0);
}

/* FUNCTION:	terminal
 * PARAMETERS:	puntatore a void
 *RITORNO:	puntatore a void
 *DESCRIZIONE:	Thread che richiede modifiche dei dati della casa al server.
*/
void * terminal(void * arg)
{
	char cmd[DIM_PROMT];
	int argc, i = 0, status;
	while(true){
		for(i = 0; i < DIM_PROMT; i++)
			cmd[i] = ' ';
		if (debug_sem)print_semaphore();
		status = readline(cmd, DIM_PROMT);
		if (debug_sem)print_semaphore();
		if(status == 0){
			if(debug)syslog(LOG_INFO, "%s: %s\n", print_thread_name(pthread_self()), cmd);

			if(debug)syslog(LOG_INFO, "%s: Richiede Comando\n", print_thread_name(pthread_self()));
			if (debug_sem)print_semaphore();

			if(connection && iface_state)
				sem_post(&sem_client);
			if (debug_sem)print_semaphore();

			if(connection && iface_state)
				sem_wait(&sem_server);
			if(debug)syslog(LOG_INFO, "%s: Aspetta il Server\n", print_thread_name(pthread_self()));

			if (debug_sem)print_semaphore();
			if(debug)syslog(LOG_INFO, "%s: Viene eseguito il comando...\n", print_thread_name(pthread_self()));

			if (debug_sem)print_semaphore();
			argc = count_argc(cmd);
			if(debug)syslog(LOG_INFO, "%s: argc: %d\n", print_thread_name(pthread_self()), argc);
			status = parse(argc, cmd);
			if(status < 0)
				command = -1;
			if (debug_sem)print_semaphore();
			if(connection && iface_state)
				sem_post(&sem_client);		// new
			if (debug_sem)print_semaphore();
			if(connection && iface_state)
				sem_wait(&sem_server);		// new

			if(!connection || !iface_state)
				if (esec_command() < 0)
					strcpy(cret, strcpy(strcpy(cret, print_thread_name(pthread_self())), " Command not found...\n"));

			if (debug_sem)print_semaphore();

			if(command != 'e')
				printf("%s: %s\n\n",print_thread_name(pthread_self()), cret);
			for (i = 0; i < strlen(cret);i++)
				cret[i] = ' ';
			if(status < 0)
				usage();
			if(debug)syslog(LOG_INFO, "%s: Fine servizio...", print_thread_name(pthread_self()));

			if (debug_sem)print_semaphore();
			if(connection && iface_state)
				sem_post(&sem_client);		// new
			for (i = 0; i < strlen(cmd);i++) {
				cmd[i] = ' ';
			}
			syslog(LOG_INFO, "command: %d sub: %d number: %d room: %d", command, sub_command, number_command, room_command);
		}
		status = command;
		command = 0;
		sub_command = 0;
		number_command = 0;
		room_command = 0;
		if (debug_sem)print_semaphore();
		if(status == 'e')
			process_exit(0);
	}
	pthread_exit(0);
}

/* FUNCTION:	load_home
 * PARAMETERS:	int
 *RITORNO:	int
 *DESCRIZIONE:	Carica i dati generali della casa dal file di configurazione.
*/
int	load_home(int fd)
{
	ssize_t status_read, status_sscanf;
	register int i, j;
	char row[DIM_STR], type[DIM_STR], value[DIM_STR], c;
	syslog(LOG_DEBUG, "Home\'s data:\n");
	do {
		for (i = 0;i < strlen(row); i++)
			row[i] = ' ';
		i = 0;
		c = ' ';
		do {
			status_read = read(fd, &c, sizeof(char));
			if (status_read != sizeof(char)) {
				if (errno != 0)
						perror("Read home");
					return errno;
			}
			else if (c != '\n' && c != ';') {
				row[i] = c;
				i++;
			}
		} while (c != '\n' && c != ';');
		status_sscanf = sscanf(row, "%s %s", type, value);
		
		if (status_sscanf < 0) {
			printf("\n\nSscanf home error");
			return 2;
		}
		if (strcmp(type, DOOR) == 0) {
			home.door = atoi(value);
			syslog(LOG_DEBUG, "Door: %s\n", home.door == closed? CLOSED: OPENED);
		}
		else if (strcmp(type, HVAC) == 0) {
			home.hvac = atoi(value);
			syslog(LOG_DEBUG, "Hvac: %s\n", home.hvac == closed? OFF: ON);
		}
		else if (strcmp(type, WEATHER) == 0) {
			home.weather = atoi(value);
			syslog(LOG_DEBUG, "Weather: %s\n", home.weather == winter? WINTER: SUMMER);
		}
		else if (strcmp(type, HVAC_POWER) == 0) {
			home.hvac_power = atoi(value);
			syslog(LOG_DEBUG, "Hvac power: %d W\n", home.hvac_power);
		}
		else if (strcmp(type, INT_HUMIDITY) == 0) {
			home.internal_humidity = atoi(value);
			syslog(LOG_DEBUG, "Internal humidity: %d %%\n", home.internal_humidity);
		}
		else if (strcmp(type, EXT_HUMIDITY) == 0) {
			home.external_humidity = atoi(value);
			syslog(LOG_DEBUG, "External humidity: %d %%\n", home.external_humidity);
		}
		else if (strcmp(type, INT_TEMP) == 0) {
			home.internal_temp = atof(value)*10;
			syslog(LOG_DEBUG, "Internal temp: %2.2f °C\n", ((double)home.internal_temp)/10);
		}
		else if (strcmp(type, EXT_TEMP) == 0) {
			home.external_temp = atof(value)*10;
			syslog(LOG_DEBUG, "External temp: %2.2f °C\n", ((double)home.external_temp)/10);
		}
		else if (strcmp(type, FLOOR_TEMP) == 0) {
			home.floor_temp = atof(value)*10;
			syslog(LOG_DEBUG, "Floor temp: %2.2f °C\n", ((double)home.floor_temp)/10);
		}
		else if (strcmp(type, MIN_TEMP) == 0) {
			home.min_temp = atof(value)*10;
			syslog(LOG_DEBUG, "Min temp: %2.2f °C\n", ((double)home.min_temp)/10);
		}
		else if (strcmp(type, DES_TEMP) == 0) {
			home.des_temp = atof(value)*10;
			syslog(LOG_DEBUG, "Desired temp: %2.2f °C\n", ((double)home.des_temp)/10);
		}
	} while (c != ';');
	home.n_room = 0;
	for(i = 0; i < DIM_ROOM; i++){
		strcpy(home.rooms[i].name, "");
		home.rooms[i].number = 0;
		home.rooms[i].presence = 0;
		home.rooms[i].power = 0;
		home.rooms[i].n_windows = 0;
		home.rooms[i].n_lights = 0;
		for(j = 0; j < (DIM_ROOM)/2; j++){
			home.rooms[i].windows[j] = 0;
			home.rooms[i].lights[j] = 0;
		}
	}
	return 0;
}

/* FUNCTION:	load_rooms
 * PARAMETERS:	int
 *RITORNO:	int
 *DESCRIZIONE:	Carica i dati generali delle camere della casa dal
 *             file di configurazione.
*/
int	load_rooms(int fd)
{
	ssize_t status_read, status_sscanf;
	register int i, j, k;
	char row[DIM_STR], sub_row[DIM_STR], type[DIM_STR], value[DIM_STR], c;
	do{
			for (i = 0;i < strlen(row); i++)
				row[i] = ' ';
			i = 0;
			c = ' ';
			do{
				status_read = read(fd, &c, sizeof(char));
				if (status_read != sizeof(char)) {
					if (errno != 0){
						perror("Read room primo");
						return errno;
					}
					else
						return 0;
				}
				else if (c != '{' && c != '\n'){
					if(c != '.'){
						home.rooms[home.n_room].name[i] = c;
						i++;
					}
					else
						return 0;
				}
			} while (c != '{');
			syslog(LOG_DEBUG, "Room %d: %s\n", home.n_room, home.rooms[home.n_room].name);
			home.rooms[home.n_room].number = home.n_room;
			for (i = 0;i < strlen(row); i++)
				row[i] = ' ';
			i = 0;
			c = ' ';
			do{
				status_read = read(fd, &c, sizeof(c));
				if (status_read != sizeof(c)){
					if (errno != 0){
						perror("Read room secondo");
						return errno;
					}
					else
						return 0;
				}
				if (c != '}') {
					row[i] = c;
					i++;
				}
		} while (c != '}');
		
		for (i = 0;i < strlen(sub_row); i++)
				sub_row[i] = ' ';
		for (i = 0, j = 0; i <strlen(row); i++) {
			if (row[i] != ','){
				sub_row[j] = row[i];
				j++;
			}
			else {
				sub_row[j] = '\0';
				j = 0;
				status_sscanf = sscanf(sub_row, "%s %s", type, value);
				if (status_sscanf < 0) {
					printf("\n\nSscanf room error");
					return 2;
				}
				if (strcmp(type, PRESENCE) == 0) {
					home.rooms[home.n_room].presence = atoi(value);
					syslog(LOG_DEBUG, "   Human Presence: %s\n", home.rooms[home.n_room].presence == false? NOBODY: SOMEBODY);
				}
				else if (strcmp(type, WINDOW) == 0) {
					home.rooms[home.n_room].windows[home.rooms[home.n_room].n_windows] = atoi(value);
					syslog(LOG_DEBUG, "   Window %d: %s\n", home.rooms[home.n_room].n_windows+1, home.rooms[home.n_room].windows[home.rooms[home.n_room].n_windows] == closed? CLOSED: OPENED);
					home.rooms[home.n_room].n_windows++;
				}
				else if (strcmp(type, LIGHT) == 0) {
					char temp[DIM_NAME];
					home.rooms[home.n_room].lights[home.rooms[home.n_room].n_lights] = atoi(value);
					sprintf(temp, "%d", home.rooms[home.n_room].lights[home.rooms[home.n_room].n_lights]);
					strcat(temp, " %");
					syslog(LOG_DEBUG, "   Light %d: %s\n", home.rooms[home.n_room].n_lights+1, home.rooms[home.n_room].lights[home.rooms[home.n_room].n_lights] == closed? OFF: temp);
					home.rooms[home.n_room].n_lights++;
				}
				for (k = 0;k < strlen(sub_row); k++)
					sub_row[k] = ' ';
			}
		}
		home.n_room++;
	} while (c != '.');
	return 0;
}

/* FUNCTION:	load_config_file
 * PARAMETERS:	void
 *RITORNO:	int
 *DESCRIZIONE:	Carica i dati della casa dal file di configurazione.
*/
int	load_config_file(void)
{
	int fd, status;
	syslog(LOG_DEBUG, "Start load config file\n");
	if((fd = open(filename, O_RDONLY)) < 0){
		perror("Open conf");
		return errno;
	}
	if((posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL)) < 0)
		perror("posix_fadvise");
	status = load_home(fd);
	if (status != 0)
		return status;
	status = load_rooms(fd);
	if (status != 0)
		return status;
	if (close(fd) == -1){
		perror("Close conf");
		return errno;
	}
	syslog(LOG_DEBUG, "End load config file\n");
	return 0;
}

/* FUNCTION:	controlla_connect
 * PARAMETERS:	void
 *RITORNO:	void
 *DESCRIZIONE:	Controlla se la connessione sull' interfaccia selezionata è up.
*/
void controlla_connect(void)
{
	register int ret;
	ifreq_flags = iflist.ifc_req[scelta];
	ret = ioctl(server_id, SIOCGIFFLAGS, &ifreq_flags);
	if(ret < 0){
		printf("\n");
		perror("Ioctl controlla connect");
	}
	if(ifreq_flags.ifr_flags & IFF_UP){
		if(iface_state == 0){
			printf("\n");
			printf("\n%s: Interface %s is UP\n", print_thread_name(iserver), iflist.ifc_req[scelta].ifr_name);
			printf("%s:@>",print_thread_name(iterminal));
			fflush(stdout);
		}
		iface_state = 1;
	}
	else{
		if(iface_state != 0){
			printf("\n");
			printf("\n%s: Interface %s is DOWN\n", print_thread_name(iserver), iflist.ifc_req[scelta].ifr_name);
			printf("%s:@>",print_thread_name(iterminal));
			fflush(stdout);
		}
		iface_state = 0;
	}
}

/* FUNCTION:	comunication_client
 * PARAMETERS:	puntatore a void
 *RITORNO:	puntatore a void
 *DESCRIZIONE:	Thread per richiedere al server di comunicare con i client.
*/
void * comunication_client(void * arg)
{
	register int i = 0;
	while(true){
		sleep(TEMPO_CLIENT);
		controlla_connect();
		sem_post(&sem_client);
		if (debug_sem)print_semaphore();
		sem_wait(&sem_server);
		if(debug)syslog(LOG_INFO, "%s: Richiede Comunicazione socket\n", print_thread_name(pthread_self()));

		if (debug_sem)print_semaphore();
		if(debug)syslog(LOG_INFO, "%s: Comunica con il Server...\n", print_thread_name(pthread_self()));

		if (debug_sem)print_semaphore();
		sem_post(&sem_client);		// new
		if (debug_sem)print_semaphore();
		sem_wait(&sem_server);		// new
		if (debug_sem)print_semaphore();

		for (i = 0; i < strlen(cret);i++)
			cret[i] = ' ';
		if(debug)syslog(LOG_INFO, "%s: Fine comunicazione...", print_thread_name(pthread_self()));

		if (debug_sem)print_semaphore();
		sem_post(&sem_client);		// new
	}
}

/* FUNCTION:	socket_create
 * PARAMETERS:	puntatore a char
 *RITORNO:	void
 *DESCRIZIONE:	Crea il socket.
*/
void socket_create(char * ip)
{
	struct sockaddr_in server_sock;
	struct in_addr server_addr;

	if(ip != NULL)
		inet_aton(ip, &server_addr);
	else
		server_addr.s_addr = ntohl(INADDR_LOOPBACK);
	server_sock.sin_family = AF_INET;
	server_sock.sin_addr = server_addr;
	server_sock.sin_port = PORT;
	printf("IP Server: %s\n\n", inet_ntoa(server_addr));
	syslog(LOG_DEBUG, " port: %d\n", server_sock.sin_port);
	
	/*if(fcntl(server_id, F_SETFL, O_NONBLOCK) < 0)
		perror("fcntl non block");*/
	
	if (bind(server_id,(struct sockaddr *) &server_sock, sizeof(server_sock)) == -1){
		perror("Bind");
		exit(errno);
	}

	syslog(LOG_DEBUG, "Bind\n");
	if (listen(server_id, SOMAXCONN) == -1){
		perror("Listen");
		exit(errno);
	}
	syslog(LOG_DEBUG, "Listen\n");
}

/* FUNCTION:	init_semaphore
 * PARAMETERS:	void
 *RITORNO:	void
 *DESCRIZIONE:	Inizializza i valori sei semafori.
*/
void init_semaphore(void)
{
	sem_init(&sem_server, 0, 0);
	sem_init(&sem_client, 0, 0);
}

/* FUNCTION:	handler_option
 * PARAMETERS:	int, puntatore a puntatore a char
 *RITORNO:	void
 *DESCRIZIONE:	Controlla se ci sono parametri passati al programma.
*/
void handler_option(int argc, char ** argv)
{
	register int i;
	char dest[DIM_STR], path_pwd[DIM_STR];
	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "man") == 0){
			print_user_manual(strcat(get_pwd(path_pwd), SOURCE_PROGRAM), NULL);
			exit(0);
		}
		else if(strcmp(argv[i], "usman") == 0){
			strcat(get_pwd(dest), USER_MANUAL_FILE);
			print_user_manual(strcat(get_pwd(path_pwd), SOURCE_PROGRAM), dest);
			exit(0);
		}
		else if(strcmp(argv[i], "help") == 0);{
			print_help(argv[0]);
			exit(0);
		}
	}
}

/* FUNCTION:	process_allarm
 * PARAMETERS:	int
 *RITORNO:	void
 *DESCRIZIONE:	Ogni SECOND_FOR_SAVE_STRUCT salva la struttura su file di backup.
*/
void process_allarm(int status)
{
	if(!config_file_finish)
		save_struct();
	alarm(SECOND_FOR_SAVE_STRUCT);
}

/* FUNCTION:	handler_signal
 * PARAMETERS:	void
 *RITORNO:	void
 *DESCRIZIONE:	Inizializza gli handler dei segnali.
*/
void handler_signal(void)
{
	sigset_t *set_no = malloc(sizeof(sigset_t));
	sigset_t *set_all = malloc(sizeof(sigset_t));
	struct sigaction *sig_process_exit = malloc(sizeof(struct sigaction));
	struct sigaction *sig_process_allarm = malloc(sizeof(struct sigaction));
	struct sigaction *sig_pipe = malloc(sizeof(struct sigaction));

	if(sigemptyset(set_no) < 0)
		perror("Sigemptyset set_no");
	if(sigfillset(set_all) < 0)
		perror("Sigfillset set_all");
	sig_process_exit->sa_handler = process_exit;
	sig_process_exit->sa_mask = *set_all;
	sig_process_exit->sa_flags = 0;
	if(sigaction(SIGTERM, sig_process_exit, NULL) < 0)	//	C-\ termina
		perror("Sigaction SIGTERM");
	if(sigaction(SIGINT, sig_process_exit, NULL) < 0)		//	C-c Interrupt da tastiera
		perror("Sigaction SIGINT");
	sig_pipe->sa_handler = SIG_IGN;
	sig_pipe->sa_mask = *set_no;
	sig_pipe->sa_flags = 0;
	if(sigaction(SIGPIPE, sig_pipe, NULL) < 0)
		perror("Sigaction SIGPIPE");
	sig_process_allarm->sa_handler = process_allarm;
	sig_process_allarm->sa_mask = *set_all;
	sig_process_allarm->sa_flags = 0;
	if(sigaction(SIGALRM, sig_process_allarm, NULL) < 0)
		perror("Sigaction SIGALRM");
	alarm(SECOND_FOR_SAVE_STRUCT);

	free(set_no);
	free(set_all);
	free(sig_process_exit);
	free(sig_pipe);
	free(sig_process_allarm);
}

/* FUNCTION:	find_iface
 * PARAMETERS:	int
 *RITORNO:	int
 *DESCRIZIONE:	cerca l'interfaccia di rete libera e inizializza il socket con il
 *             corrispondente indirizzo ip.
*/
int find_iface(void)
{
	register int ret;
	iflist.ifc_len = sizeof(buffer);
	iflist.ifc_buf = buffer;
	ret = ioctl(server_id, SIOCGIFCONF, &iflist);
	if(ret < 0){
		perror("Ioctl find_iface primo");
		return errno;
	}
	if(iflist.ifc_len == sizeof(buffer)){
		printf("\nProbable overflow, too many interfaces, cannot read\n");
		return -1;
	}else
		n_iface = iflist.ifc_len / sizeof(struct ifreq); 
	return window_iface();
}

/*	Gui function	*/

/* FUNCTION:	file_chooser_new
 * PARAMETERS:	puntatore a GtkWidget
 *RITORNO:	void
 *DESCRIZIONE:	Fa scieglere il file di configurazione da aprire.
*/
void file_chooser_open(GtkWidget * parent_window)
{
	general = 0;
	gint context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_general), "vuoto");
	gtk_statusbar_push (GTK_STATUSBAR(status_general), context_id, "");
	
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Open File",
					      GTK_WINDOW(parent_window),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);
	
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		filename = NULL;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		config_file_load  = load_config_file();
		config_file_home = -1;
		config_file_room = -1;
		config_file_done = 0;
		config_file_finish = 0;
	}
	gtk_widget_destroy (dialog);
}

/* FUNCTION:	create_home
 * PARAMETERS:	puntatore a char
 *RITORNO:	int
 *DESCRIZIONE:	Crea i dati generali della casa.
*/
int	create_home(char config_file[FILENAME_MAX])
{
	FILE * f;
	int status = -1, i, decimal, sign;  
	char *buffer, type[DIM_NAME];
	syslog(LOG_INFO, "Start create config file %s\n", filename);
	if((f = fopen(config_file, "w")) == NULL)
		perror("fopen");
	else{	
		fprintf(f, "%s %d\n%s %d\n%s %d\n%s %d\n%s %d\n%s %d",
			  DOOR, cc[0].union_value.int_value ,
			  HVAC, cc[1].union_value.int_value ,
			  HVAC_POWER, (int)cc[2].union_value.double_value ,
			  WEATHER, cc[3].union_value.int_value ,
			  INT_HUMIDITY, (int)cc[4].union_value.double_value,
			  EXT_HUMIDITY, (int)cc[5].union_value.double_value);
		for(i = 6; i < CC_NUM; i++){
			switch(cc[i].tipo){
				case it:{
					strcpy(type, INT_TEMP);
					break;
				}
				case et:{
					strcpy(type, EXT_TEMP);
					break;
				}
				case ft:{
					strcpy(type, FLOOR_TEMP);
					break;
				}
				case mt:{
					strcpy(type, MIN_TEMP);
					break;
				}
				case dt:{
					strcpy(type, DES_TEMP);
					break;
				}
				default:{
					printf("Error type!\n");
				}
			}
			buffer = ecvt(cc[i].union_value.double_value, 4, &decimal, &sign);
			sign = (sign)? '-': '+';
			((cc[i].union_value.double_value >= 10) || (cc[i].union_value.double_value <= -10))?
			fprintf(f, "\n%s %c%c%c,%c%c", type, sign, buffer[0], buffer[1], buffer[2], buffer[3]):
			((cc[i].union_value.double_value >= 1) || (cc[i].union_value.double_value <= -1))?
			fprintf(f, "\n%s %c%c%c,%c%c", type, sign,'0',buffer[0], buffer[1], buffer[2]):
			fprintf(f, "\n%s %c%c%c,%c%c", type, sign,'0','0', buffer[0], buffer[1]);
		}
		fprintf(f, ";");
		if((status = fclose(f)) == -1)
		perror("Close conf");
	}
	syslog(LOG_INFO, "End create config file general\n");
	return status;
}

/* FUNCTION:	file_chooser_new
 * PARAMETERS:	puntatore a GtkWidget
 *RITORNO:	void
 *DESCRIZIONE:	Fa scieglere la path del file di configurazione
 * 				da creare.
*/
void file_chooser_new(GtkWidget * parent_window)
{
	int user_edited_a_new_document = 0;
	char default_folder_for_saving[DIM_PROMT], filename_for_existing_document[DIM_NAME];
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Save File",
				      GTK_WINDOW(parent_window),
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	if(user_edited_a_new_document){
	    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), default_folder_for_saving);
	    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Untitled document");
	}
	else
	  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), filename_for_existing_document);
	
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	if(gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
		filename = NULL;
	    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	    config_file_home = create_home(filename);
	    config_file_room = -1;
		config_file_load = -1;
		config_file_done = -1;
		config_file_finish = -1;
	}
	gtk_widget_destroy (dialog);
}

/* FUNCTION:	create_rooms
 * PARAMETERS:	int
 *RITORNO:	int
 *DESCRIZIONE:	Create i dati generali delle camere della casa dal
 *             file di configurazione.
*/
int	create_rooms(void)
{
	FILE * f;
	int status = -1;
	register int incrW = 0, incrL = 0;
	char * win = ",window", * lig = ",light";
	syslog(LOG_INFO, "Start create room file %s\n", filename);
	if((f = fopen(filename, "a")) == NULL)
		perror("fopen");
	else{
			fprintf(f,"\n%s{presence %d",
				(char * ) gtk_entry_get_text (GTK_ENTRY(name_room)),
				(strcmp(gtk_button_get_label (GTK_BUTTON(presence[0])), NOBODY) == 0)? 0: 1);
			incrW = 0;
			incrL = 0;
			while (incrW < n_wi){
				fprintf(f, "%s %d",
						  win, (strcmp(gtk_button_get_label (GTK_BUTTON(widget_wi[incrW])), CLOSED) == 0)? 0: 1);
				incrW++;
			}
			while (incrL < n_li){
				fprintf(f, "%s %d",
						  lig, (int)gtk_range_get_value (GTK_RANGE(widget_li[incrL])));
				incrL++;
			}
			fprintf(f, "%s", ",}");
			if((status = fclose(f)) == -1)
				perror("Close conf");
	}
	syslog(LOG_INFO, "End create room file\n");
	config_file_room = status;
	config_file_load = -1;
	return status;
}

/* FUNCTION:	check_done_room
 * PARAMETERS:	puntatore a GtkWidget
 *RITORNO:	void
 *DESCRIZIONE:	Controlla se si sono inseriti tutti i dati della stanza.
*/
void check_done_room(GtkWidget *parent)
{
	gint context_id;
	if(strcmp(((char *)gtk_entry_get_text (GTK_ENTRY(name_room))), "") == 0){
		gtk_widget_grab_focus(name_room);
		context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_room), "room name");
		gtk_statusbar_push (GTK_STATUSBAR(status_room), context_id, "You must insert room name!");
	}
	else{
		create_rooms();
		config_file_done = 0;
		gtk_widget_destroy(parent);
	}
}

/* FUNCTION:	create_point
 * PARAMETERS:	void
 *RITORNO:	int
 *DESCRIZIONE:	scrive sul file di configurazione da creare il
 * 				punto finale.
*/
int create_point(void)
{
	FILE * f;
	int status = -1;
	syslog(LOG_INFO, "Start point file %s\n", filename);
	if((f = fopen(filename, "a")) == NULL)
		perror("fopen");
	else{
		fprintf(f, ".\n");
		if((status = fclose(f)) == -1)
				perror("Close conf");
	}
	syslog(LOG_INFO, "End point file\n");
	return status;
}

/* FUNCTION:	check_done_home
 * PARAMETERS:	puntatore a GtkWidget
 *RITORNO:	void
 *DESCRIZIONE:	Controlla se si sono inseriti tutti i dati generali
 * 				della casa.
*/
void check_done_home(GtkWidget *parent)
{
	register int i = 0, ok = true;
	gint context_id;
	for(i = 0; i < CC_NUM; i++){
		if(i && cc[i-1].tipo == mt && cc[i].tipo == dt &&
			cc[i-1].union_value.double_value > cc[i].union_value.double_value){
			ok = false;
			context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_home), "min <= des");
			gtk_statusbar_push (GTK_STATUSBAR(status_home), context_id, "Minimal temp <= Desired temp!");
			break;
		}
	}
	if(ok){
		file_chooser_new(parent);
		gtk_widget_destroy(parent);
		general = 0;
	}
}

/* FUNCTION:	value_changed
 * PARAMETERS:	puntatore a int, gpointer
 *RITORNO:	void
 *DESCRIZIONE:	Callback per il cambio di stato generale.
*/
void value_changed(int * type, gpointer user_data)
{
	register int i;
	gint context_id;
	if(!entro){
		if(!config_file_home){
			context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_room), "vuoto");
			gtk_statusbar_push (GTK_STATUSBAR(status_room), context_id, "");
		}
		else if(!config_file_finish){
			context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_iface), "vuoto");
			gtk_statusbar_push (GTK_STATUSBAR(status_iface), context_id, "");
		}
		else{
			context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_home), "vuoto");
			gtk_statusbar_push (GTK_STATUSBAR(status_home), context_id, "");
		}
	}	
	switch(*type){
		case door:
		case hvac:
		case weather:{
			cc[*type].tipo = *type;
			if(strcmp(gtk_button_get_label (GTK_BUTTON(user_data)), radio_names[*type][0]) == 0)
				cc[*type].union_value.int_value = 0;
			else
				cc[*type].union_value.int_value = 1;
			break;
		}
		case hvac_pw:
		case ih:
		case eh:
		case it:
		case et:
		case ft:
		case mt:
		case dt:{
			cc[*type].tipo = *type;
			cc[*type].union_value.double_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON(user_data));
			break;
		}
		case iface:{
			int ret;
			if(!entro){
				scelta = -1;
				num_select = gtk_combo_box_get_active(GTK_COMBO_BOX(user_data));
				if(num_select >= 0 && num_select < n_iface){
					ifreq_flags = iflist.ifc_req[num_select];
					ret = ioctl(server_id, SIOCGIFFLAGS, &ifreq_flags);
					if(ret < 0)
						perror("Ioctl set iface");
					else if(ifreq_flags.ifr_flags & IFF_UP){
						scelta = num_select;
						iface_state = 1;
					}
					else{
						char msg[DIM_STR];
						strcat(strcpy(msg, iflist.ifc_req[num_select].ifr_name), " is down!");
						context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_iface), "iface down");
						gtk_statusbar_push (GTK_STATUSBAR(status_iface), context_id, msg);
					}
					entro = 0;
				}
			}
			entro++;
			if(entro == n_iface)
				entro = 0;
			break;
		}
		case name:
		case pr:
		case li:
		case wi: break;
		default:{
			printf("Error %d\n", *type);
		}
	}
	if(debug && general){
		printf("\ngeneral\n");
		for(i = 0; i < CC_NUM; i++){
			printf("i %d general %d", i, cc[i].tipo);
			switch(cc[i].tipo){
				case door:
				case hvac:
				case weather:{
					printf(" value %d\n", cc[i].union_value.int_value);
					break;
				}
				case hvac_pw:
				case ih:
				case eh:
				case it:
				case et:
				case ft:
				case mt:
				case dt:{
					printf(" value %2.2f\n", cc[i].union_value.double_value);
					break;
				}
				default:{
					printf(" Error\n");
				}
			}		
		}
	}
	num_select = -1;
}

/* FUNCTION:	create_home
 * PARAMETERS:	int
 *RITORNO:	int
 *DESCRIZIONE:	Crea window per i dati generali della casa.
*/
int	window_home(void)
{
	GtkWidget *home_data, *vbox, *hbox, *button, *separator,
	*label, *radio_uno, *radio_due, *unit;
	GSList * group;
	register int i, *type;
	gdouble min_t = -20.0, max_t = 50.0, step_t = 0.1;
	gdouble min_hum = 0.0, max_hum = 100.0, step_hum = 1.0;
	gdouble min_pw = 0.0, max_pw = 4000.0, step_pw = 100.0;
	char **label_names = calloc((CC_NUM+1), sizeof(char *));
	int *item_type = calloc(CC_NUM, sizeof(int));
	
	general = 1;
	gint context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_general), "vuoto");
	gtk_statusbar_push (GTK_STATUSBAR(status_general), context_id, "");
	
	radio_names[door][0] = "Close";			// 0
	radio_names[door][1] = "Open";			// 0
	radio_names[hvac][0] = "Off";			// 1
	radio_names[hvac][1] = "On";			// 1
	radio_names[weather][0] = "Winter";		// 3
	radio_names[weather][1] = "Summer";		// 3

	label_names[0] = (char []){"Door:"};
	label_names[1] = (char []){"Hvac:"};
	label_names[2] = (char []){"Hvac Power:"};
	label_names[3] = (char []){"Weather:"};
	label_names[4] = (char []){"Internal Humidity:"};
	label_names[5] = (char []){"External Humidity:"};
	label_names[6] = (char []){"Internal Temperature:"};
	label_names[7] = (char []){"External Temperature:"};
	label_names[8] = (char []){"Floor Temperature:"};
	label_names[9] = (char []){"Minimal Temperature:"};
	label_names[10] = (char []){"Desired Temperature:"};
	label_names[11] = NULL;
	
	item_type[0] = door;
	item_type[1] = hvac;
	item_type[2] = hvac_pw;
	item_type[3] = weather;
	item_type[4] = ih;
	item_type[5] = eh;
	item_type[6] = it;
	item_type[7] = et;
	item_type[8] = ft;
	item_type[9] = mt;
	item_type[10] = dt;
	
	cc = calloc(CC_NUM, sizeof(struct_value));

	home_data = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (GTK_WIDGET (home_data), 300, 445);
    gtk_window_set_title (GTK_WINDOW (home_data), "General Home's Data");
    
    vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add (GTK_CONTAINER (home_data), vbox);
	gtk_widget_show (vbox);
    
    for(i = 0; label_names[i] != NULL; i++)
    {
		hbox = gtk_hbox_new (FALSE, 0);
	    label = gtk_label_new (label_names[i]);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, margin);
		gtk_label_set_selectable(GTK_LABEL(label), TRUE);
		gtk_widget_show (label);
	    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
	    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, margin);
	    gtk_widget_show (hbox);
	    type = &(item_type[i]);
	    cc[i].tipo = item_type[i];
		switch(item_type[i])
		{
			case door:
			case hvac:
			case weather:{
				radio_uno = gtk_radio_button_new_with_label (NULL, radio_names[i][0]);
			    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radio_uno), TRUE);
			    gtk_box_pack_start (GTK_BOX (hbox), radio_uno, FALSE, FALSE, margin);
			    g_signal_connect_swapped(radio_uno, "clicked", G_CALLBACK (value_changed), type);
			    gtk_widget_show (radio_uno);
			
			    group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_uno));
				radio_due = gtk_radio_button_new_with_label (group, radio_names[i][1]);
			    gtk_box_pack_start (GTK_BOX (hbox), radio_due, FALSE, FALSE, margin);
			    g_signal_connect_swapped(radio_due, "clicked", G_CALLBACK (value_changed), type);
			    gtk_widget_show (radio_due);
			    cc[i].union_value.int_value = 0;
				break;
			}
			case hvac_pw:{
				radio_uno = gtk_spin_button_new_with_range(min_pw, max_pw, step_pw);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(radio_uno), (gdouble) 0.0);
			    gtk_box_pack_start (GTK_BOX (hbox), radio_uno, FALSE, FALSE, margin);
			    g_signal_connect_swapped(radio_uno, "changed", G_CALLBACK (value_changed), type);
			    g_signal_connect_swapped(radio_uno, "value-changed", G_CALLBACK (value_changed), type);
			    gtk_widget_show (radio_uno);
			    unit = gtk_label_new ("W");
				gtk_box_pack_start (GTK_BOX (hbox), unit, FALSE, FALSE, margin);
				gtk_widget_show (unit);
				cc[i].union_value.int_value = 0;
				break;
			}
			case ih:
			case eh:{
				radio_uno = gtk_spin_button_new_with_range(min_hum, max_hum, step_hum);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(radio_uno), (gdouble) 50.0);
			    gtk_box_pack_start (GTK_BOX (hbox), radio_uno, FALSE, FALSE, margin);
			    g_signal_connect_swapped(radio_uno, "changed", G_CALLBACK (value_changed), type);
			    g_signal_connect_swapped(radio_uno, "value-changed", G_CALLBACK (value_changed), type);
			    gtk_widget_show (radio_uno);
				unit = gtk_label_new ("%");
				gtk_box_pack_start (GTK_BOX (hbox), unit, FALSE, FALSE, margin);
				gtk_widget_show (unit);
				cc[i].union_value.double_value = 50.0;
				break;
			}
			case it:
			case et:
			case ft:
			case mt:
			case dt:{
				radio_uno = gtk_spin_button_new_with_range(min_t, max_t, step_t);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(radio_uno), (gdouble) 15.0);
			    gtk_box_pack_start (GTK_BOX (hbox), radio_uno, FALSE, FALSE, margin);
			    g_signal_connect_swapped(radio_uno, "changed", G_CALLBACK (value_changed), type);
			    g_signal_connect_swapped(radio_uno, "value-changed", G_CALLBACK (value_changed), type);
			    gtk_widget_show (radio_uno);
				unit = gtk_label_new ("°C");
				gtk_box_pack_start (GTK_BOX (hbox), unit, FALSE, FALSE, margin);
				gtk_widget_show (unit);
				cc[i].union_value.double_value = 15.0;
				break;
			}
			default:{
				printf("Error\n");
			}
		}
	}	
    separator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
    gtk_widget_show (separator);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show (hbox);

    button = gtk_button_new_with_label ("Done");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK (check_done_home), GTK_WIDGET (home_data));
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, margin);
    gtk_widget_grab_focus(button);
    gtk_widget_show (button);
    
    status_home = gtk_statusbar_new();
    gtk_box_pack_start (GTK_BOX (vbox), status_home, TRUE, TRUE, 0);
    gtk_widget_show (status_home);
    
    gtk_window_set_modal (GTK_WINDOW (home_data), TRUE);
    gtk_window_set_position(GTK_WINDOW(home_data), GTK_WIN_POS_CENTER);
    gtk_widget_show(home_data);
    
    free(label_names);
	return 0;
}

/* FUNCTION:	add_light
 * PARAMETERS:	puntatore a GtkWidget
 *RITORNO:	void
 *DESCRIZIONE:	Aggiunge una luce alla stanza.
*/
void add_light(GtkWidget * parent)
{
	GtkWidget *hbox, *label;
	gdouble min_l = 0.0, max_l = 100.0, step_l = 1.0;
	gint width, height, next_height;
	
	if((n_li+1) <= (DIM_ROOM)/2){
		n_li++;
		hbox = gtk_hbox_new (FALSE, 0);
		label = gtk_label_new("light");
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, margin);
		gtk_widget_show (label);
		
		widget_li = realloc(widget_li, n_li * sizeof(GtkWidget *));
		
		widget_li[n_li-1] = gtk_hscale_new_with_range (min_l, max_l, step_l);
		gtk_widget_set_size_request(GTK_WIDGET(widget_li[n_li-1]), 250, 35);
		gtk_range_set_value (GTK_RANGE(widget_li[n_li-1]), 0.0);
		gtk_box_pack_start (GTK_BOX (hbox), widget_li[n_li-1], FALSE, FALSE, margin);
		g_signal_connect_swapped(widget_li[n_li-1], "change-value", G_CALLBACK (value_changed), &(ili));
		gtk_widget_show (widget_li[n_li-1]);
		
		gtk_table_attach (GTK_TABLE(table), hbox, 0, 1, n_li, n_li+1, GTK_FILL, GTK_FILL, 2, 1);
		gtk_widget_show (hbox);
		
		gtk_widget_get_size_request(GTK_WIDGET (parent), &width, &height);
		if(width < 520 && n_wi){
			width = 520;
			gtk_widget_set_size_request(GTK_WIDGET (parent), width, height);
		}
		if(n_li > n_wi){
			height += 37;
			next_height = 135 + 37 * n_li;
			if(height < next_height)
				gtk_widget_set_size_request(GTK_WIDGET (parent), width, next_height);
			else
				gtk_widget_set_size_request(GTK_WIDGET (parent), width, height);
		}
		else{
			height += 13;
			gtk_widget_set_size_request(GTK_WIDGET (parent), width, height);
		}
		gtk_widget_show(parent);
	}
	else{
		gint context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_room), "more light");
		gtk_statusbar_push (GTK_STATUSBAR(status_room), context_id, "You can't insert more light!");
	}
}

/* FUNCTION:	add_window
 * PARAMETERS:	puntatore a GtkWidget
 *RITORNO:	void
 *DESCRIZIONE:	Aggiunge una finestra alla stanza.
*/
void add_window(GtkWidget * parent)
{
	GtkWidget *hbox, *label;
	GSList * group;
	gint width, height, next_height;
	
	if((n_wi+1) <= (DIM_ROOM)/2){
		n_wi++;
		widget_wi = realloc(widget_wi, (2*n_wi) * sizeof(GtkWidget *));
		
		hbox = gtk_hbox_new (FALSE, 0);
		label = gtk_label_new("window");
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, margin);
		gtk_widget_show (label);
		
		widget_wi[2*n_wi-2] = gtk_radio_button_new_with_label (NULL, CLOSED);
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (widget_wi[2*n_wi-2]), TRUE);
	    gtk_box_pack_start (GTK_BOX (hbox), widget_wi[2*n_wi-2], FALSE, FALSE, margin);
		g_signal_connect_swapped(widget_wi[2*n_wi-2], "clicked", G_CALLBACK (value_changed), &iwi);
		gtk_widget_show (widget_wi[2*n_wi-2]);
	
	    group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (widget_wi[2*n_wi-2]));
		widget_wi[2*n_wi-1] = gtk_radio_button_new_with_label (group, OPENED);
	    gtk_box_pack_start (GTK_BOX (hbox), widget_wi[2*n_wi-1], FALSE, FALSE, margin);
	    g_signal_connect_swapped(widget_wi[2*n_wi-1], "clicked", G_CALLBACK (value_changed), &iwi);
		gtk_widget_show (widget_wi[2*n_wi-1]);
		
		gtk_table_attach (GTK_TABLE(table), hbox, 1, 2, n_wi, n_wi+1, GTK_FILL, GTK_FILL, 2, 1);
		gtk_widget_show (hbox);
		
		gtk_widget_get_size_request(GTK_WIDGET (parent), &width, &height);
		if(width < 520 && n_li){
			width = 520;
			gtk_widget_set_size_request(GTK_WIDGET (parent), width, height);
		}
		if(n_wi > n_li){
			height += 24;
			next_height = 135 + 24 * n_wi;
			if(height < next_height)
				gtk_widget_set_size_request(GTK_WIDGET (parent), width, next_height);
			else
				gtk_widget_set_size_request(GTK_WIDGET (parent), width, height);
		}
		gtk_widget_show(parent);
	}
	else{
		gint context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_room), "more window");
		gtk_statusbar_push (GTK_STATUSBAR(status_room), context_id, "You can't insert more window!");
	}
}

/* FUNCTION:	value_changed_buffer
 * PARAMETERS:	puntatore a GtkEntryBuffer
 *RITORNO:	int
 *DESCRIZIONE:	Crea window per i dati di una stanza della casa.
*/
void value_changed_buffer(GtkEntryBuffer * buffer)
{
	register int i;
	const gchar * name = gtk_entry_buffer_get_text(buffer);
	for(i = 0; i < gtk_entry_buffer_get_length(buffer); i++)
		if(!(isalpha(name[i]) || isdigit(name[i]) || name[i] == '-' || name[i] == '_'))
			gtk_entry_buffer_delete_text(buffer, i, 1);
}

/* FUNCTION:	create_rooms
 * PARAMETERS:	int
 *RITORNO:	void
 *DESCRIZIONE:	Controlla il nome della room
*/
int	window_rooms(void)
{
	GtkWidget *room_data, *vbox, *hbox, *button, *separator, *label;
	GtkEntryBuffer* buffer;
	GSList * group;
	register int i, *type, *item_type;
	char **label_names;
	if(!config_file_home){
		label_names = calloc((if_room+1), sizeof(char *));
		item_type = calloc(if_room, sizeof(int));
		general = 0;
		n_li = 0;
		n_wi = 0;
		
		gint context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_general), "vuoto");
		gtk_statusbar_push (GTK_STATUSBAR(status_general), context_id, "");
	
		label_names[0] = (char []){"Name:"};
		label_names[1] = (char []){"Presence:"};
		label_names[2] = NULL;
		
		item_type[0] = name;
		item_type[1] = pr;
		if_room = 2;
		
		room_data = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	    gtk_widget_set_size_request (GTK_WIDGET (room_data), 300, 160);
	    gtk_window_set_title (GTK_WINDOW (room_data), "Room Home's Data");
	    
	    vbox = gtk_vbox_new(FALSE, 0);
		gtk_container_add (GTK_CONTAINER (room_data), vbox);
		gtk_widget_show (vbox);
		
	    hbox = gtk_hbox_new (FALSE, 0);
	    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
	    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	    gtk_widget_show (hbox);
	
	    button = gtk_button_new_with_label ("Add Light");
	    g_signal_connect_swapped(button, "clicked", G_CALLBACK (add_light), GTK_WIDGET (room_data));
	    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, margin);
	    gtk_widget_show (button);
	    
	    button = gtk_button_new_with_label ("Add Window");
	    g_signal_connect_swapped(button, "clicked", G_CALLBACK (add_window), GTK_WIDGET (room_data));
	    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, margin);
	    gtk_widget_show (button);
	    
	    separator = gtk_hseparator_new ();
	    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
	    gtk_widget_show (separator);
	    
	    for(i = 0; label_names[i] != NULL; i++)
	    {
			hbox = gtk_hbox_new (FALSE, 0);
		    label = gtk_label_new (label_names[i]);
			gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, margin);
			gtk_label_set_selectable(GTK_LABEL(label), TRUE);
			gtk_widget_show (label);
		    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
		    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, margin);
		    gtk_widget_show (hbox);
		    type = &(item_type[i]);
			switch(item_type[i])
			{
				case name:{
					buffer = gtk_entry_buffer_new("", 0);
					g_signal_connect_swapped(buffer, "inserted-text", G_CALLBACK (value_changed_buffer), buffer);
					name_room = gtk_entry_new_with_buffer(buffer);
					gtk_entry_set_max_length(GTK_ENTRY(name_room), DIM_NAME);
				    gtk_box_pack_start (GTK_BOX (hbox), name_room, FALSE, FALSE, margin);
				    gtk_widget_grab_focus(name_room);
				    g_signal_connect_swapped(name_room, "changed", G_CALLBACK (value_changed), type);
				    gtk_widget_show (name_room);
					break;
				}
				case pr:{
					presence[0] = gtk_radio_button_new_with_label (NULL, NOBODY);
				    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (presence[0]), TRUE);
				    gtk_box_pack_start (GTK_BOX (hbox), presence[0], FALSE, FALSE, margin);
				    g_signal_connect_swapped(presence[0], "clicked", G_CALLBACK (value_changed), type);
				    gtk_widget_show (presence[0]);
				
				    group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (presence[0]));
					presence[1] = gtk_radio_button_new_with_label (group, SOMEBODY);
				    gtk_box_pack_start (GTK_BOX (hbox), presence[1], FALSE, FALSE, margin);
				    g_signal_connect_swapped(presence[1], "clicked", G_CALLBACK (value_changed), type);
				    gtk_widget_show (presence[1]);
					break;
				}
				default:{
					printf("Error\n");
				}
			}
		}
		
		table = gtk_table_new((DIM_ROOM)/2, 2, FALSE);
		gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
	    gtk_widget_show (table);
	    
	    separator = gtk_hseparator_new ();
	    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
	    gtk_widget_show (separator);
	
	    hbox = gtk_hbox_new (FALSE, 0);
	    gtk_container_set_border_width (GTK_CONTAINER (hbox), margin);
	    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	    gtk_widget_show (hbox);
	
	    button = gtk_button_new_with_label ("Done");
	    g_signal_connect_swapped(button, "clicked", G_CALLBACK (check_done_room), GTK_WIDGET (room_data));
	    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
	    gtk_widget_show (button);
	    
	    status_room = gtk_statusbar_new();
		gtk_box_pack_start (GTK_BOX (vbox), status_room, TRUE, TRUE, 0);
		gtk_widget_show (status_room);
    
	    gtk_window_set_modal (GTK_WINDOW (room_data), TRUE);
	    gtk_window_set_position(GTK_WINDOW(room_data), GTK_WIN_POS_CENTER);
	    gtk_widget_show(room_data);
	    free(label_names);
	}
	return 0;
}

/* FUNCTION:	exit_all
 * PARAMETERS:	void
 *RITORNO:	void
 *DESCRIZIONE:	Esce dal programma
*/
void exit_all(void)
{
	gtk_main_quit();
	sem_destroy(&sem_server);
	sem_destroy(&sem_client);
	syslog(LOG_DEBUG, "The end\n");
	printf("\n");
	exit(0);
}

/* FUNCTION:	window_iface
 * PARAMETERS:	void
 *RITORNO:	int
 *DESCRIZIONE:	Crea e riempi GtkWidget per la scelta dell'interfaccia
 * 				di rete da utilizzare.
*/
int window_iface(void)
{
	GtkWidget *window, *vbox, *button, *label;
   
	register int i;
	struct sockaddr_in *address = NULL;
	char name[DIM_STR];
	gint width, height;
	
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	gtk_set_locale();
        
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Find network interface");
    gtk_container_set_border_width (GTK_CONTAINER (window), margin);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
	
	label = gtk_label_new ("Select Network interface:");
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, margin);
	gtk_widget_show (label);
	widget_iface = gtk_combo_box_new_text();
	 for(i = 0; i < n_iface; i++){
		address = ( struct sockaddr_in *) &iflist.ifc_req[i].ifr_addr;
		strcat(strcat(strcpy(name, iflist.ifc_req[i].ifr_name), ", "), inet_ntoa(address->sin_addr));
		gtk_combo_box_append_text(GTK_COMBO_BOX(widget_iface), name);
		g_signal_connect_swapped(widget_iface, "changed", G_CALLBACK (value_changed), &iiface);
	}
	gtk_box_pack_start (GTK_BOX (vbox), widget_iface, TRUE, TRUE, 0);
	gtk_widget_show (widget_iface);

	button = gtk_button_new_with_label ("Done");
	g_signal_connect_swapped(button, "clicked", G_CALLBACK (check_done_iface), GTK_WIDGET (window));
	gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);
	gtk_widget_grab_focus(button);
	gtk_widget_show (button);
	
	status_iface = gtk_statusbar_new();
    gtk_box_pack_start (GTK_BOX (vbox), status_iface, TRUE, TRUE, 0);
    gtk_widget_show (status_iface);
	
	gtk_widget_get_size_request(GTK_WIDGET (window), &width, &height);
	gtk_widget_set_size_request(GTK_WIDGET (window), 300, height);
	g_signal_connect (window, "delete-event", G_CALLBACK (exit_all), NULL);
	
    gtk_widget_show(window);
	gtk_main();
	return 0;
}

/* FUNCTION:	check_destroy
 * PARAMETERS:	puntatore a GtkWidget
 *RITORNO:	void
 *DESCRIZIONE:	Distrugge la GtkWidget se è stato caricato un file di configurazione.
*/
void check_destroy(GtkWidget * parent)
{
	gint context_id;
	if(!config_file_done){
		if(config_file_load){
			if((!config_file_home) && (!config_file_room)){
				create_point();
				config_file_finish = load_config_file();
			}
		}
		if(!config_file_finish){
			gtk_main_quit();
			gtk_widget_destroy(parent);
			config_file_home = -1;
			config_file_room = -1;
			config_file_load = -1;
			config_file_done = -1;
		}
	}
	else if(!config_file_home){
		context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_general), "one room");
		gtk_statusbar_push (GTK_STATUSBAR(status_general), context_id, "You must insert at least one room!");
	}
	else{
		context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_general), "conf file");
		gtk_statusbar_push (GTK_STATUSBAR(status_general), context_id, "You must load or create conf file!");
	}
}

/* FUNCTION:	window_box
 * PARAMETERS:	puntatore a GtkWidget
 *RITORNO:	puntatore a GtkWidget
 *DESCRIZIONE:	Riempie il GtkWidget.
*/
GtkWidget * window_box(GtkWidget * parent) 
{
    GtkWidget *hbox, *button;
    gboolean expand = FALSE, fill = FALSE;
    guint padding = 0;
    /* Create a new hbox with the appropriate homogeneous
     * and spacing settings */
    hbox = gtk_hbox_new (FALSE, 0);
    
    /* Create a series of buttons with the appropriate settings */
    button = gtk_button_new_with_label ("Load Config File");
    gtk_widget_set_size_request (GTK_WIDGET (button), 150, 30);
    g_signal_connect_swapped (button, "clicked", G_CALLBACK (file_chooser_open), parent);
    gtk_box_pack_start (GTK_BOX (hbox), button, expand, fill, padding);
    gtk_widget_show (button);
    
    button = gtk_button_new_with_label ("Add General Data");
    gtk_widget_set_size_request (GTK_WIDGET (button), 150, 30);
	g_signal_connect (button, "clicked", G_CALLBACK (window_home), parent);
    gtk_box_pack_start (GTK_BOX (hbox), button, expand, fill, padding);
    gtk_widget_show (button);
    
    button = gtk_button_new_with_label ("Add Room");
    gtk_widget_set_size_request (GTK_WIDGET (button), 150, 30);
	g_signal_connect (button, "clicked", G_CALLBACK (window_rooms), parent);
    gtk_box_pack_start (GTK_BOX (hbox), button, expand, fill, padding);
    gtk_widget_show (button);
    
    button = gtk_button_new_with_label ("Done");
    gtk_widget_set_size_request (GTK_WIDGET (button), 150, 30);
    g_signal_connect_swapped (button, "clicked", G_CALLBACK (check_destroy), parent);
    gtk_box_pack_start (GTK_BOX (hbox), button, expand, fill, padding);
    gtk_widget_grab_focus(button);
    gtk_widget_show (button);
    
    return hbox;
}

/* FUNCTION:	window_gui
 * PARAMETERS:	int, puntatore a string
 *RITORNO:	int
 *DESCRIZIONE:	Prima gui per caricare o creare file di configurazione.
*/
int window_gui(int argc, char *argv[])
{
 	GtkWidget *window, *hbox, *vbox;

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	gtk_set_locale();
	gtk_init (&argc, &argv);
        
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Init Home");
    gtk_container_set_border_width (GTK_CONTAINER (window), margin);
    g_signal_connect (window, "delete-event", G_CALLBACK (exit_all), NULL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
	    
    hbox = window_box (window);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
    gtk_widget_show (hbox);
	
	status_general = gtk_statusbar_new();
    gtk_box_pack_start (GTK_BOX (vbox), status_general, TRUE, TRUE, 0);
    gtk_widget_show (status_general);
    
    gtk_widget_show(window);
	gtk_main();
	return 0;
}

/* FUNCTION:	main
 * PARAMETERS:	int, puntatore a string
 *RITORNO:	int
 *DESCRIZIONE:	Funzione principale.
*/
int main(int argc, char** argv)
{
	void * ret_val;
	char path_pwd[DIM_STR];
	register int i, ret;
	int status;
	struct sockaddr_in *address = NULL;
	
	handler_signal();
	openlog("Server", LOG_CONS, LOG_USER);
	handler_option(argc, argv);
	init_semaphore();
	status = window_gui(argc, argv);
	
	for(i = 0; i < strlen(cret); i++)
		cret[i] = ' ';

	if((history_file = fopen(strcat(get_pwd(path_pwd), HISTORY_FILE), "a")) == NULL){
		perror("Fopen history");
	}

	if((server_id = socket(PF_INET, SOCK_STREAM, 0)) == -1){
		perror("Socket");
		exit(errno);
	}
	syslog(LOG_DEBUG, "Socket: %d\n", server_id);

	if((ret = find_iface()) != 0)
		return ret;
	address = (struct sockaddr_in *) &iflist.ifc_req[scelta].ifr_addr;
	socket_create(inet_ntoa(address->sin_addr));
	
	pthread_create(&iserver, NULL, server, NULL);
	syslog(LOG_DEBUG,"%s stared\n", print_thread_name(iserver));

	pthread_create(&iterminal, NULL, terminal,NULL);
	syslog(LOG_DEBUG, "%s stared\n", print_thread_name(iterminal));
	
	pthread_create(&iclient, NULL, comunication_client,NULL);
	syslog(LOG_DEBUG, "%s stared\n", print_thread_name(iclient));

	pthread_create(&itry, NULL, wait_reconnect, NULL);
	syslog(LOG_DEBUG,"%s stared\n", print_thread_name(itry));
	
	pthread_join(iserver, &ret_val);
	if(ret_val != 0){
		printf("\n%s Error\n", print_thread_name(iserver));
	}
	process_exit(0);
	return 0;
}
