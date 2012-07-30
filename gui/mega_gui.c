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



#include "mega_gui.h"

int send_socket_type(int socket)
{
	int byte_tot = 0, byte_count, byte, * array_value;
	char * socket_type = (char *){"Gui"};
	
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
			perror("Gui socket type len ");
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
			perror("Gui socket type");
			return -1;
		}
	free(array_value);
	return 0;
}

int send_password(int socket)
{
	int byte_tot = 0, byte_count, byte, * array_value;
	char *password_copy;
	syslog(LOG_INFO,"PASSWORD:%s",password);
	byte_tot = strlen(password);
	syslog(LOG_INFO,"BYTE_TOT:%d",byte_tot);
	array_value = calloc(2, sizeof(int));
	array_value[1] = byte_tot;
	
	byte_count = 0;
	while(byte_count < 2 *sizeof(int))
		if((byte = write(socket, array_value, 2 * sizeof(int) - byte_count)) > 0){
			syslog(LOG_DEBUG, "Write password len con successo\n");
			byte_count += byte;
		}
		else if(errno != EINTR){
			perror("Gui password len ");
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
			perror("Gui password");
			return -1;
		}
	password = password_copy;
	free(array_value);
	return 0;
}

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
		syslog(LOG_DEBUG, "esito %s\n", esito);
		if(strcmp(esito, "Yes") == 0);
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

