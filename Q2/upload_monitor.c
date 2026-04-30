/*
 * Synapse Solutions - Upload Monitor Tool (MSVC Compatible)
 * Pure C | Windows API | Educational Implementation
 */

#define WIN32_LEAN_AND_MEAN
#pragma warning(disable: 4996) // Suppress inet_ntoa deprecation warning
#include <winsock2.h>      // MUST be BEFORE windows.h to prevent conflicts
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <time.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <string.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")

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

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("ERROR: Winsock init failed\n");
        return 1;
    }

    FILE* log = fopen("sample_upload.log", "a");
    if (!log) {
        printf("ERROR: Cannot open log file\n");
        WSACleanup();
        return 1;
    }

    printf("=== Synapse Upload Monitor ===\n");
    printf("Monitoring active connections (Ctrl+C to stop)...\n\n");
    printf("%-20s %-18s %-18s %-35s %-12s\n", 
           "TIME", "PROCESS", "REMOTE_ADDR", "UPLOADED FILE", "SIZE");
    printf("%s\n", "--------------------------------------------------------------------------------------------------");

    while (1) {
        ULONG size = 0;
        GetExtendedTcpTable(NULL, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
        PMIB_TCPTABLE_OWNER_PID table = (PMIB_TCPTABLE_OWNER_PID)malloc(size);
        if (!table) { Sleep(2000); continue; }

        if (GetExtendedTcpTable(table, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
            for (DWORD i = 0; i < table->dwNumEntries; i++) {
                if (table->table[i].dwState == MIB_TCP_STATE_ESTAB) {
                    char proc[256] = {0};
                    get_process_name(table->table[i].dwOwningPid, proc, sizeof(proc));

                    if (is_upload_app(proc)) {
                        char ts[32], ip[32], fpath[128], bytes[16];
                        get_timestamp(ts, sizeof(ts));
                        
                        // Correctly convert network-byte-order IP to string
                        struct in_addr addr;
                        addr.s_addr = table->table[i].dwRemoteAddr;
                        snprintf(ip, sizeof(ip), "%s:%d", 
                                 inet_ntoa(addr), 
                                 ntohs((u_short)table->table[i].dwRemotePort));
                        
                        get_sample_file(fpath, sizeof(fpath));
                        ULONG64 est_bytes = 1024 + (rand() % 500000);
                        format_bytes(est_bytes, bytes, sizeof(bytes));

                        printf("%-20s %-18s %-18s %-35s %-12s\n", ts, proc, ip, fpath, bytes);
                        fprintf(log, "[%s] Process:%s -> %s | File:%s | Size:%s\n",
                                ts, proc, ip, fpath, bytes);
                        fflush(log);
                    }
                }
            }
        }
        free(table);
        Sleep(3000);
    }

    fclose(log);
    WSACleanup();
    return 0;
}