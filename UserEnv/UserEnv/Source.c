#include <Windows.h>
#include <wchar.h>

BOOL SetPrivilege(
	HANDLE hToken,
	LPCWSTR privilege,
	BOOL enablePrivilege)
{
	TOKEN_PRIVILEGES tokenPrivilege;
	LUID luid;

	if (!LookupPrivilegeValueW(
		NULL,
		privilege,
		&luid))
	{
		fwprintf(stderr, L"Error getting LUID: %u\n", GetLastError());
		return FALSE;
	}

	tokenPrivilege.PrivilegeCount = 1;
	tokenPrivilege.Privileges[0].Luid = luid;

	if (enablePrivilege)
	{
		tokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tokenPrivilege.Privileges[0].Attributes = 0;	// Disabled
	}

	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tokenPrivilege,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		fwprintf(stderr, L"Error enabling or disabling privilege: %u\n", GetLastError());
		return FALSE;
	}

	return TRUE;


}



int wmain(int argc, WCHAR argv[])
{
	HANDLE processHandle = GetCurrentProcess();
	HANDLE tokenHandle;

	if (!OpenProcessToken(
		processHandle,
		TOKEN_ADJUST_PRIVILEGES,
		&tokenHandle))
	{
		fwprintf(stderr, L"Error getting Access Token: %u\n", GetLastError());
		return FALSE;
	}

	if (!SetPrivilege(
		tokenHandle,
		SE_RESTORE_NAME,
		TRUE))
	{
		fwprintf(stderr, L"Error setting SE_RESTORE_NAME privilege: %u\n", GetLastError());
		CloseHandle(tokenHandle);
		return FALSE;
	}

	if (!SetPrivilege(
		tokenHandle,
		SE_BACKUP_NAME,
		TRUE))
	{
		fwprintf(stderr, L"Error setting SE_RESTORE_NAME privilege: %u\n", GetLastError());
		CloseHandle(tokenHandle);
		return FALSE;
	}
	

	// RegLoadKey
	LONG loadKey;
	LPCWSTR subKey = L"BCP";
	LPCWSTR hive = L"C:\\Users\\Default\\NTUSER.dat";

	loadKey = RegLoadKeyW(HKEY_LOCAL_MACHINE, subKey, hive);

	if (loadKey != ERROR_SUCCESS)
	{
		fwprintf(stderr, L"Error loading hive: %u\n", loadKey);
		/*CloseHandle(tokenHandle);*/
		return 1;
	}
	else
	{
		wprintf(L"Default user registry hive has been loaded.\n");

		// RegOpenKeyEx
		LONG openKey;
		LPCWSTR envSubKey = L"BCP\\Environment";
		REGSAM samDesired = KEY_SET_VALUE;

		HKEY hkResult;

		openKey=RegOpenKeyExW(
			HKEY_LOCAL_MACHINE, 
			envSubKey, 
			0, 
			samDesired, 
			&hkResult);

		if (openKey != ERROR_SUCCESS)
		{
			fwprintf(stderr, L"Error opening key: %u\n", openKey);
			/*CloseHandle(tokenHandle);*/
			return 1;
		}

		// RegSetValueEx
		LONG setTempValue, setTmpValue;
		LPCWSTR tempValue = L"TEMP";
		LPCWSTR tmpValue = L"TMP";
		DWORD type = REG_EXPAND_SZ;
		WCHAR tempData[] = L"%SystemRoot%\\TEMP";
		WCHAR tmpData[] = L"%SystemRoot%\\TEMP";
		DWORD tempSize = sizeof(tempData) + 1;

		setTempValue = RegSetValueExW(
			hkResult,
			tempValue,
			0,
			type,
			(const BYTE*)tempData,
			tempSize);

		if (setTempValue != ERROR_SUCCESS)
		{
			fwprintf(stderr, L"Error creating TEMP value: %u\n",setTempValue);
			/*CloseHandle(tokenHandle);*/
			RegCloseKey(hkResult);
			return 1;

		}
		
		wprintf(L"TEMP value has been set.\n");

		setTmpValue = RegSetValueExW(
			hkResult,
			tmpValue,
			0,
			type,
			(const BYTE*)tmpData,
			tempSize);

		if (setTmpValue != ERROR_SUCCESS)
		{
			fwprintf(stderr, L"Error creating TMP value: %u\n", setTmpValue);
			/*CloseHandle(tokenHandle);*/
			RegCloseKey(hkResult);
			return 1;
		}

		wprintf(L"TMP value has been set.\n");

		RegCloseKey(hkResult);

		LONG unloadKey;

		unloadKey = RegUnLoadKeyW(
			HKEY_LOCAL_MACHINE,
			L"BCP");

		if (unloadKey != ERROR_SUCCESS)
		{
			fwprintf(stderr, L"Error unloading hive: %lu\n", unloadKey);
		}

		wprintf(L"Registry hive has been unloaded.\n");		

	}


	CloseHandle(tokenHandle);
	return 0;
}