#pragma once
#include <windows.h>
#include <credentialprovider.h>
#include <atomic>
#include <new>

inline constexpr CLSID CLSID_PreAuthPLAP =
    {0x8b0c5df7, 0x3f70, 0x4b90, {0x9b, 0x70, 0x61, 0xa6, 0x3a, 0x3b, 0x5f, 0x25}};
inline constexpr DWORD FieldTitle = 0;
inline constexpr DWORD FieldSubmit = 1;
inline constexpr DWORD FieldCount = 2;

void ModuleAddRef() noexcept;
void ModuleRelease() noexcept;
long ModuleReferences() noexcept;
HRESULT CopyString(const wchar_t* source, PWSTR* destination) noexcept;
HRESULT CreateProvider(REFIID iid, void** result) noexcept;
HRESULT CreateClassFactory(REFIID iid, void** result) noexcept;

class PreAuthPLAPCredential final : public IConnectableCredentialProviderCredential {
public:
    PreAuthPLAPCredential() noexcept;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE Advise(ICredentialProviderCredentialEvents*) override;
    HRESULT STDMETHODCALLTYPE UnAdvise() override;
    HRESULT STDMETHODCALLTYPE SetSelected(BOOL*) override;
    HRESULT STDMETHODCALLTYPE SetDeselected() override;
    HRESULT STDMETHODCALLTYPE GetFieldState(DWORD, CREDENTIAL_PROVIDER_FIELD_STATE*, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE*) override;
    HRESULT STDMETHODCALLTYPE GetStringValue(DWORD, PWSTR*) override;
    HRESULT STDMETHODCALLTYPE GetBitmapValue(DWORD, HBITMAP*) override;
    HRESULT STDMETHODCALLTYPE GetCheckboxValue(DWORD, BOOL*, PWSTR*) override;
    HRESULT STDMETHODCALLTYPE GetSubmitButtonValue(DWORD, DWORD*) override;
    HRESULT STDMETHODCALLTYPE GetComboBoxValueCount(DWORD, DWORD*, DWORD*) override;
    HRESULT STDMETHODCALLTYPE GetComboBoxValueAt(DWORD, DWORD, PWSTR*) override;
    HRESULT STDMETHODCALLTYPE SetStringValue(DWORD, PCWSTR) override;
    HRESULT STDMETHODCALLTYPE SetCheckboxValue(DWORD, BOOL) override;
    HRESULT STDMETHODCALLTYPE SetComboBoxSelectedValue(DWORD, DWORD) override;
    HRESULT STDMETHODCALLTYPE CommandLinkClicked(DWORD) override;
    HRESULT STDMETHODCALLTYPE GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*, PWSTR*, CREDENTIAL_PROVIDER_STATUS_ICON*) override;
    HRESULT STDMETHODCALLTYPE ReportResult(NTSTATUS, NTSTATUS, PWSTR*, CREDENTIAL_PROVIDER_STATUS_ICON*) override;
    HRESULT STDMETHODCALLTYPE Connect(IQueryContinueWithStatus*) override;
    HRESULT STDMETHODCALLTYPE Disconnect() override;
private:
    ~PreAuthPLAPCredential();
    std::atomic<ULONG> refs_{1};
    SRWLOCK eventsLock_ = SRWLOCK_INIT;
    ICredentialProviderCredentialEvents* events_ = nullptr;
    std::atomic<bool> completed_{false};
    std::atomic<bool> connecting_{false};
};
