// AES_Implementation.cpp : Defines the entry point for the application.
//
#include "framework.h"
#include "AES_LibExample.h"
#include "..\\AES_Library\\AES_Library.h"

#pragma comment (lib, "..\\AES_Library\\Debug\\AES_Library.lib")

// Global Variables:
HINSTANCE hInst;
BYTE* DataBuffer;
BOOL ModifiedFlag;
WCHAR sFname[MAX_PATH];
short AEStype, KeySize, KeyType;
DWORD dwFsize, dwBufsize;
BYTE Key[AES_KEYSIZE256] = { 0 };
BYTE IV[AES_BLOCKSIZE] = { 0 };

// Forward declarations of functions included in this code module:
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);
INT_PTR CALLBACK MainDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL OpenSaveGetFileName(HWND hwnd, LPCWSTR filter, LPCWSTR title, UCHAR selection, LPWSTR filepathname, LPWSTR filename);
DWORD GetSrcBufSize(LPCWSTR pssrcfname);
BOOL ReadFromFile(LPCWSTR pssrcfname, PBYTE psrcbuf, int ibufsize);
BOOL Write2File(LPCWSTR psdestfname, PBYTE pdestbuf, int idestsize);
void UpdateKeyCounter(HWND hDlg);
void GenerateSimpleInitializationVector(BYTE* IV);
void GenerateSimpleKey(BYTE* Key);