int send_autentication(int socket){
	register int esito;
	if(!send_socket_type(socket))
		send_password(socket);
	esito = receive_socket_report(socket);
	return esito;
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
	syslog(LOG_DEBUG,"int setValues(int * values)\n");

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
	home.des_temp			= values[i++];
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
 * STAMPA I DATI DELLE ROOMS 
 * 
 */

int printGRooms()
{
	syslog(LOG_DEBUG,"int printGRooms()\n");
	
	int i = 0;
	
	while(g_rooms[i].init == true && i < MAX_ROOMS)
	{
		syslog(LOG_DEBUG,"ROOM: %d\n",i + 1);
		syslog(LOG_DEBUG,"LIGTH_DIM: %d\n",g_rooms[i].gr_light_dim);
		syslog(LOG_DEBUG,"WINDOW_WIDTH: %d\n",g_rooms[i].gr_window_width);//
		syslog(LOG_DEBUG,"ROOM_WIDTH: %d\n",g_rooms[i].gr_width);
		syslog(LOG_DEBUG,"ROOM_HEIGTH: %d\n",g_rooms[i].gr_heigth);
		syslog(LOG_DEBUG,"ROOM_LAYOUT: %d\n",g_rooms[i].gr_TYPE);
		syslog(LOG_DEBUG,"POINTx1: %d POINTy1: %d POINTx2: %d POINTy2: %d \n",g_rooms[i].gr_pointx1,
																	g_rooms[i].gr_pointy1,
																	g_rooms[i].gr_pointx2,
																	g_rooms[i].gr_pointy2);
		
		i++;
	}
	
	return 0;
}



/*
 * DINAMIC METHOD
 * 
 */


int initGRooms(GtkWidget * drawing_area,int n_rooms)
{
	syslog(LOG_DEBUG,"int initGRooms(GtkWidget * drawing_area,int n_rooms)\n");
	
	gint g_width;
	gint g_heigth;
	int i;
	GtkAllocation gombAlloc;

	gombAlloc = drawing_area->allocation;
	g_width = gombAlloc.width - OFFSET;
	g_heigth = gombAlloc.height - OFFSET;
	
	int points[MAX_ROOMS][4] = {
						{OFFSET/2,OFFSET/2,
						 OFFSET/2,OFFSET/2},
						{OFFSET/2+g_width/2,OFFSET/2,
						 OFFSET/2 + g_width/2,OFFSET/2 + g_heigth},
						{OFFSET/2,OFFSET/2 + g_heigth/2,
						 OFFSET/2 + g_width/2,OFFSET/2 + g_heigth/2},
						{OFFSET/2 + g_width/2,OFFSET/2 + g_heigth/2,
						 OFFSET/2 + g_width,OFFSET/2 + g_heigth/2},
						{OFFSET/2 + g_width/4,OFFSET/2,
						 OFFSET/2 + g_width/4,OFFSET/2 + g_heigth/2},
						{OFFSET/2 + g_width/4,OFFSET/2 + g_heigth/2,
						 OFFSET/2 + g_width/4,OFFSET/2 + g_heigth},
						{OFFSET/2 + 3*(g_width/4),OFFSET/2,
						 OFFSET/2 + 3*(g_width/4),OFFSET/2 + g_heigth/2},
						{OFFSET/2 + 3*(g_width/4),OFFSET/2 + g_heigth/2,
						 OFFSET/2 + 3*(g_width/4),OFFSET/2 + g_heigth },
						{OFFSET/2 + g_width/8,OFFSET/2,
						 OFFSET/2 + g_width/8,OFFSET/2 + g_heigth/2},
						{OFFSET/2 + g_width/8,OFFSET/2 + g_heigth/2,
						 OFFSET/2 + g_width/8,OFFSET/2 + g_heigth},
						{OFFSET/2 + 3*(g_width/8),OFFSET/2,
						 OFFSET/2 + 3*(g_width/8),OFFSET/2 + g_heigth/2},
						{OFFSET/2 + 3*(g_width/8),OFFSET/2 + g_heigth/2,
						 OFFSET/2 + 3*(g_width/8),OFFSET/2 + g_heigth},
						{OFFSET/2 + 5*(g_width/8),OFFSET/2,
						 OFFSET/2 + 5*(g_width/8),OFFSET/2 + g_heigth/2},
						{OFFSET/2 + 5*(g_width/8),OFFSET/2 + g_heigth/2,
						 OFFSET/2 + 5*(g_width/8),OFFSET/2 + g_heigth},
						{OFFSET/2 + 7*(g_width/8),OFFSET/2,
						 OFFSET/2 + 7*(g_width/8),OFFSET/2 + g_heigth/2},
						{OFFSET/2 + 7*(g_width/8),OFFSET/2 + g_heigth/2,
						 OFFSET/2 + 7*(g_width/8),OFFSET/2 + g_heigth}
						};
	
	switch(n_rooms)
	{
		case 0: 
			return -1;
		case 1://caso particolare, i due punti iniziali servono solo al
			   //posizionamento delle light, nn si disegna nessuna linea
		{
			i = 0;
			
			if(g_rooms[i].init == false)
			{
				strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
				g_rooms[i].pw_bak		= home.rooms[i].power;
				g_rooms[i].init 		= true;
			}
			g_rooms[i].gr_light_dim		= g_width/6/2;
			g_rooms[i].gr_window_width	= g_width/4/2;
			g_rooms[i].gr_width			= g_width;
			g_rooms[i].gr_heigth		= g_heigth;
			g_rooms[i].gr_TYPE 			= INT_ORIZ;
			g_rooms[i].gr_pointx1		= points[i][0];
			g_rooms[i].gr_pointy1		= points[i][1];
			g_rooms[i].gr_pointx2		= points[i][2];
			g_rooms[i].gr_pointy2		= points[i][3];
			
			break;
		}
		case 2:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(g_rooms[i].init == false)
				{
					strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
					g_rooms[i].pw_bak		= home.rooms[i].power;
					g_rooms[i].init 		= true;
				}
				g_rooms[i].gr_light_dim			= g_width/6/4;
				g_rooms[i].gr_window_width		= g_width/4/4;
				g_rooms[i].gr_width				= g_width/2;
				g_rooms[i].gr_heigth			= g_heigth;
				g_rooms[i].gr_TYPE 				= META_VERT;
				g_rooms[i].gr_pointx1	= points[i][0];
				g_rooms[i].gr_pointy1	= points[i][1];
				g_rooms[i].gr_pointx2	= points[i][2];
				g_rooms[i].gr_pointy2	= points[i][3];	
			}
			
			break;
		}
		case 3:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i != 1)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/6/4;
					g_rooms[i].gr_window_width		= g_width/4/8;
					g_rooms[i].gr_width				= g_width/2;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_ORIZ;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
					
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/6/4;
					g_rooms[i].gr_window_width		= g_width/4/4;
					g_rooms[i].gr_width				= g_width/2;
					g_rooms[i].gr_heigth			= g_heigth;
					g_rooms[i].gr_TYPE 				= META_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;	
		}
		case 4:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(g_rooms[i].init == false)
				{
					strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
					g_rooms[i].pw_bak		= home.rooms[i].power;
					g_rooms[i].init 		= true;
				}
				g_rooms[i].gr_light_dim			= g_width/6/4;
				g_rooms[i].gr_window_width		= g_width/4/8;
				g_rooms[i].gr_width				= g_width/2;
				g_rooms[i].gr_heigth			= g_heigth/2;
				g_rooms[i].gr_TYPE 				= QUART_ORIZ;
				g_rooms[i].gr_pointx1	= points[i][0];
				g_rooms[i].gr_pointy1	= points[i][1];
				g_rooms[i].gr_pointx2	= points[i][2];
				g_rooms[i].gr_pointy2	= points[i][3];
			}
			
			break;
		}
		case 5:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i != 0 && i != 4)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/6/4;
					g_rooms[i].gr_window_width		= g_width/4/8;
					g_rooms[i].gr_width				= g_width/2;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_ORIZ;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/6/8;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 6:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i != 1 && i != 3)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/6/8;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/6/4;
					g_rooms[i].gr_window_width		= g_width/4/8;
					g_rooms[i].gr_width				= g_width/2;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_ORIZ;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 7:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i != 3)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/6/8;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/6/4;
					g_rooms[i].gr_window_width		= g_width/4/8;
					g_rooms[i].gr_width				= g_width/2;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_ORIZ;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 8:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(g_rooms[i].init == false)
				{
					strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
					g_rooms[i].pw_bak		= home.rooms[i].power;
					g_rooms[i].init 		= true;
				}
				g_rooms[i].gr_light_dim			= g_width/6/8;
				g_rooms[i].gr_window_width		= g_width/4/16;
				g_rooms[i].gr_width				= g_width/4;
				g_rooms[i].gr_heigth			= g_heigth/2;
				g_rooms[i].gr_TYPE 				= QUART_VERT;
				g_rooms[i].gr_pointx1	= points[i][0];
				g_rooms[i].gr_pointy1	= points[i][1];
				g_rooms[i].gr_pointx2	= points[i][2];
				g_rooms[i].gr_pointy2	= points[i][3];
			}
			
			break;
		}
		case 9:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i != 0 && i != 8)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/6/8;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/18;
					g_rooms[i].gr_window_width		= g_width/4/32;
					g_rooms[i].gr_width				= g_width/8;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= OTTAV_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 10:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i!=0 && i!=2 && i!=8 && i!=9)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/10;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/18;
					g_rooms[i].gr_window_width		= g_width/4/32;
					g_rooms[i].gr_width				= g_width/8;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= OTTAV_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 11:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i!=1 && i!=3 && i!=5 && i!=6 && i!=7)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/18;
					g_rooms[i].gr_window_width		= g_width/4/32;
					g_rooms[i].gr_width				= g_width/8;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= OTTAV_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/10;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 12:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i!=1 && i!=3 && i!=6 && i!=7)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/18;
					g_rooms[i].gr_window_width		= g_width/4/32;
					g_rooms[i].gr_width				= g_width/8;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= OTTAV_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/10;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 13:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i!=3 && i!=6 && i!=7)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/18;
					g_rooms[i].gr_window_width		= g_width/4/32;
					g_rooms[i].gr_width				= g_width/8;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= OTTAV_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/10;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 14:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i!=6 && i!=7)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/18;
					g_rooms[i].gr_window_width		= g_width/4/32;
					g_rooms[i].gr_width				= g_width/8;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= OTTAV_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/10;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 15:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(i!=7)
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/18;
					g_rooms[i].gr_window_width		= g_width/4/32;
					g_rooms[i].gr_width				= g_width/8;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= OTTAV_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
				else
				{
					if(g_rooms[i].init == false)
					{
						strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
						g_rooms[i].pw_bak		= home.rooms[i].power;
						g_rooms[i].init 		= true;
					}
					g_rooms[i].gr_light_dim			= g_width/4/10;
					g_rooms[i].gr_window_width		= g_width/4/16;
					g_rooms[i].gr_width				= g_width/4;
					g_rooms[i].gr_heigth			= g_heigth/2;
					g_rooms[i].gr_TYPE 				= QUART_VERT;
					g_rooms[i].gr_pointx1	= points[i][0];
					g_rooms[i].gr_pointy1	= points[i][1];
					g_rooms[i].gr_pointx2	= points[i][2];
					g_rooms[i].gr_pointy2	= points[i][3];
				}
			}
			
			break;
		}
		case 16:
		{
			for(i = 0;i < n_rooms;i++)
			{
				if(g_rooms[i].init == false)
				{
					strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
					g_rooms[i].pw_bak		= home.rooms[i].power;
					g_rooms[i].init 		= true;
				}
				g_rooms[i].gr_light_dim			= g_width/4/18;
				g_rooms[i].gr_window_width		= g_width/4/32;
				g_rooms[i].gr_width				= g_width/8;
				g_rooms[i].gr_heigth			= g_heigth/2;
				g_rooms[i].gr_TYPE 				= OTTAV_VERT;
				g_rooms[i].gr_pointx1	= points[i][0];
				g_rooms[i].gr_pointy1	= points[i][1];
				g_rooms[i].gr_pointx2	= points[i][2];
				g_rooms[i].gr_pointy2	= points[i][3];
			}
			
			break;
		}
		default:
			return -1;
		
	}
	
	return 0;
}


