#include "Provider.h"
#include "Resource.h"

#include <cstring>
#include <cwchar>
#include <new>
#include <shlguid.h>
#include <shlobj.h>

namespace {
const GUID kIidCredential3 = {0x64a5010e, 0x4363, 0x41f8, {0x97, 0x38, 0x19, 0x04, 0x5c, 0x20, 0xda, 0xbc}};
enum FieldId : DWORD { ACTION, LOGO, LABEL };

struct Field {
    CREDENTIAL_PROVIDER_FIELD_TYPE type;
    const wchar_t* label;
    GUID guid;
    CREDENTIAL_PROVIDER_FIELD_STATE state;
};

const Field kFields[] = {
    {CPFT_COMMAND_LINK, L"Pre-Auth", {}, CPFS_DISPLAY_IN_SELECTED_TILE},
    {CPFT_TILE_IMAGE, L"Logo", CPFG_CREDENTIAL_PROVIDER_LOGO, CPFS_DISPLAY_IN_BOTH},
    {CPFT_SMALL_TEXT, L"Pre-Auth", CPFG_CREDENTIAL_PROVIDER_LABEL, CPFS_HIDDEN}
};
constexpr DWORD kFieldCount = sizeof(kFields) / sizeof(kFields[0]);
bool opening = false;

// Strings returned through COM are owned by the caller.
HRESULT CopyString(const wchar_t* source, LPWSTR* destination) {
    const size_t bytes = (std::wcslen(source) + 1) * sizeof(wchar_t);
    *destination = static_cast<LPWSTR>(CoTaskMemAlloc(bytes));
    if (!*destination) return E_OUTOFMEMORY;
    std::memcpy(*destination, source, bytes);
    return S_OK;
}
}

CMinimoProvider::~CMinimoProvider() {
    ClearCredentials();
}

void CMinimoProvider::ClearCredentials() {
    for (DWORD i = 0; i < _credentialCount; ++i) _credentials[i]->Release();
    delete[] _credentials;
    _credentials = nullptr;
    _credentialCount = 0;
}

HRESULT CMinimoProvider::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;
    if (riid == IID_IUnknown || riid == IID_ICredentialProvider)
        *ppv = static_cast<ICredentialProvider*>(this);
    else if (riid == IID_ICredentialProviderSetUserArray)
        *ppv = static_cast<ICredentialProviderSetUserArray*>(this);
    else
        return E_NOINTERFACE;
    AddRef();
    return S_OK;
}

ULONG CMinimoProvider::AddRef() { return InterlockedIncrement(&_cRef); }
ULONG CMinimoProvider::Release() {
    const ULONG refs = InterlockedDecrement(&_cRef);
    if (!refs) delete this;
    return refs;
}

HRESULT CMinimoProvider::SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO scenario, DWORD) {
    ClearCredentials();
    return scenario == CPUS_LOGON || scenario == CPUS_UNLOCK_WORKSTATION || scenario == CPUS_CREDUI
        ? S_OK : E_NOTIMPL;
}

HRESULT CMinimoProvider::SetUserArray(ICredentialProviderUserArray* users) {
    ClearCredentials();
    if (!users) return E_INVALIDARG;
    DWORD count = 0;
    HRESULT hr = users->GetCount(&count);
    if (FAILED(hr) || !count) return hr;
    _credentials = new (std::nothrow) CMinimoCredential*[count]();
    if (!_credentials) return E_OUTOFMEMORY;

    for (DWORD i = 0; i < count; ++i) {
        ICredentialProviderUser* user = nullptr;
        LPWSTR sid = nullptr;
        hr = users->GetAt(i, &user);
        if (SUCCEEDED(hr)) hr = user ? user->GetSid(&sid) : E_UNEXPECTED;
        if (user) user->Release();
        if (SUCCEEDED(hr) && (!sid || !*sid)) hr = E_UNEXPECTED;
        if (SUCCEEDED(hr)) {
            _credentials[i] = new (std::nothrow) CMinimoCredential(sid);
            if (!_credentials[i]) hr = E_OUTOFMEMORY;
        }
        if (FAILED(hr)) {
            CoTaskMemFree(sid);
            ClearCredentials();
            return hr;
        }
        ++_credentialCount;
    }
    return S_OK;
}

HRESULT CMinimoProvider::GetCredentialCount(DWORD* count, DWORD* defaultIndex, BOOL* autoLogon) {
    if (!count || !defaultIndex || !autoLogon) return E_POINTER;
    *count = _credentialCount;
    *defaultIndex = static_cast<DWORD>(-1); // CREDENTIAL_PROVIDER_NO_DEFAULT
    *autoLogon = FALSE;
    return S_OK;
}

HRESULT CMinimoProvider::GetCredentialAt(DWORD index, ICredentialProviderCredential** credential) {
    if (!credential) return E_POINTER;
    *credential = nullptr;
    if (index >= _credentialCount) return E_INVALIDARG;
    return _credentials[index]->QueryInterface(IID_ICredentialProviderCredential,
                                       reinterpret_cast<void**>(credential));
}

HRESULT CMinimoProvider::GetFieldDescriptorCount(DWORD* count) {
    if (!count) return E_POINTER;
    *count = kFieldCount;
    return S_OK;
}

