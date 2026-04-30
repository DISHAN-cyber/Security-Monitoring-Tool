================================================================
SYNAPSE SOLUTIONS - UPLOAD MONITOR ASSIGNMENT
================================================================
DESCRIPTION:
A pure C Windows-based monitoring tool that detects and logs 
file upload activity from employee workstations. Designed to 
help organizations identify and mitigate unauthorized data 
exfiltration.

FOLDER STRUCTURE:
SynapseProject/
├── Q1\                  (Question 1 files)
├── Q2\                  (Console Application)
│   ├── upload_monitor.c
│   ├── upload_monitor.exe
│   ├── sample_upload.log
│   ├── screenshot.png
│   └── README.txt
└── Q3\                  (Windows Service - Auto-Start)
    ├── upload_monitor_service.c
    ├── upload_monitor_service.exe
    ├── service_upload.log
    ├── README_Q3.txt
    └── [Screenshots: install, start, services, log]

================================================================
COMPILATION INSTRUCTIONS:
Open "Developer Command Prompt for VS 2026" as Administrator,
navigate to the respective folder, and run:

FOR Q2 (Console App):
cl /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt" upload_monitor.c /link /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x86" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x86" iphlpapi.lib ws2_32.lib psapi.lib

FOR Q3 (Windows Service):
cl /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt" upload_monitor_service.c /link /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x86" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x86" iphlpapi.lib ws2_32.lib psapi.lib advapi32.lib

Note: Explicit SDK paths are included to bypass environment variable 
issues. Warnings are normal; ensure 0 `error` lines appear.

================================================================
USAGE INSTRUCTIONS:

Q2 - CONSOLE APPLICATION:
1. Open Command Prompt as Administrator
2. cd to Q2 folder
3. Run: .\upload_monitor.exe
4. Output prints to console + saves to sample_upload.log
5. Press Ctrl + C to stop

Q3 - WINDOWS SERVICE (AUTO-START AT BOOT):
1. Open PowerShell or CMD as Administrator
2. cd to Q3 folder
3. Install: .\upload_monitor_service.exe install
4. Start:   C:\Windows\System32\net.exe start "UploadMonitorService"
5. Verify:  C:\Windows\System32\sc.exe query "UploadMonitorService"
            (Look for STATE: 4 RUNNING)
6. Configure Auto-Start: Built into installation (START_TYPE: AUTO_START)
7. Uninstall: .\upload_monitor_service.exe uninstall

Note: Full paths to net.exe/sc.exe are used to avoid PowerShell 
path resolution issues. Always run as Administrator for service ops.

================================================================
OUTPUT & LOG FORMAT:
[YYYY-MM-DD HH:MM:SS]
Remote IP: <IP>:<Port>
Application: <process.exe>
Detected File: <full_local_path>
Data Transferred: <size in B/KB/MB/GB>

Example:
[2025-12-09 16:43:10]
Remote IP: 165.22.221.132:443
Application: chrome.exe
Detected File: D:\Test\Test.pdf
Data Transferred: 24.50 KB

================================================================
TROUBLESHOOTING:
- "net/services.msc not recognized": Use full paths:
  C:\Windows\System32\net.exe start "ServiceName"
  C:\Windows\System32\services.msc
- "OpenSCManager failed (5)": Access Denied. Open terminal as Administrator.
- "Cannot open include file 'winsock2.h'": Use explicit /I SDK paths in cl command.
- "LINK : fatal error LNK1104": Use explicit /LIBPATH SDK lib paths in cl command.

================================================================
SUBMISSION CHECKLIST:
[ ] Q2 contains: source.c, .exe, README.txt, sample log, screenshot
[ ] Q3 contains: service.c, .exe, log, README, 4 config screenshots
[ ] ZIP includes Q1, Q2, Q3 folders
[ ] Email sent to synapsemvas@gmail.com
[ ] Subject: Assignment Submission - [Your Name]

