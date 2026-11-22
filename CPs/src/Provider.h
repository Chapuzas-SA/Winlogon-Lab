#pragma once

#include <windows.h>
#include <credentialprovider.h>

extern HINSTANCE g_hInstance;

// Interface contract: https://github.com/lithnet/windows-credential-provider/tree/main/src/Lithnet.CredentialProvider/Interop/Interfaces
#ifndef __ICredentialProviderCredential3_INTERFACE_DEFINED__
#define __ICredentialProviderCredential3_INTERFACE_DEFINED__
MIDL_INTERFACE("64a5010e-4363-41f8-9738-19045c20dabc")
ICredentialProviderCredential3 : public ICredentialProviderCredential2 {
    virtual HRESULT STDMETHODCALLTYPE GetBitmapBufferValue(DWORD id, DWORD* size, BYTE** buffer) = 0;
};
#endif

class CMinimoCredential final : public ICredentialProviderCredential3 {
public:
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents* events) override;
    IFACEMETHODIMP UnAdvise() override;
    IFACEMETHODIMP SetSelected(BOOL* autoLogon) override;
    IFACEMETHODIMP SetDeselected() override { return S_OK; }
    IFACEMETHODIMP GetFieldState(DWORD id, CREDENTIAL_PROVIDER_FIELD_STATE* state, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* interactive) override;
    IFACEMETHODIMP GetStringValue(DWORD id, LPWSTR* value) override;
    IFACEMETHODIMP GetBitmapValue(DWORD id, HBITMAP* bitmap) override;
    IFACEMETHODIMP GetBitmapBufferValue(DWORD id, DWORD* size, BYTE** buffer) override;
    IFACEMETHODIMP GetUserSid(LPWSTR* sid) override;
    IFACEMETHODIMP GetCheckboxValue(DWORD, BOOL*, LPWSTR*) override { return E_NOTIMPL; }
    IFACEMETHODIMP GetSubmitButtonValue(DWORD, DWORD*) override { return E_NOTIMPL; }
    IFACEMETHODIMP GetComboBoxValueCount(DWORD, DWORD*, DWORD*) override { return E_NOTIMPL; }
    IFACEMETHODIMP GetComboBoxValueAt(DWORD, DWORD, LPWSTR*) override { return E_NOTIMPL; }
    IFACEMETHODIMP SetStringValue(DWORD, LPCWSTR) override { return E_NOTIMPL; }
    IFACEMETHODIMP SetCheckboxValue(DWORD, BOOL) override { return E_NOTIMPL; }
    IFACEMETHODIMP SetComboBoxSelectedValue(DWORD, DWORD) override { return E_NOTIMPL; }
    IFACEMETHODIMP CommandLinkClicked(DWORD id) override;
    IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* response, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* serialization, LPWSTR* text, CREDENTIAL_PROVIDER_STATUS_ICON* icon) override;
    IFACEMETHODIMP ReportResult(NTSTATUS, NTSTATUS, LPWSTR* text, CREDENTIAL_PROVIDER_STATUS_ICON* icon) override;

    // Takes ownership of the SID allocated by ICredentialProviderUser::GetSid.
    explicit CMinimoCredential(LPWSTR sid) : _sid(sid) {}
    CMinimoCredential(const CMinimoCredential&) = delete;
    CMinimoCredential& operator=(const CMinimoCredential&) = delete;

private:
    ~CMinimoCredential() { UnAdvise(); CoTaskMemFree(_sid); }
    LONG _cRef = 1;
    LPWSTR _sid;
    ICredentialProviderCredentialEvents* _events = nullptr;
};

// One sign-in option for each user enumerated by Windows.
class CMinimoProvider final : public ICredentialProvider, public ICredentialProviderSetUserArray {
public:
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO scenario, DWORD) override;
    IFACEMETHODIMP SetUserArray(ICredentialProviderUserArray* users) override;
    IFACEMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*) override { return S_OK; }
    IFACEMETHODIMP Advise(ICredentialProviderEvents*, UINT_PTR) override { return S_OK; }
    IFACEMETHODIMP UnAdvise() override { return S_OK; }
    IFACEMETHODIMP GetFieldDescriptorCount(DWORD* count) override;
    IFACEMETHODIMP GetFieldDescriptorAt(DWORD index, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** descriptor) override;
    IFACEMETHODIMP GetCredentialCount(DWORD* count, DWORD* defaultIndex, BOOL* autoLogon) override;
    IFACEMETHODIMP GetCredentialAt(DWORD index, ICredentialProviderCredential** credential) override;

    CMinimoProvider() = default;
    CMinimoProvider(const CMinimoProvider&) = delete;
    CMinimoProvider& operator=(const CMinimoProvider&) = delete;

private:
    ~CMinimoProvider();
    void ClearCredentials();
    LONG _cRef = 1;
    CMinimoCredential** _credentials = nullptr;
    DWORD _credentialCount = 0;
};
