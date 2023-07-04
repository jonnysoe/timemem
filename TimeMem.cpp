// TimeMem.cpp - Windows port of Unix /usr/bin/time utility

#include <Windows.h>
#include <Psapi.h>
#include <processenv.h>
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

// A makeshift where command that only returns the first instance of the command
// NOTE: the string objects in the vector are corrupted, do not use size(), empty(), etc.
std::vector<auto_string> where(_TCHAR* command) {
    std::vector<auto_string> target(2);
    target[0].reserve(MAX_PATH);
    target[1].reserve(MAX_PATH);

    // Attempt to search for fully qualified file path.
    SearchPath(nullptr, command, _T(".exe"), MAX_PATH, target[0].data(), nullptr);

    int err = GetLastError();
    if (err == ENOENT) {
        // Reattempt with cmd.
        ///@todo: Need to account for PowerShell parent process.
        SearchPath(nullptr, _T("cmd.exe"), nullptr, MAX_PATH, target[0].data(), nullptr);
        target[1] = _T("/c ") + auto_string(command) + _T(" ");
    }
    return target;
}

// Make the params pointer relative to full command line without needing to reconstruct arguments.
// GetCommandLine() is typically the source for full command line.
_TCHAR* partial_command(_TCHAR* const full_command, const std::vector<_TCHAR*>& args, int const offset) {
    _TCHAR* params = _T("");
    if (args.size() > offset) {
        // Normally argv with spaces does not have quote `"` but full command line does,
        // so -1 was blindly used to account for it.
        params = _tcsstr(full_command, args[offset]) - 1;
        if (*params != _T('\"')) {
            // This argv does not have have space so `"` wasn't inserted, bump the starting pointer.
            ++params;
        }
    }
    return params;
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
    _tprintf(_T("\n"));
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
    // Can add this in the debugger watchpoint.
    // argv[0] is always the current program so skip it.
    std::vector<_TCHAR*> args(argv + 1, argv + argc);

    // If we have no arguments, display usage info and exit.
    if (argc == 1)
    {
        usage();
        return EPERM;
    }

    // Acquire a normalized target command.
    auto command = where(args[0]);

    // Get full command line.
    _TCHAR* const full_command = partial_command(GetCommandLine(), args, 0);

    // Get first command argument.
    _TCHAR* const params = partial_command(full_command, args, 1);

    // Append the arguments.
    command[1] += params;

    // Create the process.
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    if (!CreateProcess(command[0].data(), command[1].data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi))
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
