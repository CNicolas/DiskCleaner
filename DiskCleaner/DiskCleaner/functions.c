#include "autoClean.h"

/*
** This function is the main algorithm of cleaning.
** It is called recursively in the path given to delete *.tmp or temporary files.
*/
BOOL				autoClean(char *path)
{
	WIN32_FIND_DATA	FindFileData;
	HANDLE			hFind;
	char			*fullpath = malloc(MAX_PATH * sizeof(char));

	fullpath = _strdup(path);
	lstrcat(fullpath, "\\*.*");
	if ((hFind = FindFirstFile(fullpath, &FindFileData)) == INVALID_HANDLE_VALUE)
	{
		writeInLogFile("autoClean => FindFirstFile() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}
	do
	{
		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			&& (strcmp(FindFileData.cFileName, ".") != 0)
			&& (strcmp(FindFileData.cFileName, "..") != 0))
		{
			char	*newpath = goInto(path, FindFileData.cFileName);
			autoClean(newpath);
		}
		else if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)
		{
			if (!deleteFoundFile(path, FindFileData.cFileName))
				return(FALSE);
		}
		else if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		{
			if (isExtensionTMP(FindFileData.cFileName))
				if (!deleteFoundFile(path, FindFileData.cFileName))
					return(FALSE);
		}
	} while (FindNextFile(hFind, &FindFileData) != 0);

	if (!FindClose(hFind))
	{
		writeInLogFile("autoClean => FindClose() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return (FALSE);
	}

	return (TRUE);
}

/*
** This function sets the path to clean in the service registry key.
*/
BOOL		setPathToClean(char *path)
{
	HKEY	hKey;

	if (PathFileExists(path))
	{
		if (RegCreateKeyEx(SERVICE_ROOT_KEY, SERVICE_PATH_TO_CLEAN, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
		{
			writeInLogFile("setPathToClean => RegCreateKey() failed to create \"HKEY_LOCAL_MACHINE", ERROR_SUCCESS);
			writeInLogFile(SERVICE_PATH_TO_CLEAN, ERROR_SUCCESS);
			writeInLogFile("\" with : ", GetLastError());
			writeInLogFile("\"\r\n", ERROR_SUCCESS);
			return (FALSE);
		}
		if (RegSetValueEx(hKey, "path", 0, REG_SZ, path, strlen(path)) != ERROR_SUCCESS)
		{
			writeInLogFile("setPathToClean => RegSetValueEx() failed with : ", GetLastError());
			writeInLogFile("\r\n", ERROR_SUCCESS);
			return (FALSE);
		}
		if (RegCloseKey(hKey) != ERROR_SUCCESS)
		{
			writeInLogFile("setPathToClean => RegCloseKey() failed with : ", GetLastError());
			writeInLogFile("\r\n", ERROR_SUCCESS);
			return (FALSE);
		}
		writeInLogFile("setPathToClean => The key has been successfully created with path : \"", ERROR_SUCCESS);
		writeInLogFile(path, ERROR_SUCCESS);
		writeInLogFile("\"\r\n", ERROR_SUCCESS);
		return (TRUE);
	}
	else
	{
		writeInLogFile("setPathToClean => \"", ERROR_SUCCESS);
		writeInLogFile(path, ERROR_SUCCESS);
		writeInLogFile("\" is not a valid path, PathFileExists() failed with : ", GetLastError());
		writeInLogFile("\r\n", ERROR_SUCCESS);
		return(FALSE);
	}
}

/*
** This function gets the path from the service registry key.
*/
BOOL		getPathToClean(char **path) {
	HKEY	hKey;
	BYTE	buf[255];
	DWORD	dwType;
	DWORD	dwBufSize;

	if (RegOpenKey(SERVICE_ROOT_KEY, SERVICE_PATH_TO_CLEAN, &hKey) == ERROR_SUCCESS)
	{
		dwBufSize = sizeof(buf);
		dwType = REG_SZ;
		if (RegQueryValueEx(hKey, "path", 0, &dwType, buf, &dwBufSize) == ERROR_SUCCESS)
		{
			if (RegCloseKey(hKey) != ERROR_SUCCESS)
			{
				writeInLogFile("getPathToClean => RegCloseKey() failed with : ", GetLastError());
				return (FALSE);
			}
			*path = _strdup(buf);
			printf("");
			return (TRUE);
		}
		else
		{
			writeInLogFile("getPathToClean => RegQueryValueEx() failed with : ", GetLastError());
			return (FALSE);
		}
	}
	else
	{
		writeInLogFile("getPathToClean => RegOpenKey() failed to open \"HKEY_LOCAL_MACHINE", ERROR_SUCCESS);
		writeInLogFile(SERVICE_PATH_TO_CLEAN, ERROR_SUCCESS);
		writeInLogFile("\" with : ", GetLastError());
		writeInLogFile("\"\r\n", ERROR_SUCCESS);
		return(FALSE);
	}
}

/*
** This function is just a cleaner way to build the path to a sub-directory,
** with the addition of "\\".
*/
char		*goInto(char *path, char *filename) {
	char	*newpath = malloc(MAX_PATH * sizeof(char));

	newpath = _strdup(path);
	lstrcat(newpath, "\\");
	lstrcat(newpath, filename);

	return (newpath);
}

/*
** This function tests if a file has a .tmp extension.
*/
BOOL		isExtensionTMP(char *filename)
{
	char	ext[5];

	memcpy(ext, &filename[strlen(filename) - 4], 4);
	ext[4] = 0;
	if (strcmp(ext, ".tmp") == 0 || strcmp(ext, ".TMP") == 0)
		return (TRUE);
	else
		return (FALSE);
}

/*
** This function deletes a file.
*/
BOOL	deleteFoundFile(char *path, char *cFileName)
{
	char	*ficToDele = goInto(path, cFileName);

	if (!DeleteFile(ficToDele))
	{
		writeInLogFile("deleteFoundFile => DeleteFile() failed with : ", GetLastError());
		return (FALSE);
	}
	writeInLogFile("deleteFoundFile => \"", ERROR_SUCCESS);
	writeInLogFile(ficToDele, ERROR_SUCCESS);
	writeInLogFile("\" deleted without problem.\r\n", GetLastError());
	return (TRUE);
}

/*
** This function's aim is to log what happened during the execution.
*/
BOOL		writeInLogFile(char *log, int errorCode)
{
	int		len;
	int		loglen;
	char	err[6];
	char	*newlog;

	loglen = strlen(log);
	newlog = _strdup(log);
	if (errorCode != ERROR_SUCCESS)
	{
		if (_itoa_s(errorCode, err, 6, 10) != ERROR_SUCCESS)
		{
			fprintf(stderr, "writeInLogFile => _itoa_s failed with : %d\n", GetLastError());
			return (FALSE);
		}
		lstrcat(newlog, err);
	}
	loglen = strlen(newlog);

	if (!WriteFile(g_LogFile, newlog, loglen, &len, NULL))
	{
		fprintf(stderr, "writeInLogFile => WriteFile failed with : %d\n", GetLastError());
		return (FALSE);
	}
	return (TRUE);
}

/*
** This function launch the installation of the service.
*/
int launchInstall(BOOL logfileready)
{
	if (!InstallMyService())
	{
		if (logfileready)
		{
			writeInLogFile("Main = > InstallMyService() failed with : ", GetLastError());
			writeInLogFile("\r\n", ERROR_SUCCESS);
		}
		else
			fprintf(stderr, "Main => InstallMyService() failed with : %d\n", GetLastError());
		return(EXIT_FAILURE);
	}
	if (logfileready)
		writeInLogFile("Main => InstallMyService() succeeded\r\n", ERROR_SUCCESS);
	else
		fprintf(stdout, "Main => InstallMyService() succeeded.\n");
	return (EXIT_SUCCESS);
}

/*
** This function launch the treatment to set the path
*/
int	launchSetPath(char *path, BOOL logfileready)
{
	if (!setPathToClean(path))
	{
		if (logfileready)
		{
			writeInLogFile("Main = > setPathToClean() failed with : ", GetLastError());
			writeInLogFile("\r\n", ERROR_SUCCESS);
		}
		else
			fprintf(stderr, "Main => setPathToClean() failed with : %d\n", GetLastError());
		return(EXIT_FAILURE);
	}
	if (logfileready)
		writeInLogFile("Main => setPathToClean() succeeded\r\n", ERROR_SUCCESS);
	else
		fprintf(stdout, "Main => setPathToClean() succeeded.\n");
	return (EXIT_SUCCESS);
}

/*
** This function launch the deletion of the service.
*/
int	launchDelete(BOOL logfileready)
{
	if (!DeleteMyService())
	{
		if (logfileready)
		{
			writeInLogFile("Main = > DeleteMyService() failed with : ", GetLastError());
			writeInLogFile("\r\n", ERROR_SUCCESS);
		}
		else
			fprintf(stderr, "Main => DeleteMyService() failed with : %d\n", GetLastError());
		return(EXIT_FAILURE);
	}
	if (logfileready)
		writeInLogFile("Main => DeleteMyService() succeeded\r\n", ERROR_SUCCESS);
	else
		fprintf(stdout, "Main => DeleteMyService() succeeded.\n");
	return (EXIT_SUCCESS);
}