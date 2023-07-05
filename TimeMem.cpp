// TimeMem.cpp - Windows port of Unix /usr/bin/time utility

#include <fcntl.h>
#include <io.h>
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
    _ftprintf(stderr, _T("Usage: TimeMem command [args...]\n"));
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
    if ((err == ENOENT)) {
        // Reattempt with cmd.
        ///@todo: Need to account for PowerShell parent process.
        SearchPath(nullptr, _T("cmd.exe"), nullptr, MAX_PATH, target[0].data(), nullptr);
        target[1] = _T("/c ") + auto_string(command) + _T(" ");
    }
    return target;
}

// Make the params pointer relative to full command line without needing to reconstruct arguments.
// GetCommandLine() is typically the source for full command line.
_TCHAR* strip_command(_TCHAR* const full_command) {
    return _tcsstr(full_command, _T(" ")) + 1;
}

// Displays information about a process.
DWORD info(HANDLE hProcess, _TCHAR* const command, _TCHAR* const full_command) {
    // Get error code from CreateProcess
    DWORD dwExitCode = GetLastError();
    PROCESS_MEMORY_COUNTERS pmc = { sizeof(PROCESS_MEMORY_COUNTERS), 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    double tElapsed = 0;
    double tKernel = 0;
    double tUser = 0;
    _ftprintf(stderr, _T("\n"));

    if (hProcess != nullptr) {

        // Exit code
        switch (dwExitCode) {
            ///@todo Generic prints, check GNU time behaviors
            case ENOENT:
                // CreateProcess failed due to file not found
                _ftprintf(stderr, _T("TimeMem: cannot run %s: %s\n"), command, _tcserror(dwExitCode));
                // GNU time command replace file not found with 127
                dwExitCode = 127;
                break;
            case 0:
                // Replace error code if CreateProcess was successful
                GetExitCodeProcess(hProcess, &dwExitCode);
                if (dwExitCode == 0) {
                    break;
                }
            default:
                // Take error code as is
                _ftprintf(stderr, _T("%s: %s\n"), command, _tcserror(dwExitCode));
                break;
        }

        // CPU info
        FILETIME64 ftCreation;
        FILETIME64 ftExit;
        FILETIME64 ftKernel;
        FILETIME64 ftUser;
        if (GetProcessTimes(hProcess, &ftCreation.legacy, &ftExit.legacy, &ftKernel.legacy, &ftUser.legacy)) {
            tElapsed = 1.0e-7 * (ftExit.time - ftCreation.time);
            tKernel = 1.0e-7 * ftKernel.time;
            tUser = 1.0e-7 * ftUser.time;

            // Memory info
            // Print information about the memory usage of the process.
            // More memory information?
            // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa965225(v=vs.85)
            GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
        }
    }

    // Display info.
    if (dwExitCode != 0) {
        _ftprintf(stderr, _T("Command exited with non-zero status %u\n"), dwExitCode);
    }
    _ftprintf(stderr, _T("\tCommand being timed: %s\n"), full_command);

    _ftprintf(stderr, _T("\tUser time (seconds): %.2lf\n"), tUser);
    _ftprintf(stderr, _T("\tSystem time (seconds): %.2lf\n"), tKernel);
    _ftprintf(stderr, _T("\tElapsed (wall clock) time (h:mm:ss or m:ss): %.2lf\n"), tElapsed);

    // KIV temporary calculations which may not be accurate.
    // Minor/Soft page faults, virtual memory swapped out to disk os needs to retrieve page from disk to physical memory
    auto minor_page_fault = pmc.QuotaPeakPagedPoolUsage;

    _ftprintf(stderr, _T("\tMaximum resident set size (kbytes): %zd\n"), pmc.PeakWorkingSetSize/1024);
    _ftprintf(stderr, _T("\tTotal page faults: %u\n"), pmc.PageFaultCount);
    _ftprintf(stderr, _T("\tPaged pool     : %zd KB\n"), pmc.QuotaPeakPagedPoolUsage/1024);
    _ftprintf(stderr, _T("\tNon-paged pool : %zd KB\n"), pmc.QuotaPeakNonPagedPoolUsage/1024);
    _ftprintf(stderr, _T("\tPage file size : %zd KB\n"), pmc.PeakPagefileUsage/1024);

    _ftprintf(stderr, _T("\tExit status: %u\n"), dwExitCode);

    return dwExitCode;
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

#ifdef _UNICODE
    if (!IsDebuggerPresent()) {
        // Need to change terminal to unicode mode
        // NOTE: vscode's debug console does not support unicode mode
        _setmode(_fileno(stdout), _O_U16TEXT);
        _setmode(_fileno(stderr), _O_U16TEXT);
    }
#endif

    // If we have no arguments, display usage info and exit.
    if (argc == 1)
    {
        usage();
        return EPERM;
    }

    // Acquire a normalized target command.
    auto command = where(argv[1]);

    // Get full command line.
    _TCHAR* const full_command = strip_command(GetCommandLine());

    // Get first command argument.
    _TCHAR* const params = strip_command(full_command);

    // Append the arguments.
    command[1] += params;

    // Create the process.
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    if (!CreateProcess(command[0].data(), command[1].data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi))
    {
        // Most likely won't ever get in anymore due to command replacement...
        _ftprintf(stderr, _T("Error: Cannot create process.\n"));
        return info(pi.hProcess, argv[1], full_command);
    }

    // Wait for the process to finish.
    if (WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)
    {
        ///@todo This waits indefinitely, probably only useful for long processes to exit with Ctrl-C (SIGINT)
        _ftprintf(stderr, _T("Error: Cannot wait for process.\n"));
        return info(pi.hProcess, argv[1], full_command);
    }

    // Display process statistics.
    auto const ret = info(pi.hProcess, argv[1], full_command);

    // Close process handles.
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return ret;
}
