//////////////////////////////////////////////////////////////////////////////
//
//  Test DetourCreateProcessWithDll function (withdll.cpp).
//
//  Microsoft Research Detours Package, Version 3.0.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Shellapi.h>
#include "..\..\detours\detours.h"



WCHAR *pszExe = L"UHDPlayer.exe";
WCHAR *pszDllPath = L"HDMLClonerLoaderData.dll";



//////////////////////////////////////////////////////////////////////////////
//
void PrintUsage(void)
{
	wchar_t buf[1024];

    swprintf(buf, L"Usage:\n"
           L"    withdll.exe [options] [command line]\n"
           L"Options:\n"
           L"    /d:file.dll   : Start the process with file.dll.\n"
           //L"    /v            : Verbose, display memory at start.\n"
           L"    /?            : This help screen.\n");

	MessageBox(0, buf, L"Launcher", MB_OK);
}

//////////////////////////////////////////////////////////////////////////////
//
//  This code verifies that the named DLL has been configured correctly
//  to be imported into the target process.  DLLs must export a function with
//  ordinal #1 so that the import table touch-up magic works.
//
struct ExportContext
{
    BOOL    fHasOrdinal1;
    ULONG   nExports;
};

static BOOL CALLBACK ExportCallback(PVOID pContext,
                                    ULONG nOrdinal,
                                    PCHAR pszSymbol,
                                    PVOID pbTarget)
{
    (void)pContext;
    (void)pbTarget;
    (void)pszSymbol;

    ExportContext *pec = (ExportContext *)pContext;

    if (nOrdinal == 1) {
        pec->fHasOrdinal1 = TRUE;
    }
    pec->nExports++;

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//

//////////////////////////////////////////////////////////////////////////////
//

void TypeToString(DWORD Type, wchar_t *pszBuffer, size_t cBuffer)
{
    if (Type == MEM_IMAGE) {
        swprintf_s(pszBuffer, cBuffer, L"img");
    }
    else if (Type == MEM_MAPPED) {
        swprintf_s(pszBuffer, cBuffer, L"map");
    }
    else if (Type == MEM_PRIVATE) {
        swprintf_s(pszBuffer, cBuffer, L"pri");
    }
    else {
        swprintf_s(pszBuffer, cBuffer, L"%x", Type);
    }
}

void StateToString(DWORD State, wchar_t *pszBuffer, size_t cBuffer)
{
    if (State == MEM_COMMIT) {
        swprintf_s(pszBuffer, cBuffer, L"com");
    }
    else if (State == MEM_FREE) {
        swprintf_s(pszBuffer, cBuffer, L"fre");
    }
    else if (State == MEM_RESERVE) {
        swprintf_s(pszBuffer, cBuffer, L"res");
    }
    else {
        swprintf_s(pszBuffer, cBuffer, L"%x", State);
    }
}

void ProtectToString(DWORD Protect, wchar_t *pszBuffer, size_t cBuffer)
{
    if (Protect == 0) {
        swprintf_s(pszBuffer, cBuffer, L"");
    }
    else if (Protect == PAGE_EXECUTE) {
        swprintf_s(pszBuffer, cBuffer, L"--x");
    }
    else if (Protect == PAGE_EXECUTE_READ) {
        swprintf_s(pszBuffer, cBuffer, L"r-x");
    }
    else if (Protect == PAGE_EXECUTE_READWRITE) {
        swprintf_s(pszBuffer, cBuffer, L"rwx");
    }
    else if (Protect == PAGE_EXECUTE_WRITECOPY) {
        swprintf_s(pszBuffer, cBuffer, L"rcx");
    }
    else if (Protect == PAGE_NOACCESS) {
        swprintf_s(pszBuffer, cBuffer, L"---");
    }
    else if (Protect == PAGE_READONLY) {
        swprintf_s(pszBuffer, cBuffer, L"r--");
    }
    else if (Protect == PAGE_READWRITE) {
        swprintf_s(pszBuffer, cBuffer, L"rw-");
    }
    else if (Protect == PAGE_WRITECOPY) {
        swprintf_s(pszBuffer, cBuffer, L"rc-");
    }
    else if (Protect == (PAGE_GUARD | PAGE_EXECUTE)) {
        swprintf_s(pszBuffer, cBuffer, L"g--x");
    }
    else if (Protect == (PAGE_GUARD | PAGE_EXECUTE_READ)) {
        swprintf_s(pszBuffer, cBuffer, L"gr-x");
    }
    else if (Protect == (PAGE_GUARD | PAGE_EXECUTE_READWRITE)) {
        swprintf_s(pszBuffer, cBuffer, L"grwx");
    }
    else if (Protect == (PAGE_GUARD | PAGE_EXECUTE_WRITECOPY)) {
        swprintf_s(pszBuffer, cBuffer, L"grcx");
    }
    else if (Protect == (PAGE_GUARD | PAGE_NOACCESS)) {
        swprintf_s(pszBuffer, cBuffer, L"g---");
    }
    else if (Protect == (PAGE_GUARD | PAGE_READONLY)) {
        swprintf_s(pszBuffer, cBuffer, L"gr--");
    }
    else if (Protect == (PAGE_GUARD | PAGE_READWRITE)) {
        swprintf_s(pszBuffer, cBuffer, L"grw-");
    }
    else if (Protect == (PAGE_GUARD | PAGE_WRITECOPY)) {
        swprintf_s(pszBuffer, cBuffer, L"grc-");
    }
    else {
        swprintf_s(pszBuffer, cBuffer, L"%x", Protect);
    }
}

static BYTE buffer[65536];

typedef union
{
    struct
    {
        DWORD Signature;
        IMAGE_FILE_HEADER FileHeader;
    } ih;

    IMAGE_NT_HEADERS32 ih32;
    IMAGE_NT_HEADERS64 ih64;
} IMAGE_NT_HEADER;

struct SECTIONS
{
    PBYTE   pbBeg;
    PBYTE   pbEnd;
    CHAR    szName[16];
} Sections[256];
DWORD SectionCount = 0;
DWORD Bitness = 0;

PCHAR FindSectionName(PBYTE pbBase, PBYTE& pbEnd)
{
    for (DWORD n = 0; n < SectionCount; n++) {
        if (Sections[n].pbBeg == pbBase) {
            pbEnd = Sections[n].pbEnd;
            return Sections[n].szName;
        }
    }
    pbEnd = NULL;
    return NULL;
}

ULONG PadToPage(ULONG Size)
{
    return (Size & 0xfff)
        ? Size + 0x1000 - (Size & 0xfff)
        : Size;
}

BOOL GetSections(HANDLE hp, PBYTE pbBase)
{
    DWORD beg = 0;
    DWORD cnt = 0;
    SIZE_T done;
    IMAGE_DOS_HEADER idh;
	wchar_t buf[1024];

    if (!ReadProcessMemory(hp, pbBase, &idh, sizeof(idh), &done) || done != sizeof(idh)) {
        return FALSE;
    }

    if (idh.e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }

    IMAGE_NT_HEADER inh;
    if (!ReadProcessMemory(hp, pbBase + idh.e_lfanew, &inh, sizeof(inh), &done) || done != sizeof(inh)) {
        swprintf(buf, L"No Read\n");
		MessageBox(0, buf, L"Launcher", MB_OK);
        return FALSE;
    }

    if (inh.ih.Signature != IMAGE_NT_SIGNATURE) {
        swprintf(buf, L"No NT\n");
		MessageBox(0, buf, L"Launcher", MB_OK);
        return FALSE;
    }

    beg = idh.e_lfanew
        + FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader )
        + inh.ih.FileHeader.SizeOfOptionalHeader;
    cnt = inh.ih.FileHeader.NumberOfSections;
    Bitness = (inh.ih32.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) ? 32 : 64;
#if 0
    printf("%d %d count=%d\n", beg, Bitness, cnt);
#endif

    IMAGE_SECTION_HEADER ish;
    for (DWORD n = 0; n < cnt; n++) {
        if (!ReadProcessMemory(hp, pbBase + beg + n * sizeof(ish), &ish, sizeof(ish), &done) || done != sizeof(ish)) {
            swprintf(buf, L"No Read\n");
			MessageBox(0, buf, L"Launcher", MB_OK);
            return FALSE;
        }
        Sections[n].pbBeg = pbBase + ish.VirtualAddress;
        Sections[n].pbEnd = pbBase + ish.VirtualAddress + PadToPage(ish.Misc.VirtualSize);
        memcpy(Sections[n].szName, ish.Name, sizeof(ish.Name));
        Sections[n].szName[sizeof(ish.Name)] = '\0';
#if 0
        printf("--- %p %s\n", Sections[n].pbBeg, Sections[n].szName);
#endif
    }
    SectionCount = cnt;

    return TRUE;
}

