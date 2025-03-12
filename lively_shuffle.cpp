#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdlib>
#include <algorithm>
#include <ctime>

namespace fs = std::filesystem;

std::string livelyPath;
std::string wallpaperFolder;

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_RANDOMIZE 1002
#define ID_TRAY_SETTIMER 1003
#define ID_TRAY_SUPPORT 1004

NOTIFYICONDATAA nid = {};
HWND hwnd;
std::atomic<bool> running{ true };
std::atomic<int> shuffleDelay{600};

std::string normalizePath(std::string path) {
    std::replace(path.begin(), path.end(), '/', '\\');
    return fs::weakly_canonical(fs::path(path)).string();
}

void loadPaths() {
    std::ifstream file("lively_paths.txt");
    if (!file) return;

    std::string line;
    while (std::getline(file, line)) {
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) continue;

        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);

        key.erase(0, key.find_first_not_of(" \t\""));
        key.erase(key.find_last_not_of(" \t\"") + 1);

        value.erase(0, value.find_first_not_of(" \t\""));
        value.erase(value.find_last_not_of(" \t\"") + 1);

        value = normalizePath(value);

        if (key == "livelycu_path") livelyPath = value;
        else if (key == "wallpaper_path") wallpaperFolder = value;
    }
}

int readShuffleDelay() {
    std::ifstream file("shuffle_settings.txt");
    if (!file) return 600;

    std::string line;
    while (getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        key.erase(0, key.find_first_not_of(" \t\""));
        key.erase(key.find_last_not_of(" \t\"") + 1);

        value.erase(0, value.find_first_not_of(" \t\""));
        value.erase(value.find_last_not_of(" \t\"") + 1);

        if (key == "time_delay") {
            try {
                int delay = std::stoi(value);
                if (delay > 0) return delay;
            } catch (...) {
                return 600;
            }
        }
    }
    return 600;
}

int getNumDisplays() { return GetSystemMetrics(SM_CMONITORS); }

std::vector<std::string> getWallpapers(const std::string& folder) {
    std::vector<std::string> wallpapers;
    for (const auto& entry : fs::directory_iterator(folder)) {
        std::string ext = entry.path().extension().string();
        if (ext == ".jpg" || ext == ".png" || ext == ".gif" || ext == ".mp4" || ext == ".mov" || ext == ".avi") {
            wallpapers.push_back(entry.path().string());
        }
    }
    return wallpapers;
}

void setWallpaper(int display, const std::string& wallpaper) {
    std::string parameters = "setwp --file \"" + wallpaper + "\" --monitor " + std::to_string(display + 1);

    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(STARTUPINFOA);

    char command[512];
    snprintf(command, sizeof(command), "\"%s\" %s", livelyPath.c_str(), parameters.c_str());

    if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void initializeWallpapers(int numDisplays, const std::vector<std::string>& wallpapers) {
    if (wallpapers.empty()) return;
    for (int i = 0; i < numDisplays; i++)
        setWallpaper(i, wallpapers[rand() % wallpapers.size()]);
}

void wallpaperTimerThread() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(shuffleDelay));
        initializeWallpapers(getNumDisplays(), getWallpapers(wallpaperFolder));
    }
}

void ShowTrayMenu() {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_RANDOMIZE, "Randomize Wallpaper");
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_SETTIMER, "Set Shuffle Timer");
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_SUPPORT, "Support The Developer");
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_EXIT, "Exit");
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

INT_PTR CALLBACK TimerDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char buffer[128];

    switch (uMsg) {
        case WM_INITDIALOG: {
            SetDlgItemInt(hDlg, 1001, shuffleDelay.load(), FALSE);

            RECT rcDlg;
            GetWindowRect(hDlg, &rcDlg);
            int dlgWidth = rcDlg.right - rcDlg.left;
            int dlgHeight = rcDlg.bottom - rcDlg.top;

            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);

            int posX = (screenWidth - dlgWidth) / 2;
            int posY = (screenHeight - dlgHeight) / 2;

            SetWindowPos(hDlg, NULL, posX, posY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

            return TRUE;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemTextA(hDlg, 1001, buffer, sizeof(buffer));
                int newDelay = atoi(buffer);
                if (newDelay > 0) {
                    shuffleDelay = newDelay;
                    std::ofstream file("shuffle_settings.txt");
                    file << "time_delay = \"" << newDelay << "\"";
                    initializeWallpapers(getNumDisplays(), getWallpapers(wallpaperFolder));
                    EndDialog(hDlg, IDOK);
                } else {
                    MessageBoxA(hDlg, "Please enter a valid positive integer.", "Error", MB_OK);
                }
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_TRAYICON && lParam == WM_RBUTTONUP) ShowTrayMenu();
    else if (uMsg == WM_COMMAND) {
        if (LOWORD(wParam) == ID_TRAY_RANDOMIZE) {
            initializeWallpapers(getNumDisplays(), getWallpapers(wallpaperFolder));
        }
        else if (LOWORD(wParam) == ID_TRAY_SETTIMER) {
            DialogBoxParamA(GetModuleHandle(NULL), MAKEINTRESOURCEA(101), hwnd, TimerDialogProc, 0);
        }
        else if (LOWORD(wParam) == ID_TRAY_SUPPORT) {
            ShellExecuteA(hwnd, "open", "https://paypal.me/valekll", NULL, NULL, SW_SHOWNORMAL);
        }
        else if (LOWORD(wParam) == ID_TRAY_EXIT) {
            running = false;
            Shell_NotifyIconA(NIM_DELETE, &nid);
            PostQuitMessage(0);
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    FreeConsole();
    srand((unsigned int)time(0));

    loadPaths();
    shuffleDelay = readShuffleDelay();
    initializeWallpapers(getNumDisplays(), getWallpapers(wallpaperFolder));

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "LivelyShuffleClass";
    RegisterClassA(&wc);

    hwnd = CreateWindowA("LivelyShuffleClass", "", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = (HICON)LoadImageA(NULL, "lively_shuffle.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    strcpy_s(nid.szTip, "Lively Shuffle");
    Shell_NotifyIconA(NIM_ADD, &nid);

    std::thread timerThread(wallpaperTimerThread);
    timerThread.detach();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
