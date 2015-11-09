#include "autoClean.h"

/*
** This function sets up the service.
*/
BOOL			InstallMyService()
{
	char		strDir[MAX_PATH + 1];
	SC_HANDLE	schSCManager;
	SC_HANDLE	schService;

	if (!GetCurrentDirectory(MAX_PATH, strDir))
	{
		writeInLogFile("InstallMyService => GetCurrentDirectory() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}
	lstrcat(strDir, "\\"SERVICE_BIN_NAME);

	if ((schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
	{
		writeInLogFile("InstallMyService => OpenSCManager() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}

	schService = CreateService(schSCManager, SERVICE_NAME, SERVICE_DESCRIPTOR,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		(LPCSTR)strDir, NULL, NULL, NULL, NULL, NULL);

	if (schService == NULL)
	{
		writeInLogFile("InstallMyService => CreateService() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}

	if (!CloseServiceHandle(schService))
	{
		writeInLogFile("InstallMyService => CloseServiceHandle() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}
	return (TRUE);
}

/*
** This function uninstalls the service.
*/
BOOL			DeleteMyService()
{
	SC_HANDLE	schSCManager;
	SC_HANDLE	hService;

	if ((schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
	{
		writeInLogFile("DeleteMyService => OpenSCManager() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}
	if ((hService = OpenService(schSCManager, SERVICE_NAME, SC_MANAGER_ALL_ACCESS)) == NULL)
	{
		writeInLogFile("DeleteMyService => OpenService() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}
	if (!DeleteService(hService))
	{
		writeInLogFile("DeleteMyService => DeleteService() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}
	if (!CloseServiceHandle(hService))
	{
		writeInLogFile("DeleteMyService => CloseServiceHandle() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}
	return (TRUE);
}

/*
** This function is called when an event is received by the service.
*/
void WINAPI ServiceCtrlHandler(DWORD Opcode)
{
	switch (Opcode)
	{
	case SERVICE_CONTROL_PAUSE:
		g_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_STOP:
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwCheckPoint = 0;
		g_ServiceStatus.dwWaitHint = 0;
		if (!SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus))
		{
			writeInLogFile("ServiceCtrlHandler => SetServiceStatus() failed with : ", GetLastError());
			writeInLogFile("\r\n", ERROR_SUCCESS);
			return;
		}
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		break;
	}
}

/*
** This function is the entering point of the service.
** It starts the ServiceCtrlHandler and calls the autoClean function.
*/
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	writeInLogFile("ServiceMain => Starting the service\r\n", ERROR_SUCCESS);

	g_ServiceStatus.dwServiceType = SERVICE_WIN32;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;

	g_ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
	if (g_ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)
	{
		writeInLogFile("ServiceMain => RegisterServiceCtrlHandler() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return;
	}

	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwCheckPoint = 0;
	g_ServiceStatus.dwWaitHint = 0;
	if (!SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus))
	{
		writeInLogFile("ServiceMain => SetServiceStatus() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return;
	}


	writeInLogFile("ServiceMain => Starting to clean\r\n", ERROR_SUCCESS);
	/*
	** ALGORITHME ICI
	*/
	char *path;

	path = malloc(MAX_PATH * sizeof(char));
	getPathToClean(&path);
	if (path == NULL)
		return;
	if (!autoClean(path))
		return;
	writeInLogFile("ServiceMain => ended without problem\r\n", ERROR_SUCCESS);
}

/*
** This is the main function.
*/
int	main(int argc, char* argv[])
{
	BOOL logfileready = TRUE;
	SERVICE_TABLE_ENTRY table[] = { { SERVICE_NAME, ServiceMain }, { NULL, NULL } };

	g_LogFile = CreateFile(SERVICE_LOG_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (g_LogFile == INVALID_HANDLE_VALUE)
	{
		logfileready = FALSE;
		fprintf(stderr, "Main => CreateFile() failed with : %d\nThe service won't log correctly...\n", GetLastError());
	}

	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-i") == 0)
			{
				if (launchInstall(logfileready) != ERROR_SUCCESS)
				{
					return (EXIT_FAILURE);
				}
			}
			else if ((strcmp(argv[i], "-p") == 0) && (argc > i + 1))
			{
				if (launchSetPath(argv[i + 1], logfileready) != ERROR_SUCCESS)
				{
					return (EXIT_FAILURE);
				}
				i++;
			}
			else if (strcmp(argv[i], "-d") == 0)
			{
				if (launchDelete(logfileready) != ERROR_SUCCESS)
				{
					return (EXIT_FAILURE);
				}
			}
			else
			{
				fprintf(stderr, "Bad argument. Expected :\n\t[-i] to install\n\t[-d] to delete\n\t[-p path] to set a path to clean");
				return (EXIT_FAILURE);
			}
		}
	}
	else
		StartServiceCtrlDispatcher(table);

	if (!CloseHandle(g_LogFile))
		fprintf(stderr, "Main => CloseHandle failed with : %d\n", GetLastError());

	return (EXIT_SUCCESS);
}