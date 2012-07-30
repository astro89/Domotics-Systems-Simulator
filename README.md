Domotics-Systems-Simulator
==========================
PORT: 6530

AUTHORS:

Matteo Micheletti kingletti88@gmail.com 
Simone Serafini astro.simone@gmail.com

1.Makefile use
2.Makefile options

1.MAKEFILE USE:

	Open a shell and place to your current directory, then type the  command 
	make [OPTION].

	For a detailed denscription of the options, see below.

2.MAKEFILE OPTIONS:

	#compiles c source files
	all
	
	#compiles c files and launchs the program 
	runall
	
	#compiles the server application
	server
		
	#compiles the client application
	client:
	
	#compiles the gui application
	gui
	
	#launchs the program 
	run
	
	#launchs the server application
	runserver
		
	#launchs the client application
	runclient
	
	#launchs the gui application	
	rungui
		
	#print all the includes utilized by the source files
	depend
		
	#print all the includes utilized by the source files
	serverdep
		
	#print all the includes utilized by the source files
	clientdep
		
	#print all the includes utilized by the source files
	guidep
	
	#deletes the binary files
	clean
		
	#deletes the dependencies file
	cleandep
		
	#deletes the dependencies backupfile
	cleandepbak
	
	#makes a tar of the program files
	tar
		
	#makes a ps file of all the source and headers files
	printps
		
	#makes the server ps file
	printserverps
		
	#makes the client ps file
	printclientps
		
	#makes the gui ps file
	printguips
		
	#deletes the ps files
	cleanps