/*-------------------------------------------------------------------------------
  GenerateSimpleKey

  Output: Key
-------------------------------------------------------------------------------*/
void GenerateSimpleKey(BYTE* Key)
{
    SYSTEMTIME st;
    int random = rand();
    GetSystemTime(&st);
    st.wYear += st.wMilliseconds;
    st.wMonth += random;
    memcpy(Key, &st, 16);
    if(KeySize != AES_KEYSIZE128)
    {
        random = rand();
        memcpy(Key + 16, &random, 4);
        random = rand();
        memcpy(Key + 20, &random, 4);
        random = rand();
        memcpy(Key + 24, &random, 4);
        random = rand();
        memcpy(Key + 28, &random, 4);
    }
}
/*-------------------------------------------------------------------------------
  GenerateSimpleInitializationVector
  
  Output: Iv
-------------------------------------------------------------------------------*/
void GenerateSimpleInitializationVector(BYTE* IV)
{
    SYSTEMTIME st;

    GetSystemTime(&st);
    memcpy(IV, &st, 16);
}
/*-------------------------------------------------------------------------------
  RunEncrytion
  Input: KeySize, Func, Mode
  Output: Iv
-------------------------------------------------------------------------------*/
void RunEncryption(int KeySize, int Func, short Mode, BYTE* Iv)
{
    switch (Mode)
    {
    case AES_MODECBC:
    case AES_MODEECB:
        AES_ECBmodeCBCmodeEncrypt(Key, KeySize, DataBuffer, dwBufsize, Iv, Mode);
        break;
    case AES_MODECTR:
    case AES_MODEOFB:
        dwBufsize = dwFsize;
        AES_CTRmodeOFBmodeEncryptDecrypt(Key, KeySize, DataBuffer, dwBufsize, Iv, Mode);
        break;
    case AES_MODECFB:
        dwBufsize = dwFsize;
        AES_CFBmodeEncryptDecrypt(Key, KeySize, DataBuffer, dwBufsize, Iv, Func);
        break;
    }
    return;
}
/*-------------------------------------------------------------------------------
  RunDecryption
  Input: KeySize, Func, Mode
  Output: Iv
-------------------------------------------------------------------------------*/
void RunDecryption(int KeySize, int Func, short Mode, BYTE* Iv)
{
    switch (Mode)
    {
    case AES_MODECBC:
    case AES_MODEECB:
        AES_ECBmodeCBCmodeDecrypt(Key, KeySize, DataBuffer, dwBufsize, Iv, Mode);
        break;
    case AES_MODECTR:
    case AES_MODEOFB:
        dwBufsize = dwFsize;
        AES_CTRmodeOFBmodeEncryptDecrypt(Key, KeySize, DataBuffer, dwBufsize, Iv, Mode);
        break;
    case AES_MODECFB:
        dwBufsize = dwFsize;
        AES_CFBmodeEncryptDecrypt(Key, KeySize, DataBuffer, dwBufsize, Iv, Func);
        break;
    }
    return;
}
/*-------------------------------------------------------------------------------
  HexFromNumber
  Input: Input
  Output: Output
-------------------------------------------------------------------------------*/
void HexFromNumber(WCHAR* Output, BYTE* Input, int Size)
{
    WCHAR t[3];
    for (int i = 0; i < Size; i++)
    {
        StringCchPrintf(t, 3, L"%02X", Input[i]);
        Output[i * 2] = t[0];
        Output[i * 2 + 1] = t[1];
    }
    Output[Size * 2] = '\0';
    return;
}
/*-------------------------------------------------------------------------------
  HexFromNumber
  Input: Input, Length
  Output: Key
-------------------------------------------------------------------------------*/
void KeyFromHex(WCHAR* Input, BYTE* Key, int Length)
{
    WCHAR t[2];
    for (int i = 0; i < Length; i += 2)
    {
        t[0] = Input[i];
        t[1] = Input[i + 1];
        UINT tmp = wcstoul(t, NULL, 16);
        UCHAR x = tmp;
        Key[i / 2] = x;
    }
    return;
}
/*-------------------------------------------------------------------------------
  UpdateKeyCounter
  Input: hDlg
  Globals: KeyType, KeySize
-------------------------------------------------------------------------------*/
void UpdateKeyCounter(HWND hDlg)
{
    int length;
    WCHAR stmp[100];
    length = Edit_GetTextLength(GetDlgItem(hDlg, IDC_EDIT1));
    StringCchPrintf(stmp, 10, L"%d / %d", length, KeyType * KeySize);
    Static_SetText(GetDlgItem(hDlg, IDC_COUNTER), stmp);
    return;
}
/*-------------------------------------------------------------------------------
  EnableDisableButtons
  Input: hDlg, Status
-------------------------------------------------------------------------------*/
void EnableDisableButtons(HWND hDlg, BOOL Status)
{
    Button_Enable(GetDlgItem(hDlg, IDC_BUTTONFSAVE), Status);
    Button_Enable(GetDlgItem(hDlg, IDC_BUTTONENCRYPT), Status);
    Button_Enable(GetDlgItem(hDlg, IDC_BUTTONDECRYPT), Status);
    return;
}
/*-------------------------------------------------------------------------------
  SetKeySize
  Input: hDlg, Type, Keysize
  Global: KeySize, KeyType
-------------------------------------------------------------------------------*/
void SetKeySize(HWND hDlg, int Type, short Keysize)
{
    KeySize = Keysize;
    Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT1), KeyType * KeySize);
    Button_SetCheck(GetDlgItem(hDlg, IDC_RADIO128), FALSE);
    Button_SetCheck(GetDlgItem(hDlg, IDC_RADIO192), FALSE);
    Button_SetCheck(GetDlgItem(hDlg, IDC_RADIO256), FALSE);
    Button_SetCheck(GetDlgItem(hDlg, Type), TRUE);
    Static_SetText(GetDlgItem(hDlg, IDC_EDIT1), L"");
    UpdateKeyCounter(hDlg);
    return;
}
/*-------------------------------------------------------------------------------
  SetAEStype
  Input: hDlg, Type
  Global: AEStype
-------------------------------------------------------------------------------*/
void SetAEStype(HWND hDlg, int Type)
{
    // uncheck all then check selected one
    Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOECB), FALSE);
    Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOCBC), FALSE);
    Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOCTR), FALSE);
    Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOOFB), FALSE);
    Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOCFB), FALSE);
    Button_SetCheck(GetDlgItem(hDlg, Type), TRUE);
    if (Type == IDC_RADIOECB)
        Button_Enable(GetDlgItem(hDlg, IDC_EDIT2), FALSE);
    else
        Button_Enable(GetDlgItem(hDlg, IDC_EDIT2), TRUE);
    AEStype = Type - IDC_RADIOECB;
    return;
}
/*-----------------------------------------------------------------------------------------------
    About
    Message handler for About dialog box.
-----------------------------------------------------------------------------------------------*/
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
/*-----------------------------------------------------------------------------------------------
    MainDialog
    Message handler for main dialog box.
-----------------------------------------------------------------------------------------------*/
INT_PTR CALLBACK MainDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    int length;
    WCHAR stmp[MAX_PATH];
    HWND hwndOwner;
    RECT rc, rcDlg, rcOwner;

    switch (message)
    {
    case WM_INITDIALOG:
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_AESIMPLEMENTATION)));
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL)));
        Button_SetCheck(GetDlgItem(hDlg, IDC_RADIO128), TRUE);
        Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOASCII), TRUE);
        Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOECB), TRUE);
        Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT1), 16);
        Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT2), 32);
        EnableDisableButtons(hDlg, FALSE);
        AEStype = AES_MODEECB;
        KeySize = AES_KEYSIZE128;
        KeyType = AES_KEYASCII;
        hwndOwner = GetDesktopWindow();
        GetWindowRect(hwndOwner, &rcOwner);
        GetWindowRect(hDlg, &rcDlg);
        CopyRect(&rc, &rcOwner);
        OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
        OffsetRect(&rc, -rc.left, -rc.top);
        OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);
        SetWindowPos(hDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0, SWP_NOSIZE);
        return (INT_PTR)TRUE;
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_BUTTONFOPEN:
            if (ModifiedFlag == TRUE)
            {
                if(MessageBox(hDlg, L"Current work not yet saved !\n\nSelect OK button to discard unsaved work\and open new file",
                    L"Warning", MB_OKCANCEL) == IDCANCEL)
                break;
            }
            ModifiedFlag = FALSE;
            if (OpenSaveGetFileName(hDlg, L"All files\0*.*\0", L"File Open", 1, sFname, stmp))
            {
                dwFsize = GetSrcBufSize(sFname);
                if (dwFsize <= 0)
                {
                    Static_SetText(GetDlgItem(hDlg, IDC_FNAME), L"");
                    Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"File is Empty, Failed to load !!!");
                    break;
                }
                dwBufsize = dwFsize;
                if (dwFsize % AES_BLOCKSIZE != 0)
                {
                     dwBufsize = dwFsize / AES_BLOCKSIZE;
                     dwBufsize = (dwBufsize + 1) * AES_BLOCKSIZE;
                }
                if(DataBuffer != NULL)
                     realloc(DataBuffer, dwBufsize);
                else
                    DataBuffer = (BYTE*)malloc(dwBufsize);
                ZeroMemory(DataBuffer, dwBufsize);
                if (!ReadFromFile(sFname, DataBuffer, dwBufsize))
                {
                    Static_SetText(GetDlgItem(hDlg, IDC_FNAME), L"");
                    Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"Failed to read File !!!");
                    break;
                }
                Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"File loaded, Ready for Encryption / Decryption.");
                Static_SetText(GetDlgItem(hDlg, IDC_FNAME), sFname);
                EnableDisableButtons(hDlg, TRUE);
            }
            break;
        case IDC_BUTTONFSAVE:
            if (OpenSaveGetFileName(hDlg, L"All files\0*.*\0", L"File Save", 0, sFname, stmp))
            {
                if (dwBufsize <= 0)
                    break;
                if (!Write2File(sFname, DataBuffer, dwBufsize))
                {
                    Static_SetText(GetDlgItem(hDlg, IDC_FNAME), L"");
                    Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"Failed to save File !!!");
                    break;
                }
                Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"File saved !");
                Static_SetText(GetDlgItem(hDlg, IDC_FNAME), sFname);
                EnableDisableButtons(hDlg, FALSE);
            }
            ModifiedFlag = FALSE;
            break;
        case IDC_BUTTONENCRYPT:
            if (AEStype != AES_MODEECB)
            {
                length = (int)Edit_GetTextLength(GetDlgItem(hDlg, IDC_EDIT2));
                if (length != 32)
                {
                    Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"IV is Incomplete !!!");
                    break;
                }
                ZeroMemory(IV, sizeof(IV));
                Edit_GetText(GetDlgItem(hDlg, IDC_EDIT2), stmp, length + 1);
                KeyFromHex(stmp, IV, length);
            }
            length = Edit_GetTextLength(GetDlgItem(hDlg, IDC_EDIT1));
            if (length != KeyType * KeySize)
            {
                Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"Key is Incomplete !!!");
                break;
            }
            ZeroMemory(Key, sizeof(Key));
            Edit_GetText(GetDlgItem(hDlg, IDC_EDIT1), stmp, length +1);
            if (KeyType == AES_KEYHEX)
                KeyFromHex(stmp, Key, length);
            else
                wcstombs_s(NULL, (char*)Key, length + 1, stmp, length + 1);

            RunEncryption(length,ENCRYPT, AEStype, IV);
            Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"Encryption Completed !");
            EnableDisableButtons(hDlg, FALSE);
            Static_SetText(GetDlgItem(hDlg, IDC_FNAME), sFname);
            if (AEStype != AES_MODEECB)
            {
                HexFromNumber(stmp, IV, 16);
                Edit_SetText(GetDlgItem(hDlg, IDC_EDIT2), stmp);
            }
            Button_Enable(GetDlgItem(hDlg, IDC_BUTTONFSAVE), TRUE);
            ModifiedFlag = TRUE;
            break;
        case IDC_BUTTONDECRYPT:
            if (AEStype != AES_MODEECB)
            {
                length = Edit_GetTextLength(GetDlgItem(hDlg, IDC_EDIT2));
                if (length != 32)
                {
                    Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"IV is Incomplete !!!");
                    break;
                }
                ZeroMemory(IV, sizeof(IV));
                Edit_GetText(GetDlgItem(hDlg, IDC_EDIT2), stmp, length + 1);
                KeyFromHex(stmp, IV, length);
            }
            length = Edit_GetTextLength(GetDlgItem(hDlg, IDC_EDIT1));
            if (length != KeyType * KeySize)
            {
                Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"Key is Incomplete !!!");
                break;
            }
            ZeroMemory(Key, sizeof(Key));
            Edit_GetText(GetDlgItem(hDlg, IDC_EDIT1), stmp, length + 1);
            if (KeyType == AES_KEYHEX)
                KeyFromHex(stmp, Key, length);
            else
                wcstombs_s(NULL, (char*)Key, length + 1, stmp, length + 1);
            RunDecryption(length, DECRYPT, AEStype, IV);
            Static_SetText(GetDlgItem(hDlg, IDC_STATUS), L"Decryption Completed !");
            EnableDisableButtons(hDlg, FALSE);
            Static_SetText(GetDlgItem(hDlg, IDC_FNAME), sFname);
            Button_Enable(GetDlgItem(hDlg, IDC_BUTTONFSAVE), TRUE);
            ModifiedFlag = TRUE;
            break;
        case IDC_BUTTONAUTOKEY: // generate auto Key
            SendMessage(hDlg, WM_COMMAND, IDC_RADIOHEX, 0);
            GenerateSimpleKey(Key);
            HexFromNumber(stmp, Key, KeySize);
            Edit_SetText(GetDlgItem(hDlg, IDC_EDIT1), stmp);
            break;
        case IDC_BUTTONAUTOIV: // generate auto iv
            GenerateSimpleInitializationVector(IV);
            HexFromNumber(stmp, IV, 16);
            Edit_SetText(GetDlgItem(hDlg, IDC_EDIT2), stmp);
            break;
        case IDC_EDIT1: //Key entry
            length = Edit_GetTextLength(GetDlgItem(hDlg, IDC_EDIT1));
            if (length > 0)
                UpdateKeyCounter(hDlg);
            break;
        case IDC_EDIT2: //IV entry
            length = Edit_GetTextLength(GetDlgItem(hDlg, IDC_EDIT2));
            if (length > 0)
            {
                StringCchPrintf(stmp, 25, L"%d/%d (in HEX)", length, 32);
                Static_SetText(GetDlgItem(hDlg, IDC_COUNTER2), stmp);
            }
            break;
        case IDC_RADIOASCII: // ASCII Key
            Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOASCII), TRUE);
            Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOHEX), FALSE);
            Static_SetText(GetDlgItem(hDlg, IDC_EDIT1), L"");
            KeyType = AES_KEYASCII;
            UpdateKeyCounter(hDlg);
            break;
        case IDC_RADIOHEX: // Hex Key
            Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOASCII), FALSE);
            Button_SetCheck(GetDlgItem(hDlg, IDC_RADIOHEX), TRUE);
            Static_SetText(GetDlgItem(hDlg, IDC_EDIT1), L"");
            KeyType = AES_KEYHEX;
            UpdateKeyCounter(hDlg);
            break;
        case IDC_RADIOECB: // ECB
        case IDC_RADIOCBC: // CBC
        case IDC_RADIOCTR: // CTR
        case IDC_RADIOOFB: // OFB
        case IDC_RADIOCFB: // CFB
            SetAEStype(hDlg, LOWORD(wParam));
            break;
        case IDC_RADIO128: // 128 bit
            SetKeySize(hDlg, LOWORD(wParam), AES_KEYSIZE128);
            break;
        case IDC_RADIO192: // 192 bit
            SetKeySize(hDlg, LOWORD(wParam), AES_KEYSIZE192);
            break;
        case IDC_RADIO256: // 256 bit
            SetKeySize(hDlg, LOWORD(wParam), AES_KEYSIZE256);
            break;
        case IDM_ABOUT:
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, (DLGPROC)About, 0);
            break;
        case IDOK:
        case IDCANCEL:
            {
                free(DataBuffer);
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
        }
    }
        break;
	case WM_CLOSE:
        EndDialog(hDlg, 0);
        return FALSE;
    }
    return (INT_PTR)FALSE;
}
/*-----------------------------------------------------------------------------------------------
    MAIN FUNCTION
-----------------------------------------------------------------------------------------------*/
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    hInst = hInstance;
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)MainDialog, 0);
}
