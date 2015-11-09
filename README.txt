#####################################################################################
#																					#
#								AutoClean : ETNA									#
#																					#
#####################################################################################

Three commands are available :
	setpath => "AutoClean.exe -p path_to_clean". It sets the path to clean in the
		register, for example if you put "D:\Users\me\test", this directory will 
		be cleaned
	install => "AutoClean.exe -i". It installs the service on you system. It is now
		available in your services.msc
	delete => "AutoClean.exe -d". It deletes the service of your system. It will 
		disappear from you services.msc
		
The file with the logs of the last call is "C:\AutoClean.log"

#################################### INSTALL ########################################
To install the programm :															#
																					#
1) Run the "AutoClean_setup.exe"													#
#####################################################################################

#################################### INSTALL ########################################
To install the service :															#
																					#
2) Go into the setup directory, and run a cmd.exe with administrator rights			#
3) Call the 'setpath' and 'install' (the order doesn't matter).						#
#####################################################################################


################################### AUTOCLEAN #######################################
To launch the service :																#
																					#
4) Go in the services.msc															#
5) Find the description "Service de nettoyage ETNA"									#
6) Click on the "start service" button, or right-click -> start						#
7) Then you can stop by clicking on the "stop serice" button, or right-click -> stop#
8) The directory you entered is cleaned recursively !								#
#####################################################################################


#################################### DELETE #########################################
To remove the service																#
																					#
9) Call the 'delete' command to remove the service. 								#
#####################################################################################

################################### UNINSTALL #######################################
To uninstall the programm:															#
																					#
10) In the setup directory, simply run uninst*.exe									#
#####################################################################################