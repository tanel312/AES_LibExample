#include <windows.h>
#include <strsafe.h>
#include <io.h>
#include <fcntl.h>
#include <Commdlg.h>

BOOL OpenSaveGetFileName(HWND hwnd, LPCWSTR filter, LPCWSTR title, UCHAR selection, LPWSTR filepathname, LPWSTR filename)
{
	OPENFILENAME openfilename;

	// Initialize OPENFILENAME
	ZeroMemory(&openfilename, sizeof(openfilename));
	openfilename.lStructSize = sizeof(openfilename);
	openfilename.hwndOwner = hwnd;
	openfilename.lpstrFile = filepathname;
	openfilename.lpstrFile[0] = '\0';
	openfilename.nMaxFile = MAX_PATH;
	openfilename.lpstrFilter = filter;
	openfilename.nFilterIndex = 1;
	openfilename.lpstrFileTitle = NULL;
	openfilename.nMaxFileTitle = 0;
	openfilename.lpstrInitialDir = NULL;
	openfilename.lpstrFileTitle = filename;
	openfilename.nMaxFileTitle = MAX_PATH;
	openfilename.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT;
	openfilename.lpstrTitle = title;
	if (selection == 1)
	{
		if (!GetOpenFileName(&openfilename))
			return FALSE;
	}
	else
	{
		if (!GetSaveFileName(&openfilename))
			return FALSE;
	}
	return TRUE;
}
DWORD GetSrcBufSize(LPCWSTR pssrcfname)
{
	HANDLE hfile;
	LARGE_INTEGER  filesize = { 0 };

	hfile = CreateFile(pssrcfname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	GetFileSizeEx(hfile, &filesize);
	CloseHandle(hfile);
	return filesize.LowPart;
}

BOOL ReadFromFile(LPCWSTR pssrcfname, PBYTE psrcbuf, int ibufsize)
{
	HANDLE hfile;
	DWORD  dwreadsize = { 0 };

	hfile = CreateFile(pssrcfname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	if (FALSE == ReadFile(hfile, psrcbuf, ibufsize, &dwreadsize, NULL))
	{
		CloseHandle(hfile);
		return FALSE;
	}
	CloseHandle(hfile);
	if (dwreadsize == 0)
	{
		return FALSE;
	}
	return TRUE;
}
BOOL Write2File(LPCWSTR psdestfname, PBYTE pdestbuf, int idestsize)
{
	HANDLE hfile;
	DWORD  dwwrotesize = { 0 };

	hfile = CreateFile(psdestfname, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	if (FALSE == WriteFile(hfile, pdestbuf, idestsize, &dwwrotesize, NULL))
	{
		CloseHandle(hfile);
		return FALSE;
	}
	CloseHandle(hfile);
	if (dwwrotesize != idestsize)
		return FALSE;
	return TRUE;
}