/*
 * Disegna testo in un'area disegnabile
 * 
 */

void draw_text (GdkDrawable *drawable, GdkGC *gc,gint x, gint y, PangoLayout *layout) 
{ 
	syslog(LOG_DEBUG,"void draw_text (GdkDrawable *drawable, GdkGC *gc,gint x, gint y, PangoLayout *layout)\n");
	
	gdk_draw_layout (GDK_DRAWABLE(drawable), gc, x, y, layout);
	pango_layout_context_changed(layout);
}


/*
 * Disegna la casa
 * 
 */

gboolean drawStructure(GtkWidget *drawing_area/*,GdkEventExpose *event, gpointer data*/)
{
	syslog(LOG_DEBUG,"gboolean drawStructure(GtkWidget *drawing_area,GdkEventExpose *event, gpointer data)\n");
	
	gint g_width;
	gint g_heigth;
	int i = 0,j = 0;
	GtkAllocation dimAlloc; 
	GdkWindow *window = NULL;
	GdkGC * gc = NULL,*red_gc = NULL,*yellow_gc = NULL,*blue_gc = NULL,*green_gc = NULL,*background_gc = NULL;
	GdkColor _red = {0, 65535, 0, 0} ;
	GdkColor _green = {0, 0, 65535, 0} ;
	GdkColor _blue = {0, 0, 0, 65535} ;
	GdkColor _yellow = {0, 57600, 57600, 0};
	GdkColor _background = {0, 61680, 60395,58082};
   		
	window = drawing_area->window;
	dimAlloc = drawing_area->allocation;
	g_width = dimAlloc.width - OFFSET;
	g_heigth = dimAlloc.height - OFFSET;
	
	if(window != NULL)
	{
		gc = gdk_gc_new(window);
		red_gc = gdk_gc_new(window);
		gdk_gc_set_rgb_fg_color(red_gc, &_red);
		yellow_gc = gdk_gc_new(window);
		gdk_gc_set_rgb_fg_color(yellow_gc, &_yellow);
		blue_gc = gdk_gc_new(window);
		gdk_gc_set_rgb_fg_color(blue_gc, &_blue);
		green_gc = gdk_gc_new(window);
		gdk_gc_set_rgb_fg_color(green_gc, &_green);
		background_gc = gdk_gc_new(window);
		gdk_gc_set_rgb_fg_color(background_gc, &_background);
	}
	else
		printf("window_ = NULL");
		
	
	if(receiveMessage(values) == 0){
		
		initGRooms(drawing_area,home.n_room);
			
		if(window != NULL)
			gdk_draw_rectangle(window,gc,FALSE,OFFSET/2,OFFSET/2,g_width,g_heigth);
		else
			printf("window = NULL");
		
		
		if(home.door == closed)
		{
			gdk_draw_line(window,
		                  red_gc,
						  OFFSET/2, 
						  OFFSET/2 + g_heigth/4 - g_heigth/12,
						  OFFSET/2,
						  OFFSET/2 + g_heigth/4 + g_heigth/12
						  );
			gdk_draw_line(window,
		                  background_gc,
						  OFFSET/2, 
						  OFFSET/2 + g_heigth/4 - g_heigth/12,
						  OFFSET/2 - g_heigth/6,
						  OFFSET/2 + g_heigth/4 - g_heigth/12
						  );
		}
		else
		{
			gdk_draw_line(window,
		                  green_gc,
						  OFFSET/2, 
						  OFFSET/2 + g_heigth/4 - g_heigth/12,
						  OFFSET/2,
						  OFFSET/2 + g_heigth/4 + g_heigth/12
						  );
			gdk_draw_line(window,
		                  green_gc,
						  OFFSET/2, 
						  OFFSET/2 + g_heigth/4 - g_heigth/12,
						  OFFSET/2 - g_heigth/6,
						  OFFSET/2 + g_heigth/4 - g_heigth/12
						  );
		}
		
		PangoLayout *nm_layout,*pw_layout;
		
		while(i < MAX_ROOMS && g_rooms[i].init == true)
		{
					
			if(window != NULL)
			{
				gdk_draw_line(window,
		                      gc,
		                      g_rooms[i].gr_pointx1,
		                      g_rooms[i].gr_pointy1,
		                      g_rooms[i].gr_pointx2,
		                      g_rooms[i].gr_pointy2);
		                      
		        const gchar *room_name = home.rooms[i].name, *rm_bk_name = g_rooms[i].nm_bak;
		        gchar *s_int = calloc(6,sizeof(gchar)), *bk_int = calloc(6,sizeof(gchar));
				g_sprintf(s_int,"%d",home.rooms[i].power);
				g_sprintf(bk_int,"%d",g_rooms[i].pw_bak);
				gchar *room_power = g_strconcat(s_int," W", NULL), *bk_rm_power = g_strconcat(bk_int," W", NULL);
				
				nm_layout = gtk_widget_create_pango_layout(GTK_WIDGET(drawing_area),room_name);
				pw_layout = gtk_widget_create_pango_layout(GTK_WIDGET(drawing_area),room_power);
		        
		        if(!nm_drawed[i])
		        {
					PangoLayout *rm_bg_layout = gtk_widget_create_pango_layout(GTK_WIDGET(drawing_area),rm_bk_name);
					draw_text(window,
								 background_gc,
								 g_rooms[i].gr_pointx1 + g_rooms[i].gr_width -  10*strlen(room_name),
								 g_rooms[i].gr_pointy1 + g_rooms[i].gr_heigth -  g_rooms[i].gr_heigth/8,
								 rm_bg_layout);
								 
					draw_text(window,
								 gc,
								 g_rooms[i].gr_pointx1 + g_rooms[i].gr_width -  10*strlen(room_name),
								 g_rooms[i].gr_pointy1 + g_rooms[i].gr_heigth -  g_rooms[i].gr_heigth/8,
								 nm_layout);
								 
					nm_drawed[i] = true;
					strcpy(g_rooms[i].nm_bak,home.rooms[i].name);
				}
				if(!pw_drawed[i])
				{
					PangoLayout *pw_bg_layout = gtk_widget_create_pango_layout(GTK_WIDGET(drawing_area),bk_rm_power);
					draw_text(window,
						 background_gc,
						 g_rooms[i].gr_pointx1 + g_rooms[i].gr_width -  20*strlen(room_name),
						 g_rooms[i].gr_pointy1 + g_rooms[i].gr_heigth -  g_rooms[i].gr_heigth/8,
						 pw_bg_layout);
						 
					draw_text(window,
						 gc,
						 g_rooms[i].gr_pointx1 + g_rooms[i].gr_width -  20*strlen(room_name),
						 g_rooms[i].gr_pointy1 + g_rooms[i].gr_heigth -  g_rooms[i].gr_heigth/8,
						 pw_layout);
						 
					pw_drawed[i] = true;
					g_rooms[i].pw_bak = home.rooms[i].power;
				}
		                   
		        if(g_rooms[i].gr_TYPE == INT_ORIZ)
		        {
					int up = 1,down = 1;
					
					for(j = 0; j < home.rooms[i].n_lights;j++)//
					{
						if (j%2 == 0)
						{
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + g_heigth/3 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + g_heigth/3 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							
							up++;
						}
					    else
					    {
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + 2*(g_heigth/3) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + 2*(g_heigth/3) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							down++;
						}
					}
					
					up =1;down =1;
					
					for(j = 0; j < home.rooms[i].n_windows;j++)
					{
						if (j%2 == 0)
						{
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
								
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
											  
							}
							
							up++;
						}
					    else
					    {
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							}
	
							down++;
						}
					}
				}
				else if(g_rooms[i].gr_TYPE == META_VERT)
		        {
					int left = 1,rigth = 1;
					
					for(j = 0; j < home.rooms[i].n_lights;j++)
					{
						if (j%2 == 0)
						{
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + g_width/3/2 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + left*(g_heigth/5) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + g_width/3/2 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + left*(g_heigth/5) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							left++;
						}
					    else
					    {
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + 2*(g_width/3/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + rigth*(g_heigth/5) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + 2*(g_width/3/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + rigth*(g_heigth/5) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
	
							rigth++;
						}
					}
					
					int up =1,down =1;
					
					for(j = 0; j < home.rooms[i].n_windows;j++)
					{
						if (j%2 == 0)
						{
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
											  
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
							}
							
							up++;
						}
					    else
					    {
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							
							}
							down++;
						}
					}
				}
				if(g_rooms[i].gr_TYPE == QUART_ORIZ)
		        {
					int up = 1,down = 1;
					
					for(j = 0; j < home.rooms[i].n_lights;j++)
					{
						if (j%2 == 0)
						{
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + g_heigth/3/2 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + g_heigth/3/2 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							up++;
						}
					    else
					    {
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + 2*(g_heigth/3/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + 2*(g_heigth/3/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							
							down++;
						}
					}
					
					up =1,down =1;
					
					for(j = 0; j < home.rooms[i].n_windows;j++)
					{
						if (i == 0 || i == 1)
						{
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
											  
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
							}
							
							up++;
						}
					    else
					    {
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/4) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							}
							
							down++;
						}
					}
				}
				else if(g_rooms[i].gr_TYPE == QUART_VERT)
		        {
					int left = 1,rigth = 1;
					
					for(j = 0; j < home.rooms[i].n_lights;j++)
					{
						if (j%2 == 0)
						{
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + g_width/3/4 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + left*(g_heigth/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + g_width/3/4 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + left*(g_heigth/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							left++;
						}
					    else
					    {
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + 2*(g_width/3/4) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + rigth*(g_heigth/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + 2*(g_width/3/4) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + rigth*(g_heigth/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
											  
							rigth++;
						}
					}
					
					int up =1,down =1;
					
					for(j = 0; j < home.rooms[i].n_windows;j++)
					{
						if (i == 0 || i == 1 || i == 4 || i == 6)
						{
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
											  
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
											  
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
							}
							
							up++;
						}
					    else
					    {
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/8) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							}
							
							down++;
						}
					}
				}
				else if(g_rooms[i].gr_TYPE == OTTAV_VERT)
		        {
					int left = 1,rigth = 1;
					
					for(j = 0; j < home.rooms[i].n_lights;j++)
					{
						if (j%2 == 0)
						{
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + g_width/3/8 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + left*(g_heigth/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + g_width/3/8 - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + left*(g_heigth/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
	
							left++;
						}
					    else
					    {
							if(home.rooms[i].lights[j] == off)
								gdk_draw_arc(window,
											  gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + 2*(g_width/3/8) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + rigth*(g_heigth/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							else
								gdk_draw_arc(window,
											  yellow_gc,
											  TRUE,
											  g_rooms[i].gr_pointx1 + 2*(g_width/3/8) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_pointy1 + rigth*(g_heigth/5/2) - g_rooms[i].gr_light_dim/2,
											  g_rooms[i].gr_light_dim,
											  g_rooms[i].gr_light_dim,
											  0,
											  64*360);
							rigth++;
						}
					}
					
					int up =1,down =1;
					
					for(j = 0; j < home.rooms[i].n_windows;j++)
					{
						if (i == 0 || i == 1 || i == 4 || i == 6 ||
							i == 8 || i == 10 || i == 12 || i == 14	)
						{
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2
											  );
											  
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2,
											  g_rooms[i].gr_pointx1 + up*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 - g_rooms[i].gr_window_width/2
											  );
							}
							up++;
						}
					    else
					    {
							if(home.rooms[i].windows[j] == closed)
							{
								gdk_draw_line(window,
											  red_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  background_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							}
							else
							{
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) + g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
								gdk_draw_line(window,
											  blue_gc,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth,
											  g_rooms[i].gr_pointx1 + down*(g_width/5/16) - g_rooms[i].gr_window_width/2,
											  OFFSET/2 + g_heigth + g_rooms[i].gr_window_width/2
											  );
							}
							down++;
						}
					}
				}
			}
			else
				printf("window = NULL");
			
			i++;
		}
	}
	
	
	return true;
	
}