/*BOOL DumpProcess(HANDLE hp)
{
    ULONG64 base;
    ULONG64 next;

    MEMORY_BASIC_INFORMATION mbi;

    printf("  %12s %8s %8s: %3s %3s %4s %3s : %8s\n", "Address", "Offset", "Size", "Typ", "Sta", "Prot", "Ini", "Contents");
    printf("  %12s %8s %8s: %3s %3s %4s %3s : %8s\n", "------------", "--------", "--------", "---", "---", "----", "---", "-----------------");

    for (next = 0;;) {
        base = next;
        ZeroMemory(&mbi, sizeof(mbi));
        if (VirtualQueryEx(hp, (PVOID)base, &mbi, sizeof(mbi)) == 0) {
            break;
        }
        if ((mbi.RegionSize & 0xfff) == 0xfff) {
            break;
        }

        next = (ULONG64)mbi.BaseAddress + mbi.RegionSize;

        if (mbi.State == MEM_FREE) {
            continue;
        }

        CHAR szType[16];
        TypeToString(mbi.Type, szType, ARRAYSIZE(szType));
        CHAR szState[16];
        StateToString(mbi.State, szState, ARRAYSIZE(szState));
        CHAR szProtect[16];
        ProtectToString(mbi.Protect, szProtect, ARRAYSIZE(szProtect));
        CHAR szAllocProtect[16];
        ProtectToString(mbi.AllocationProtect, szAllocProtect, ARRAYSIZE(szAllocProtect));

        CHAR szFile[MAX_PATH];
        szFile[0] = '\0';
        DWORD cb = 0;
        PCHAR pszFile = szFile;

        if (base == (ULONG64)mbi.AllocationBase) {
#if 0
            cb = pfGetMappedFileName(hp, (PVOID)mbi.AllocationBase, szFile, ARRAYSIZE(szFile));
#endif
            if (GetSections(hp, (PBYTE)mbi.AllocationBase)) {
                next = base + 0x1000;
                sprintf_s(szFile, ARRAYSIZE(szFile), "%d-bit PE", Bitness);
            }
        }
        if (cb > 0) {
            for (DWORD c = 0; c < cb; c++) {
                szFile[c] = (szFile[c] >= 'a' && szFile[c] <= 'z')
                    ? szFile[c] - 'a' + 'A' : szFile[c];
            }
            szFile[cb] = '\0';
        }

        if ((pszFile = strrchr(szFile, '\\')) == NULL) {
            pszFile = szFile;
        }
        else {
            pszFile++;
        }

        PBYTE pbEnd;
        PCHAR pszSect = FindSectionName((PBYTE)base, pbEnd);
        if (pszSect != NULL) {
            pszFile = pszSect;
            if (next > (ULONG64)pbEnd) {
                next = (ULONG64)pbEnd;
            }
        }

        CHAR szDesc[128];
        ZeroMemory(&szDesc, ARRAYSIZE(szDesc));
        if (base == (ULONG64)mbi.AllocationBase) {
            sprintf_s(szDesc, ARRAYSIZE(szDesc), "  %12I64x %8I64x %8I64x: %3s %3s %4s %3s : %s",
                      (ULONG64)base,
                      (ULONG64)base - (ULONG64)mbi.AllocationBase,
                      (ULONG64)next - (ULONG64)base,
                      szType,
                      szState,
                      szProtect,
                      szAllocProtect,
                      pszFile);


        }
        else {
            sprintf_s(szDesc, ARRAYSIZE(szDesc), "  %12s %8I64x %8I64x: %3s %3s %4s %3s : %s",
                      "-",
                      (ULONG64)base - (ULONG64)mbi.AllocationBase,
                      (ULONG64)next - (ULONG64)base,
                      szType,
                      szState,
                      szProtect,
                      szAllocProtect,
                      pszFile);
        }
        printf("%s\n", szDesc);
    }
    return TRUE;
}*/




