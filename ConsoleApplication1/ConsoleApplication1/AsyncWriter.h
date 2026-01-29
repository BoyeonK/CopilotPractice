#pragma once

#include <windows.h>
#include <string>
#include <thread>
#include <iostream>
#include "Actor.h"
#include "CustomClass.h"

// AsyncWriter issues an overlapped WriteFile and posts completion results to an Actor<CustomClass>.
class AsyncWriter {
public:
    explicit AsyncWriter(Actor<CustomClass>& actor) : actor_(actor) {}

    // write data to filename asynchronously. id is placed into the message text.
    void writeFileAsync(const std::string& filename, const std::string& data, const std::string& id) {
        // open file (create/truncate)
        HANDLE fh = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, nullptr);
        if (fh == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateFile failed: " << GetLastError() << "\n";
            // post failure message with -1
            actor_.post(CustomClass(id + " (open failed)", -1));
            return;
        }

        OVERLAPPED* pov = new OVERLAPPED();
        ZeroMemory(pov, sizeof(OVERLAPPED));
        pov->hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (!pov->hEvent) {
            std::cerr << "CreateEvent failed: " << GetLastError() << "\n";
            CloseHandle(fh);
            delete pov;
            actor_.post(CustomClass(id + " (event failed)", -1));
            return;
        }

        DWORD bytesWritten = 0;
        BOOL res = WriteFile(fh, data.c_str(), static_cast<DWORD>(data.size()), &bytesWritten, pov);
        if (!res) {
            DWORD err = GetLastError();
            if (err != ERROR_IO_PENDING) {
                std::cerr << "WriteFile failed: " << err << "\n";
                CloseHandle(pov->hEvent);
                CloseHandle(fh);
                delete pov;
                actor_.post(CustomClass(id + " (write failed)", -1));
                return;
            }
        }

        // spawn a waiter thread to wait for completion and then post to actor
        std::thread([this, fh, pov, id]() mutable {
            DWORD wait = WaitForSingleObject(pov->hEvent, INFINITE);
            if (wait == WAIT_OBJECT_0) {
                DWORD n = 0;
                if (GetOverlappedResult(fh, pov, &n, FALSE)) {
                    actor_.post(CustomClass(id, static_cast<int>(n)));
                } else {
                    actor_.post(CustomClass(id + " (result failed)", -1));
                }
            } else {
                actor_.post(CustomClass(id + " (wait failed)", -1));
            }
            CloseHandle(pov->hEvent);
            CloseHandle(fh);
            delete pov;
        }).detach();
    }

private:
    Actor<CustomClass>& actor_;
};