/*
 * 
 * 
 * 
 */

gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{	
  gdk_draw_rectangle (widget->window, widget->style->fg_gc[gtk_widget_get_state (widget)],
                TRUE, 0, 50, widget->allocation.width, widget->allocation.height-100);
  return TRUE;
}


/*
 * 
 * 
 * 
 */


gboolean int_temp_changed(GtkLabel * it)
{
	syslog(LOG_DEBUG,"gboolean int_temp_changed(GtkLabel * it)\n");
	
	gchar *s_int = calloc(3,sizeof(gchar));
	gchar *s_dec = calloc(3,sizeof(gchar));
	g_sprintf(s_int,"%d",home.internal_temp/10);
	g_sprintf(s_dec,"%d",abs(home.internal_temp -((home.internal_temp/10)*10)));
	gchar *string = g_strconcat("Internal_temp ",s_int,".",s_dec ,"° C", NULL);

	gtk_label_set_text( GTK_LABEL(it),
						string);

	return TRUE;
}


/*
 * 
 * 
 * 
 */


gboolean ext_temp_changed(GtkLabel * et)
{
	syslog(LOG_DEBUG,"gboolean ext_temp_changed(GtkLabel * et)\n");
	
	gchar *s_int = calloc(3,sizeof(gchar));
	gchar *s_dec = calloc(3,sizeof(gchar));
	g_sprintf(s_int,"%d",home.external_temp/10);
	g_sprintf(s_dec,"%d",abs(home.external_temp -((home.external_temp/10)*10)));
	gchar *string = g_strconcat("External_temp ",s_int,".",s_dec ,"° C", NULL);

	gtk_label_set_text( GTK_LABEL(et),
						string);

	return TRUE;
}


