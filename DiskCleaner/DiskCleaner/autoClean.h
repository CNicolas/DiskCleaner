#pragma once

#ifndef _AC_FUNC
#define _AC_FUNC

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <Shlwapi.h>
#include <math.h>

/*
** MACROS
*/
# define SERVICE_NAME			"AutoClean"
# define SERVICE_DESCRIPTOR		"Service de nettoyage ETNA"
# define SERVICE_BIN_NAME		"AutoClean.exe"
# define SERVICE_ROOT_KEY		HKEY_LOCAL_MACHINE
# define SERVICE_PATH_TO_CLEAN	"SOFTWARE\\ETNA\\AutoClean"
//# define ROOT_KEY				HKEY_CURRENT_USER
//# define SERVICE_PATH_TO_CLEAN	"ETNA\\AutoClean"
# define SERVICE_LOG_FILE		"C:\\AutoClean.log"

/*
** GLOBALS
*/
SERVICE_STATUS			g_ServiceStatus;
SERVICE_STATUS_HANDLE	g_ServiceStatusHandle;
HANDLE					g_LogFile;

/*
** FUNCTIONS
*/
extern BOOL	InstallMyService();
extern BOOL	DeleteMyService();

extern int	launchInstall(BOOL logfileready);
extern int	launchSetPath(char *path, BOOL logfileready);
extern int	launchDelete(BOOL logfileready);
extern int	launchSetLogPath(char *path, BOOL logfileready);


extern BOOL	setPathToClean(char *path);
extern BOOL	getPathToClean(char **path);
extern BOOL	autoClean(char *path);
extern char	*goInto(char *path, char *filename);
extern BOOL	isExtensionTMP(char *filename);
extern BOOL deleteFoundFile(char *path, char *cFileName);
extern BOOL	writeInLogFile(char *log, int errorCode);

#endif