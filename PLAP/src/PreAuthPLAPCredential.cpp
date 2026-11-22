#include "PreAuthPLAP.h"
#include <shlobj.h>
#include <shobjidl.h>
#include <cwchar>

PreAuthPLAPCredential::PreAuthPLAPCredential() noexcept { ModuleAddRef(); }
PreAuthPLAPCredential::~PreAuthPLAPCredential() { UnAdvise(); ModuleRelease(); }

static HRESULT OpenFile(HWND window)
{
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
        swprintf(text, ARRAYSIZE(text), L"Could not open the file picker. Error: 0x%08lX.",
                 static_cast<DWORD>(result));
        MessageBoxW(window, text, L"PreAuth PLAP", MB_OK | MB_ICONERROR);
    }
    return result;
}

HRESULT PreAuthPLAPCredential::QueryInterface(REFIID iid, void** result) {
    if (!result) return E_POINTER;
    *result = nullptr;
    if (iid != IID_IUnknown && iid != IID_ICredentialProviderCredential &&
        iid != IID_IConnectableCredentialProviderCredential) return E_NOINTERFACE;
    *result = static_cast<IConnectableCredentialProviderCredential*>(this);
    AddRef();
    return S_OK;
}
ULONG PreAuthPLAPCredential::AddRef() { return ++refs_; }
ULONG PreAuthPLAPCredential::Release() {
    const ULONG refs = --refs_;
    if (!refs) delete this;
    return refs;
}
HRESULT PreAuthPLAPCredential::Advise(ICredentialProviderCredentialEvents* events) {
    if (!events) return E_INVALIDARG;
    events->AddRef();
    AcquireSRWLockExclusive(&eventsLock_);
    auto previous = events_;
    events_ = events;
    ReleaseSRWLockExclusive(&eventsLock_);
    if (previous) previous->Release();
    return S_OK;
}
HRESULT PreAuthPLAPCredential::UnAdvise() {
    AcquireSRWLockExclusive(&eventsLock_);
    auto previous = events_;
    events_ = nullptr;
    ReleaseSRWLockExclusive(&eventsLock_);
    if (previous) { previous->Release(); }
    return S_OK;
}
HRESULT PreAuthPLAPCredential::SetSelected(BOOL* autoLogon) {
    if (!autoLogon) return E_POINTER;
    *autoLogon = FALSE;
    completed_ = false;
    return S_OK;
}
HRESULT PreAuthPLAPCredential::SetDeselected() { completed_ = false; return S_OK; }
HRESULT PreAuthPLAPCredential::GetFieldState(DWORD id, CREDENTIAL_PROVIDER_FIELD_STATE* state, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* interactive) {
    if (!state || !interactive) return E_POINTER;
    *state = CPFS_HIDDEN;
    *interactive = CPFIS_NONE;
    if (id >= FieldCount) return E_INVALIDARG;
    *state = id == FieldTitle ? CPFS_DISPLAY_IN_BOTH : CPFS_DISPLAY_IN_SELECTED_TILE;
    return S_OK;
}
HRESULT PreAuthPLAPCredential::GetStringValue(DWORD id, PWSTR* value) {
    if (!value) return E_POINTER;
    *value = nullptr;
    if (id >= FieldCount) return E_INVALIDARG;
    return CopyString(id == FieldTitle ? L"PreAuth PLAP" : L"Connect", value);
}
HRESULT PreAuthPLAPCredential::GetSubmitButtonValue(DWORD id, DWORD* adjacent) {
    if (!adjacent) return E_POINTER;
    *adjacent = FieldTitle; // Text field; never a bitmap.
    return id == FieldSubmit ? S_OK : E_INVALIDARG;
}
HRESULT PreAuthPLAPCredential::Connect(IQueryContinueWithStatus* query) {
    if (connecting_.exchange(true)) return HRESULT_FROM_WIN32(ERROR_BUSY);
    struct Reset { std::atomic<bool>& flag; ~Reset() { flag = false; } } reset{connecting_};
    completed_ = false;
    if (query) {
        const HRESULT hr = query->QueryContinue();
        if (hr != S_OK) {
            return FAILED(hr) ? hr : HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
    }
    AcquireSRWLockShared(&eventsLock_);
    auto events = events_;
    if (events) events->AddRef();
    ReleaseSRWLockShared(&eventsLock_);
    if (!events) { return E_UNEXPECTED; }
    HWND hwndOwner = nullptr;
    const HRESULT hr = events->OnCreatingWindow(&hwndOwner);
    events->Release();
    if (FAILED(hr) || !hwndOwner || !IsWindow(hwndOwner)) {
        return FAILED(hr) ? hr : HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    }
    /* The owner HWND has been obtained and validated. */
    const HRESULT result = OpenFile(hwndOwner);
    if (FAILED(result)) return result;
    completed_ = true;
    return S_OK;
}
HRESULT PreAuthPLAPCredential::Disconnect() { completed_ = false; return S_OK; }
HRESULT PreAuthPLAPCredential::GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* response,
    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* serialization, PWSTR* text, CREDENTIAL_PROVIDER_STATUS_ICON* icon) {
    if (!response || !serialization || !text || !icon) return E_POINTER;
    *serialization = {};
    *text = nullptr;
    *icon = CPSI_NONE;
    // Ask the host to finish the PLAP interaction without passing anything to LSA.
    *response = completed_ ? CPGSR_RETURN_NO_CREDENTIAL_FINISHED : CPGSR_NO_CREDENTIAL_NOT_FINISHED;
    return S_OK;
}
HRESULT PreAuthPLAPCredential::ReportResult(NTSTATUS, NTSTATUS, PWSTR* text, CREDENTIAL_PROVIDER_STATUS_ICON* icon) {
    if (!text || !icon) return E_POINTER;
    *text = nullptr; *icon = CPSI_NONE;
    return S_OK;
}
// No input fields, bitmap, checkbox, combo box or command links are advertised.
HRESULT PreAuthPLAPCredential::GetBitmapValue(DWORD, HBITMAP* value) { if (!value) return E_POINTER; *value = nullptr; return E_NOTIMPL; }
HRESULT PreAuthPLAPCredential::GetCheckboxValue(DWORD, BOOL* checked, PWSTR* label) {
    if (!checked || !label) return E_POINTER;
    *checked = FALSE; *label = nullptr; return E_NOTIMPL;
}
HRESULT PreAuthPLAPCredential::GetComboBoxValueCount(DWORD, DWORD* count, DWORD* selected) {
    if (!count || !selected) return E_POINTER;
    *count = 0; *selected = 0; return E_NOTIMPL;
}
HRESULT PreAuthPLAPCredential::GetComboBoxValueAt(DWORD, DWORD, PWSTR* value) { if (!value) return E_POINTER; *value = nullptr; return E_NOTIMPL; }
HRESULT PreAuthPLAPCredential::SetStringValue(DWORD, PCWSTR) { return E_NOTIMPL; }
HRESULT PreAuthPLAPCredential::SetCheckboxValue(DWORD, BOOL) { return E_NOTIMPL; }
HRESULT PreAuthPLAPCredential::SetComboBoxSelectedValue(DWORD, DWORD) { return E_NOTIMPL; }
HRESULT PreAuthPLAPCredential::CommandLinkClicked(DWORD) { return E_NOTIMPL; }