/*
 * 
 * 
 * 
 */


gboolean ext_hum_changed(GtkLabel * eh)
{
	
	syslog(LOG_DEBUG,"gboolean ext_hum_changed(GtkLabel * eh)\n");
	
	gchar *s_int = calloc(3,sizeof(gchar));

	g_sprintf(s_int,"%d",home.external_humidity);

	gchar *string = g_strconcat("External_hum ",s_int,"%", NULL);

	gtk_label_set_text( GTK_LABEL(eh),
						string);

	return TRUE;
}


/*
 * 
 * 
 * 
 */


gboolean floor_temp_changed(GtkLabel * ft)
{
	syslog(LOG_DEBUG,"gboolean floor_temp_changed(GtkLabel * ft)\n");
	
	gchar *s_int = calloc(3,sizeof(gchar));
	gchar *s_dec = calloc(3,sizeof(gchar));
	g_sprintf(s_int,"%d",home.floor_temp/10);
	g_sprintf(s_dec,"%d",abs(home.floor_temp -((home.floor_temp/10)*10)));
	gchar *string = g_strconcat("Floor_temp ",s_int,".",s_dec ,"° C", NULL);

	gtk_label_set_text( GTK_LABEL(ft),
						string);

	return TRUE;
}


/*
 * 
 * 
 * 
 */