//////////////////////////////////////////////////////////////////////// main.
//
int CDECL main(int argc, wchar_t **argv)
{
    BOOLEAN fNeedHelp = FALSE;
    BOOLEAN fVerbose = FALSE;

	wchar_t buf[1024];

    int arg = 1;
    for (; arg < argc && (argv[arg][0] == L'-' || argv[arg][0] == L'/'); arg++) {

        WCHAR *argn = argv[arg] + 1;
        WCHAR *argp = argn;
        while (*argp && *argp != L':' && *argp != L'=')
            argp++;
        if (*argp == L':' || *argp == L'=')
            *argp++ = L'\0';

        switch (argn[0]) {
          case L'd':                                     // Set DLL Name
          case L'D':
            pszDllPath = argp;
            break;

          case L'v':                                     // Verbose
          case L'V':
            fVerbose = TRUE;
            break;

          case L'?':                                     // Help
            fNeedHelp = TRUE;
            break;

          default:
            fNeedHelp = TRUE;
            swprintf(buf, L"withdll.exe: Bad argument: %s\n", argv[arg]);
			MessageBox(0, buf, L"Launcher", MB_OK);
            break;
        }
    }

    if (arg >= argc) {
        //fNeedHelp = TRUE;
    }
	else
	{
		pszExe = argv[arg];
	}

    if (pszDllPath == NULL) {
        //fNeedHelp = TRUE;
    }

    if (fNeedHelp) {
        PrintUsage();
        return 9001;
    }

    /////////////////////////////////////////////////////////// Validate DLLs.
    //
    WCHAR szDllPath[1024];
    PWCHAR pszFilePart = NULL;

    if (!GetFullPathName(pszDllPath, ARRAYSIZE(szDllPath), szDllPath, &pszFilePart)) {
        swprintf(buf, L"withdll.exe: Error: %s is not a valid path name..\n",
               pszDllPath);
		MessageBox(0, buf, L"Launcher", MB_OK);
        return 9002;
    }

    HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (hDll == NULL) {
        swprintf(buf, L"withdll.exe: Error: %s failed to load (error %d).\n",
               pszDllPath,
               GetLastError());
		MessageBox(0, buf, L"Launcher", MB_OK);
        return 9003;
    }

    ExportContext ec;
    ec.fHasOrdinal1 = FALSE;
    ec.nExports = 0;
    DetourEnumerateExports(hDll, &ec, ExportCallback);
    FreeLibrary(hDll);

    if (!ec.fHasOrdinal1) {
        swprintf(buf, L"withdll.exe: Error: %s does not export ordinal #1.\nSee help entry DetourCreateProcessWithDllEx in Detours.chm.\n",
               pszDllPath);
		MessageBox(0, buf, L"Launcher", MB_OK);
        return 9004;
    }

    //////////////////////////////////////////////////////////////////////////
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    WCHAR szCommand[2048];
	WCHAR szExe[1024];
    WCHAR szFullExe[1024];
    PWCHAR pszFileExe = NULL;

	szFullExe[0] = L'\0';

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    szCommand[0] = L'\0';

#ifdef _CRT_INSECURE_DEPRECATE
    wcscpy_s(szExe, 1024, pszExe);
#else
    wcscpy(szExe, pszExe);
#endif
    for (; arg < argc; arg++) {
        if (wcschr(argv[arg], L' ') != NULL || wcschr(argv[arg], L'\t') != NULL) {
#ifdef _CRT_INSECURE_DEPRECATE
            wcscat_s(szCommand, 2048, L"\"");
            wcscat_s(szCommand, 2048, argv[arg]);
            wcscat_s(szCommand, 2048, L"\"");
#else
            strcat(szCommand, "\"");
            strcat(szCommand, argv[arg]);
            strcat(szCommand, "\"");
#endif
        }
        else {
#ifdef _CRT_INSECURE_DEPRECATE
            wcscat_s(szCommand, 2048, argv[arg]);
#else
            strcat(szCommand, argv[arg]);
#endif
        }

        if (arg + 1 < argc) {
#ifdef _CRT_INSECURE_DEPRECATE
            wcscat_s(szCommand, 2048, L" ");
#else
            strcat(szCommand, " ");
#endif
        }
    }
    //printf("withdll.exe: Starting: `%s'\n", szCommand);
    //printf("withdll.exe:   with `%s'\n", szDllPath);
    //fflush(stdout);

    DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

    SetLastError(0);
    if (!SearchPath(NULL, szExe, L".exe", ARRAYSIZE(szFullExe), szFullExe, &pszFileExe))
	{
		swprintf(buf, L"Cannot find program: %s", szExe);
		MessageBox(0, buf, L"Launcher", MB_OK);
		return -1;
	}

	// convert szDllPath from wchar_t* to char*
	size_t origsize = wcslen(szDllPath) + 1;
    size_t convertedChars = 0;
    char nstring[1024];
    wcstombs_s(&convertedChars, nstring, origsize, szDllPath, _TRUNCATE);

    if (!DetourCreateProcessWithDllEx(szFullExe[0] ? szFullExe : NULL, szCommand,
                                      NULL, NULL, TRUE, dwFlags, NULL, NULL,
                                      &si, &pi, nstring, NULL)) {
        DWORD dwError = GetLastError();
        swprintf(buf, L"Launcher: DetourCreateProcessWithDllEx failed: %d when starting:\n%s %s\nwith dll: %s", dwError, szFullExe, szCommand, nstring);
		MessageBox(0, buf, L"Launcher", MB_OK);
        if (dwError == ERROR_INVALID_HANDLE) {
#if DETOURS_64BIT
            printf("withdll.exe: Can't detour a 32-bit target process from a 64-bit parent process.\n");
#else
            swprintf(buf, L"withdll.exe: Can't detour a 64-bit target process from a 32-bit parent process.\n");
			MessageBox(0, buf, L"Launcher", MB_OK);
#endif
        }
        ExitProcess(9009);
    }

    if (fVerbose) {
        //DumpProcess(pi.hProcess);
    }

    ResumeThread(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD dwResult = 0;
    if (!GetExitCodeProcess(pi.hProcess, &dwResult)) {
        swprintf(buf, L"withdll.exe: GetExitCodeProcess failed: %d\n", GetLastError());
		MessageBox(0, buf, L"Launcher", MB_OK);
        return 9010;
    }

    return dwResult;
}
//
///////////////////////////////////////////////////////////////// End of File.

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int argc;
	wchar_t **argv = (wchar_t**)CommandLineToArgvW(GetCommandLine(), &argc);
	main(argc, argv);
	return 0;
}