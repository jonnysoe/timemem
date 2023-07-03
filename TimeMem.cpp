// TimeMem.cpp - Windows port of Unix /usr/bin/time utility

#include <Windows.h>
#include <Psapi.h>
#include <stdint.h>
#include <stdio.h>
#include <tchar.h>

#include <string>
#include <vector>

// Alternative to ULARGE_INTEGER, this will give a proper FILETIME type
union FILETIME64 {
    FILETIME legacy;
    uint64_t time;
};

// Automatic string for UNICODE and _UNICODE
using auto_string = std::basic_string<_TCHAR>;

namespace {
// Displays usage help for this program.
void usage() {
    _tprintf(_T("Usage: TimeMem command [args...]\n"));
}

// Displays information about a process.
int info(HANDLE hProcess) {
    DWORD dwExitCode;

    // Exit code
    if (!GetExitCodeProcess(hProcess, &dwExitCode)) {
        return 1;
    }

    // CPU info
    FILETIME64 ftCreation;
    FILETIME64 ftExit;
    FILETIME64 ftKernel;
    FILETIME64 ftUser;
    if (!GetProcessTimes(hProcess, &ftCreation.legacy, &ftExit.legacy, &ftKernel.legacy, &ftUser.legacy)) {
        return 1;
    }
    double const tElapsed = 1.0e-7 * (ftExit.time - ftCreation.time);
    double const tKernel = 1.0e-7 * ftKernel.time;
    double const tUser = 1.0e-7 * ftUser.time;

    // Memory info
    // Print information about the memory usage of the process.
    PROCESS_MEMORY_COUNTERS pmc = { sizeof(PROCESS_MEMORY_COUNTERS) };
    if (!GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        return 1;
    }

    // Display info.
    _tprintf(_T("Exit code      : %u\n"), dwExitCode);

    _tprintf(_T("Elapsed time   : %.2lf\n"), tElapsed);
    _tprintf(_T("Kernel time    : %.2lf (%.1lf%%)\n"), tKernel, 100.0*tKernel/tElapsed);
    _tprintf(_T("User time      : %.2lf (%.1lf%%)\n"), tUser, 100.0*tUser/tElapsed);

    _tprintf(_T("page fault #   : %u\n"), pmc.PageFaultCount);
    _tprintf(_T("Working set    : %zd KB\n"), pmc.PeakWorkingSetSize/1024);
    _tprintf(_T("Paged pool     : %zd KB\n"), pmc.QuotaPeakPagedPoolUsage/1024);
    _tprintf(_T("Non-paged pool : %zd KB\n"), pmc.QuotaPeakNonPagedPoolUsage/1024);
    _tprintf(_T("Page file size : %zd KB\n"), pmc.PeakPagefileUsage/1024);

    return 0;
}
}

// Todo:
// - mimic linux time utility interface; e.g. see http://linux.die.net/man/1/time
// - build under 64-bit
// - display detailed error message

int _tmain(int argc, _TCHAR* argv[]) {
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    // If we have no arguments, display usage info and exit.
    if (argc == 1)
    {
        usage();
        return EPERM;
    }

    // Read the command line.
    auto szCmdLine = GetCommandLine();

    // Get first argument, argument 0 is always the current program so skip it.
    auto szBegin = _tcsstr(szCmdLine, argv[1]);
    if (*(szBegin - 1) == _T('\"')) {
        --szBegin;
    }

    // Create the process.
    if (!CreateProcess(NULL, szBegin, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        _tprintf(_T("Error: Cannot create process.\n"));
        return 1;
    }

    // Wait for the process to finish.
    if (WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)
    {
        _tprintf(_T("Error: Cannot wait for process.\n"));
        return 1;
    }

    // Display process statistics.
    int ret = info(pi.hProcess);

    // Close process handles.
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return ret;
}