gboolean int_hum_changed(GtkLabel * ih)
{
	syslog(LOG_DEBUG,"gboolean int_hum_changed(GtkLabel * ih)\n");
	
	gchar *s_int = calloc(3,sizeof(gchar));
	
	g_sprintf(s_int,"%d",home.internal_humidity);
	
	gchar *string = g_strconcat("Internal_hum ",s_int,"%", NULL);

	gtk_label_set_text( GTK_LABEL(ih),
						string);
						
	return TRUE;
}


/*
 * 
 * 
 * 
 */


gboolean gen_wth_pw_changed(GtkWidget * status_bar)
{
	syslog(LOG_DEBUG,"gboolean int_hum_changed(GtkLabel * ih)\n");
	
	gchar *s_int = calloc(8,sizeof(gchar));
	gchar *s_dec = calloc(8,sizeof(gchar));
	gchar *string = calloc(DIM_NAME,sizeof(gchar));
	
	strcpy(s_int,home.weather ? "summer": "winter");
	g_sprintf(s_dec,"%d",home.hvac_power);
	
	strcpy(string,g_strconcat("Weather: ",s_int," Power: ",s_dec, NULL));
	
	gtk_statusbar_pop(GTK_STATUSBAR(status_bar),context_id);
	context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_bar), "General_Power and weather");
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), context_id,string);

	return TRUE;
}

/*
 * 
 * 
 * 
 * 
 */

gboolean print_rooms()
{
	syslog(LOG_DEBUG,"gboolean print_rooms()\n");
	
	if(home.n_room)
	{
		int i;
		for(i = 0; i < home.n_room;i++)
		{
			if(strcmp(g_rooms[i].nm_bak,home.rooms[i].name))
			{
				printf("ROOM DIVERSA:%s %s\n",g_rooms[i].nm_bak,home.rooms[i].name);
				nm_drawed[i] = false;
			}
			if(g_rooms[i].pw_bak != home.rooms[i].power)
			{
				printf("POWER DIVERSO:%d %d\n",g_rooms[i].pw_bak,home.rooms[i].power);
				pw_drawed[i] = false;
			}
		}
	}
	
	return TRUE;
}

/*
 * 
 * PER IL SEGNALE DI EXPOSE
 * 
 */

gboolean redraw_all(GdkEventExpose *event, gpointer data)
{
	syslog(LOG_DEBUG,"gboolean redraw_all(GdkEventExpose *event, gpointer data)\n");
	
	if(home.n_room)
	{
		int i;
		for(i = 0; i < home.n_room;i++)
		{
			nm_drawed[i] = false;
		
			pw_drawed[i] = false;
		}
	}
	
	return TRUE;
}


/*
 * 
 * FUNZIONE USCITA
 * 
 */

void quit()
{

	g_source_remove (int_tmp_timer);
	
	g_source_remove (flr_tmp_timer);
	
	g_source_remove (int_hum_timer);
	
	g_source_remove (prt_roms_timer);
	
	g_source_remove (ext_tmp_timer);
	
	g_source_remove (ext_hum_timer);
	
	g_source_remove (draw_timer);
	
	g_source_remove (gen_wth_pw_timer);
	
	free(nm_drawed);
	free(pw_drawed);
	free(g_rooms);
	
	gtk_main_quit();
}

/*
 * 
 * FUNZIONE USCITA , ESCE DIRETTAMENTE
 * 
 */

void quit_init()
{

	free(nm_drawed);
	free(pw_drawed);
	free(g_rooms);
	
	gtk_main_quit();
}


/*
 * 
 * Crea la finestra che conterrà la struttura della casa.
 * 
 */

