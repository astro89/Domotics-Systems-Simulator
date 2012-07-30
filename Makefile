#Utilized by the Makefiles in server,client and gui directories
#DEFINES  = -Wall
#OPTIMIZE = -lpthread
#CFLAGS   = $(DEFINES) $(OPTIMIZE)
#GTK = `pkg-config --cflags --libs gtk+-2.0`
#CC = gcc

SRCS = ./client/engine_client.c ./server/engine_server.c ./gui/mega_gui.c
HDRS =  ./client/engine_client.h ./server/engine_server.h ./gui/mega_gui.h


#compiles c source files
all: 
	cd server; make
	cd client; make
	cd gui; make
	@echo "program succesfully compiled"

#compiles the server application
server: 
	cd server; make
	
#compiles the client application
client: 
	cd client; make
	
#compiles the gui application
gui:
	cd gui; make

#launchs the program 
run: runserver runclient rungui
	@echo "program succesfully runned"

#launchs the server application
runserver:
	gnome-terminal --command=./server/server --title=SERVER
	
#launchs the client application
runclient:
	gnome-terminal --command=./client/client --title=CLIENT

#launchs the gui application	
rungui:
	gnome-terminal --command=./gui/gui --title=GUI
	
#compiles c files and launchs the program 
runall: all run
	
#print all the includes utilized by the source files
depend: 
	touch dep.dep
	makedepend ${SRCS} ${HDRS} -fdep.dep
	@echo "dependencies file succesfully created: you can find it in your current directory"
	
#print all the includes utilized by the source files
serverdep: 
	touch ./server/servdep.dep
	makedepend ./server/engine_server.c -f./server/servdep.dep
	@echo "server dependencies file succesfully created in your current directory"
	
#print all the includes utilized by the source files
clientdep: 
	touch ./client/clientdep.txt
	makedepend ./client/engine_client.c -f./client/clientdep.dep
	@echo "client dependencies file succesfully created in your current directory"
	
#print all the includes utilized by the source files
guidep: 
	touch ./gui/guidep.dep
	makedepend ./gui/mega_gui.c -f./gui/guidep.dep
	@echo "gui dependencies file succesfully created in your current directory"

#deletes the binary files
clean:
	rm -f  ./server/server
	rm -f  ./client/client
	rm -f  ./gui/gui
	@echo "binaries succesfully deleted"
	
#deletes the dependencies file
cleandep:
	rm -f  *.dep
	rm -f  ./server/*.dep
	rm -f  ./client/*.dep
	rm -f  ./gui/*.dep
	@echo "dependencies file succesfully deleted"
	
#deletes the dependencies backupfile
cleandepbak:
	rm -f  *.dep.bak
	rm -f  ./server/*.dep.bak
	rm -f  ./client/*.dep.bak
	rm -f  ./gui/*.dep.bak
	@echo "dependencies backup file succesfully deleted"

#makes a tar of the program files
tar:
	tar czvf Progetto_SO.tar  Makefile ${SRCS} ${HDRS} ./server/*.conf ./server/*.txt README
	@echo "tar succesfully created: you can find it in your current directory"
	
#makes a ps file of all the source and headers files
printps:
	more Makefile $(HDRS) $(SRCS) | enscript -2r -p listing.ps
	@echo "ps file succesfully created: you can find it in your current directory"
	
#makes the server ps file
printserverps:
	more Makefile ./server/engine_server.c  ./server/engine_server.h | enscript -2r -p serverlisting.ps
	@echo "server ps file succesfully created: you can find it in server directory"
	
#makes the client ps file
printclientps:
	more Makefile ./client/engine_client.c  ./client/engine_client.h | enscript -2r -p clientlisting.ps
	@echo "client ps file succesfully created: you can find it in client directory"
	
#makes the gui ps file
printguips:
	more Makefile ./gui/mega_gui.c  ./gui/mega_gui.h | enscript -2r -p guilisting.ps
	@echo "gui ps file succesfully created: you can find it in gui directory"
	
#deletes the ps files
cleanps:
	rm -f  *.ps
	rm -f  ./server/*.ps
	rm -f  ./client/*.ps
	rm -f  ./gui/*.ps
	@echo "ps files succesfully deleted"


# DO NOT DELETE
