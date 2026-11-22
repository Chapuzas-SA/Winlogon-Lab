#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <cwchar>

namespace {
constexpr int kHotkey = 1;
bool opening = false;

void OpenFile(HWND window)
{
    if (opening) return;
    opening = true;
    HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(result)) {
        IFileOpenDialog* dialog = nullptr;
        IShellItem* folder = nullptr;
        result = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog));
        FILEOPENDIALOGOPTIONS options = {};
        if (SUCCEEDED(result)) {
            result = dialog->GetOptions(&options);
        }
        if (SUCCEEDED(result)) {
            result = dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST |
                                        FOS_PATHMUSTEXIST | FOS_NOCHANGEDIR | FOS_DONTADDTORECENT |
                                        FOS_HIDEPINNEDPLACES);
        }
        if (SUCCEEDED(result)) {
            const COMDLG_FILTERSPEC filters[] = {{L"All files (*.*)", L"*.*"}};
            result = dialog->SetFileTypes(ARRAYSIZE(filters), filters);
        }
        if (SUCCEEDED(result)) {
            result = SHGetKnownFolderItem(FOLDERID_System, KF_FLAG_DEFAULT, nullptr,
                                          IID_IShellItem, reinterpret_cast<void**>(&folder));
        }
        if (SUCCEEDED(result)) {
            result = dialog->SetFolder(folder);
        }
        if (SUCCEEDED(result)) {
            result = dialog->Show(window);
        }
        if (folder) folder->Release();
        if (dialog) dialog->Release();
        CoUninitialize();
    }

    if (FAILED(result) && result != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        wchar_t text[256];
        swprintf(text, ARRAYSIZE(text), L"Could not open the file dialog. Error: 0x%08lX.",
                 static_cast<DWORD>(result));
        MessageBoxW(window, text, L"Recovery System", MB_OK | MB_ICONERROR);
    }
    opening = false;
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_HOTKEY && wparam == kHotkey) {
        OpenFile(window);
        return 0;
    }
    if (message == WM_CLOSE) {
        return 0;
    }
    if (message == WM_DESTROY) {
        UnregisterHotKey(window, kHotkey);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    BOOL inJob = FALSE;
    if (!IsProcessInJob(GetCurrentProcess(), nullptr, &inJob) || !inJob) {
        MessageBoxW(nullptr,
                    L"Enable Recovery System in the Ease of Access Center.",
                    L"Recovery System", MB_OK | MB_ICONINFORMATION);
        return 1;
    }

    WNDCLASSW cls = {};
    cls.lpfnWndProc = WindowProc;
    cls.hInstance = instance;
    cls.lpszClassName = L"EoA_SysRecovery";
    if (!RegisterClassW(&cls)) {
        return 2;
    }
    HWND window = CreateWindowExW(0, cls.lpszClassName, nullptr,
                                  0, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
    if (!window) {
        return 3;
    }

    if (!RegisterHotKey(window, kHotkey, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, VK_F9)) {
        DWORD error = GetLastError();
        wchar_t text[256];
        swprintf(text, ARRAYSIZE(text),
                 L"Could not register Ctrl+Alt+F9. Win32 error: %lu.\n"
                 L"Check whether another instance or application is using this shortcut.", error);
        MessageBoxW(window, text, L"Recovery System", MB_OK | MB_ICONERROR);
        DestroyWindow(window);
        return 4;
    }

    MSG message = {};
    int result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    DestroyWindow(window);
    return result == -1 ? 5 : 0;
}