void home_gui(void)
{
	syslog(LOG_DEBUG,"void home_gui(void)\n");
	
 	GtkWidget *window,* status_bar, *vbox, *hbox_high,
			*hbox_low, *it, *et, *ih, *eh, *ft;
	
				

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	/* create a new window */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (GTK_WIDGET (window), 800, 600);
    gtk_window_set_title (GTK_WINDOW (window), "Home's Data");
    g_signal_connect (window, "delete-event",G_CALLBACK(quit), NULL);
    guint32 interval = 1050;// timer interval: here's the problem!!!! if it's too little,then the window parts that aren't
							//				   part of the drawing area aren't showed or updated on the screen... 

    /* A vbox to put a menu and a button in: */
    vbox = gtk_vbox_new (FALSE, 3);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show (vbox);

	
	hbox_high = gtk_hbox_new (FALSE, 3);
	gtk_container_set_border_width (GTK_CONTAINER (hbox_high), 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox_high, FALSE, TRUE, 0);
	gtk_widget_show (hbox_high);
	
	it = gtk_label_new ("Internal_temp 0.0° C");
	gtk_box_pack_start (GTK_BOX (hbox_high), it, TRUE, TRUE, 0);
	gtk_widget_show (it);
	
	//int_tmp_timer = g_timeout_add(interval + 15,(GtkFunction)int_temp_changed,(gpointer)it);
	
	ft = gtk_label_new ("Floor_temp 0.0° C");
	gtk_box_pack_start (GTK_BOX (hbox_high), ft, TRUE, TRUE, 0);
	gtk_widget_show (ft);
	
	//flr_tmp_timer = g_timeout_add(interval + 30,(GtkFunction)floor_temp_changed,(gpointer)ft);
	
	ih = gtk_label_new ("Internal_hum 0%");
	gtk_box_pack_start (GTK_BOX (hbox_high), ih, TRUE, TRUE, 0);
	gtk_widget_show (ih);
	
	//int_hum_timer = g_timeout_add(interval + 45,(GtkFunction)int_hum_changed,(gpointer)ih);
	
	GtkWidget *drawing_area = gtk_drawing_area_new ();
	gtk_widget_set_size_request (GTK_WIDGET(drawing_area), -1, -1);// mod 650,625
	
	gtk_widget_add_events (GTK_WIDGET(drawing_area), GDK_ALL_EVENTS_MASK);
	
	g_signal_connect (G_OBJECT (drawing_area), "expose-event", G_CALLBACK (redraw_all),NULL);
	//prt_roms_timer = g_timeout_add(interval + 60,(GtkFunction)print_rooms,NULL);
			
	gtk_box_pack_start (GTK_BOX (vbox), drawing_area, TRUE, TRUE, 0);
	gtk_widget_show (drawing_area);
	
	hbox_low = gtk_hbox_new (FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (hbox_low), 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox_low, FALSE, FALSE, 0);
	gtk_widget_show (hbox_low);
	
	et = gtk_label_new ("External_temp 0.0° C");
	gtk_box_pack_start (GTK_BOX (hbox_low), et, TRUE, TRUE, 0);
	gtk_widget_show (et);
	
	//ext_tmp_timer = g_timeout_add(interval + 75,(GtkFunction)ext_temp_changed,(gpointer)et);
	
	eh = gtk_label_new ("External_hum 0%");
	gtk_box_pack_start (GTK_BOX (hbox_low), eh, TRUE, TRUE, 0);
	gtk_widget_show (eh);
	
	//ext_hum_timer = g_timeout_add(interval + 90,(GtkFunction)ext_hum_changed,(gpointer)eh);
	
	
	status_bar = gtk_statusbar_new();
	context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(status_bar), "General_Power and weather");
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), context_id,"");
    gtk_box_pack_start (GTK_BOX (vbox), status_bar, FALSE,FALSE, 0);
    gtk_widget_show (status_bar);
    
    gen_wth_pw_timer = g_timeout_add(interval + 15,(GtkFunction)gen_wth_pw_changed,(gpointer)status_bar);
    
    int_tmp_timer = g_timeout_add(interval + 30,(GtkFunction)int_temp_changed,(gpointer)it);
    
    flr_tmp_timer = g_timeout_add(interval + 45,(GtkFunction)floor_temp_changed,(gpointer)ft);
    
    int_hum_timer = g_timeout_add(interval + 60,(GtkFunction)int_hum_changed,(gpointer)ih);
    
    ext_tmp_timer = g_timeout_add(interval + 75,(GtkFunction)ext_temp_changed,(gpointer)et);
    
    ext_hum_timer = g_timeout_add(interval + 90,(GtkFunction)ext_hum_changed,(gpointer)eh);
    
    prt_roms_timer = g_timeout_add(interval + 105,(GtkFunction)print_rooms,NULL);
    
	draw_timer = g_timeout_add (interval + 120,(GtkFunction)drawStructure,(gpointer)drawing_area);
    
    /* always display the window as the last step so it all splashes on
     * the screen at once. */
    gtk_widget_show (window);
    
    gtk_main();
    
}

/*
 * 
 * MAIN
 * 
 */

int main (int argc, char *argv[])
{
	syslog(LOG_DEBUG,"int main (int argc, char *argv[])\n");
	
	nm_drawed = calloc(MAX_ROOMS,sizeof(gboolean));
	pw_drawed = calloc(MAX_ROOMS,sizeof(gboolean));
	g_rooms = calloc(MAX_ROOMS,sizeof(g_room)+4*sizeof(int));
	values = calloc((12+home.n_room*(2+DIM_ROOM)+1),sizeof(int));
	password = calloc(DIM_NAME, sizeof(char));
	//string = calloc(DIM_NAME,sizeof(gchar));
	//s_int = calloc(8,sizeof(gchar));
	//s_dec = calloc(8,sizeof(gchar));
		
	
	gtk_init (&argc, &argv);
	gtk_set_locale ();
		
	_IP = calloc(16,sizeof(char));
	_DOOR = 0;
	signal(SIGPIPE,SIG_IGN);
	
	connection(_IP,_DOOR);
		
	if(gui_connected)
		home_gui();
	
	return 0;
}