HRESULT CMinimoProvider::GetFieldDescriptorAt(DWORD index, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** descriptor) {
    if (!descriptor) return E_POINTER;
    *descriptor = nullptr;
    if (index >= kFieldCount) return E_INVALIDARG;
    auto* field = static_cast<CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*>(CoTaskMemAlloc(sizeof(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR)));
    if (!field) return E_OUTOFMEMORY;
    *field = {index, kFields[index].type, nullptr, kFields[index].guid};
    const HRESULT hr = CopyString(kFields[index].label, &field->pszLabel);
    if (FAILED(hr)) {
        CoTaskMemFree(field);
        return hr;
    }
    *descriptor = field;
    return S_OK;
}

HRESULT CMinimoCredential::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;
    if (riid != IID_IUnknown && riid != IID_ICredentialProviderCredential &&
        riid != IID_ICredentialProviderCredential2 && riid != kIidCredential3) return E_NOINTERFACE;
    *ppv = static_cast<ICredentialProviderCredential3*>(this);
    AddRef();
    return S_OK;
}

ULONG CMinimoCredential::AddRef() { return InterlockedIncrement(&_cRef); }
ULONG CMinimoCredential::Release() {
    const ULONG refs = InterlockedDecrement(&_cRef);
    if (!refs) delete this;
    return refs;
}

HRESULT CMinimoCredential::Advise(ICredentialProviderCredentialEvents* events) {
    if (!events) return E_POINTER;
    events->AddRef();
    UnAdvise();
    _events = events;
    return S_OK;
}

HRESULT CMinimoCredential::UnAdvise() {
    auto* events = _events;
    _events = nullptr;
    if (events) events->Release();
    return S_OK;
}

HRESULT CMinimoCredential::GetFieldState(DWORD id, CREDENTIAL_PROVIDER_FIELD_STATE* state, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* interactive) {
    if (!state || !interactive) return E_POINTER;
    *state = CPFS_HIDDEN;
    *interactive = CPFIS_NONE;
    if (id >= kFieldCount) return E_INVALIDARG;
    *state = kFields[id].state;
    return S_OK;
}

HRESULT CMinimoCredential::GetStringValue(DWORD id, LPWSTR* value) {
    if (!value) return E_POINTER;
    *value = nullptr;
    return id < kFieldCount && id != LOGO ? CopyString(kFields[id].label, value) : E_INVALIDARG;
}

HRESULT CMinimoCredential::GetUserSid(LPWSTR* sid) {
    if (!sid) return E_POINTER;
    return CopyString(_sid, sid);
}

HRESULT CMinimoCredential::GetBitmapValue(DWORD id, HBITMAP* bitmap) {
    if (!bitmap) return E_POINTER;
    *bitmap = nullptr;
    if (id != LOGO) return E_INVALIDARG;
    *bitmap = static_cast<HBITMAP>(LoadImageW(g_hInstance, MAKEINTRESOURCEW(IDB_PROVIDER_LOGO),
                                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION));
    if (*bitmap) return S_OK;
    const DWORD error = GetLastError();
    return error ? HRESULT_FROM_WIN32(error) : E_FAIL;
}

HRESULT CMinimoCredential::GetBitmapBufferValue(DWORD id, DWORD* size, BYTE** buffer) {
    if (size) *size = 0;
    if (buffer) *buffer = nullptr;
    if (!size || !buffer) return E_POINTER;
    if (id != LOGO) return E_INVALIDARG;
    HRSRC resource = FindResourceW(g_hInstance, MAKEINTRESOURCEW(IDR_PROVIDER_LOGO_PNG), RT_RCDATA);
    if (!resource) return HRESULT_FROM_WIN32(ERROR_RESOURCE_NAME_NOT_FOUND);
    const DWORD bytes = SizeofResource(g_hInstance, resource);
    HGLOBAL loaded = LoadResource(g_hInstance, resource);
    const void* pixels = loaded ? LockResource(loaded) : nullptr;
    if (!pixels || !bytes) return E_FAIL;
    auto* copy = static_cast<BYTE*>(CoTaskMemAlloc(bytes));
    if (!copy) return E_OUTOFMEMORY;
    std::memcpy(copy, pixels, bytes);
    *buffer = copy;
    *size = bytes;
    return S_OK;
}

HRESULT CMinimoCredential::GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* response, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* serialization, LPWSTR* text, CREDENTIAL_PROVIDER_STATUS_ICON* icon) {
    if (!response) return E_POINTER;
    *response = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
    if (serialization) *serialization = {};
    if (text) *text = nullptr;
    if (icon) *icon = CPSI_NONE;
    return S_OK;
}

HRESULT CMinimoCredential::ReportResult(NTSTATUS, NTSTATUS, LPWSTR* text, CREDENTIAL_PROVIDER_STATUS_ICON* icon) {
    if (text) *text = nullptr;
    if (icon) *icon = CPSI_NONE;
    return S_OK;
}

HRESULT CMinimoCredential::SetSelected(BOOL* autoLogon) {
    if (!autoLogon) return E_POINTER;
    *autoLogon = FALSE;
    return S_OK;
}

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
        swprintf(text, ARRAYSIZE(text), L"Could not open the file picker. Error: 0x%08lX.",
                 static_cast<DWORD>(result));
        MessageBoxW(window, text, L"Recovery System", MB_OK | MB_ICONERROR);
    }
    opening = false;
}

HRESULT CMinimoCredential::CommandLinkClicked(DWORD id) {
    if (id != ACTION) return E_INVALIDARG;
    if (!_events) return E_UNEXPECTED;

    HWND owner = nullptr;
    auto* events = _events;
    events->AddRef();
    const HRESULT hr = events->OnCreatingWindow(&owner);
    events->Release();
    if (FAILED(hr)) return hr;
    if (!owner) return E_UNEXPECTED;
    OpenFile(owner);
    
    return S_OK;

}
