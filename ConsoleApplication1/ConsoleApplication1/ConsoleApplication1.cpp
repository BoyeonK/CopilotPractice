// Overlapped I/O example (Windows)
#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>
#include <chrono>
#include <thread>
#include "AsyncWriter.h"

int main()
{
    const char* filename = "overlapped_example.txt";

    HANDLE fh = CreateFileA(filename,
                            GENERIC_WRITE,
                            0,
                            nullptr,
                            CREATE_ALWAYS,
                            FILE_FLAG_OVERLAPPED,
                            nullptr);

    if (fh == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateFile failed: " << GetLastError() << "\n";
        return 1;
    }

    const char* msg = "Hello from Overlapped I/O\n";
    DWORD length = static_cast<DWORD>(std::strlen(msg));

    OVERLAPPED ov{};
    ov.Offset = 0;
    ov.OffsetHigh = 0;
    ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!ov.hEvent) {
        std::cerr << "CreateEvent failed: " << GetLastError() << "\n";
        CloseHandle(fh);
        return 1;
    }

    DWORD bytesWritten = 0;
    BOOL writeRes = WriteFile(fh, msg, length, &bytesWritten, &ov);
    if (!writeRes) {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING) {
            // Async pending, wait for completion
            DWORD wait = WaitForSingleObject(ov.hEvent, 5000); // 5s timeout
            if (wait == WAIT_OBJECT_0) {
                if (GetOverlappedResult(fh, &ov, &bytesWritten, FALSE)) {
                    std::cout << "Asynchronously wrote " << bytesWritten << " bytes to '" << filename << "'\n";
                } else {
                    std::cerr << "GetOverlappedResult failed: " << GetLastError() << "\n";
                }
            } else if (wait == WAIT_TIMEOUT) {
                std::cerr << "Wait timed out\n";
            } else {
                std::cerr << "Wait failed: " << GetLastError() << "\n";
            }
        } else {
            std::cerr << "WriteFile failed: " << err << "\n";
        }
    } else {
        // completed synchronously
        std::cout << "Synchronously wrote " << bytesWritten << " bytes to '" << filename << "'\n";
    }

    CloseHandle(ov.hEvent);
    CloseHandle(fh);
    return 0;
}