/*
 * ------------------------------ CLOSES SOCKET --------------------------------
 * 
 * Chiude il socket attivo.
 *
 */

void closeSocket()
{
	syslog(LOG_DEBUG,"void closeSocket()\n");
	close(id_client);
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


int createSocketByIP(char* IP, int Door)
{
	syslog(LOG_DEBUG,"int createSocketByIP(char* IP, int Porta)\n");
	
	struct sockaddr_in client_address;
	struct in_addr server_addr;

	inet_aton(IP, &server_addr);

	//Tipo di indirizzo
	client_address.sin_family = AF_INET;
	client_address.sin_addr   = server_addr;
	client_address.sin_port	  = Door;

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

	if(send_autentication(id_client))
		return -1;
	return id_server;
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
	syslog(LOG_DEBUG,"int getRoomName(int n_room, int exact_value)\n");
	int nDati,success = -1;
	_try = 0;

	if ((nDati = read(id_client,home.rooms[n_room].name,exact_value)) < 0)
			tryToReconnect(success);
	else
		;

	return 0;
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
	syslog(LOG_DEBUG,"int receiveMessage(int * values)\n");

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
			if(setValues(values) != 0)
			{			
				perror("Not Inizialized, error code ");
				exit(1);
			}
			
		}
	}
	else if(exact_value[0] > 0)
	{
		getRoomName((exact_value[0] - 1),exact_value[1]);
	}
	
	return exact_value[0];
	
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
	syslog(LOG_DEBUG,"int tryToReconnect(int success)\n");
	
	printf("\n\nConnection is fallen, trying to reconnect:   ");

	while(success < 0 && _try < MAX_TRY)
	{
		printf("%c",8);
		
		success = createSocketByIP(_IP,_DOOR);
		_try++;
		
		usleep(200000);
		
	}

	if(_try == MAX_TRY)
	{
		perror("\n\nConnection timeout, error");
		exit(1);
	}

	return 0;
}

/*
 * 
 * 
 * 
 */


void check_connect_data(GtkButton *button,GtkWidget *status_bar)
{
		char *__IP = (char*)gtk_entry_get_text(GTK_ENTRY(entry_ip));
		
		char *__DOOR = (char*)gtk_entry_get_text(GTK_ENTRY(entry_door));
		
		strcpy(password,(char*)gtk_entry_get_text(GTK_ENTRY(entry_pwd)));
		
		
		if(createSocketByIP(__IP,atoi(__DOOR)) == 0)
		{
			gtk_widget_destroy(input_mask);
			gui_connected = true;
			gtk_main_quit();
		}
		else
			gtk_statusbar_push (GTK_STATUSBAR(status_bar), 
								gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar),"Wrong"),
								"Wrong values, please retry...");	
}


/*
 * CONNESSIONE
 * 
 * 
 */

int connection(char * _IP,int _DOOR)
{
	syslog(LOG_DEBUG,"int connection(char * _IP,int Porta)\n");
	
	gui_connected = false;
	GtkWidget *vbox,*label_ip,*label_door,*label_pwd,*done,*status_bar;
	const char *ip_text = "Please insert a valid IP address",
				*door_text = "Please insert a valid DOOR",
				*pwd_text = "Please insert your PASSWORD",
				*done_text = "Done";
				
		
	input_mask = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (GTK_WIDGET (input_mask), 325, 220);
    gtk_window_set_title (GTK_WINDOW (input_mask), "Please insert the required datas");
    g_signal_connect (input_mask, "delete-event",G_CALLBACK(quit_init), NULL);

	vbox = gtk_vbox_new (FALSE, 6);
    gtk_container_add (GTK_CONTAINER (input_mask), vbox);
    gtk_widget_show (vbox);
    
    label_ip = gtk_label_new(ip_text);
    gtk_container_add (GTK_CONTAINER (vbox), label_ip);
    gtk_widget_show (label_ip);
    
   	entry_ip = gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (vbox), entry_ip);
    gtk_widget_show (entry_ip);
    	    
    label_door = gtk_label_new(door_text);
    gtk_container_add (GTK_CONTAINER (vbox), label_door);
    gtk_widget_show (label_door);
    
    entry_door = gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (vbox), entry_door);
    gtk_widget_show (entry_door);
    
    label_pwd = gtk_label_new(pwd_text);
    gtk_container_add (GTK_CONTAINER (vbox), label_pwd);
    gtk_widget_show (label_pwd);
    
    entry_pwd = gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (vbox), entry_pwd);
    gtk_widget_show (entry_pwd);
    	    
    done = gtk_button_new_with_label (done_text);
    gtk_container_add (GTK_CONTAINER (vbox), done);
    gtk_widget_show (done);
    
    status_bar = gtk_statusbar_new();
	gtk_box_pack_start (GTK_BOX (vbox), status_bar, FALSE,FALSE, 0);
	gtk_widget_show (status_bar);
    
    g_signal_connect(G_OBJECT(done),"clicked",G_CALLBACK(check_connect_data),GTK_WIDGET(status_bar));
	
	gtk_widget_show (input_mask);
		
	gtk_main();
	
	return 0;
	
}
