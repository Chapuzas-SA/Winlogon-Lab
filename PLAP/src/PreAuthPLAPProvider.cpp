#include "PreAuthPLAP.h"
#include <cwchar>
#include <cstring>

HRESULT CopyString(const wchar_t* source, PWSTR* destination) noexcept {
    if (!destination) return E_POINTER;
    *destination = nullptr;
    const size_t bytes = (std::wcslen(source) + 1) * sizeof(wchar_t);
    auto copy = static_cast<PWSTR>(CoTaskMemAlloc(bytes));
    if (!copy) return E_OUTOFMEMORY;
    std::memcpy(copy, source, bytes);
    *destination = copy;
    return S_OK;
}

namespace {
class PreAuthPLAPProvider final : public ICredentialProvider {
public:
    PreAuthPLAPProvider() noexcept { ModuleAddRef(); }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) override {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (iid != IID_IUnknown && iid != IID_ICredentialProvider) return E_NOINTERFACE;
        *result = static_cast<ICredentialProvider*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++refs_; }
    ULONG STDMETHODCALLTYPE Release() override {
        const ULONG refs = --refs_;
        if (!refs) delete this;
        return refs;
    }
    HRESULT STDMETHODCALLTYPE SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO scenario, DWORD) override {
        if (scenario != CPUS_PLAP) return E_NOTIMPL;
        if (!credential_) credential_ = new (std::nothrow) PreAuthPLAPCredential;
        if (!credential_) return E_OUTOFMEMORY;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*) override { return E_NOTIMPL; }
    // This provider never changes its credential list, so no provider events are retained.
    HRESULT STDMETHODCALLTYPE Advise(ICredentialProviderEvents*, UINT_PTR) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE UnAdvise() override { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetFieldDescriptorCount(DWORD* count) override {
        if (!count) return E_POINTER;
        *count = FieldCount;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetFieldDescriptorAt(DWORD index, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** result) override {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (index >= FieldCount) return E_INVALIDARG;
        auto field = static_cast<CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*>(CoTaskMemAlloc(sizeof(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR)));
        if (!field) return E_OUTOFMEMORY;
        *field = {};
        field->dwFieldID = index;
        field->cpft = index == FieldTitle ? CPFT_LARGE_TEXT : CPFT_SUBMIT_BUTTON;
        const HRESULT hr = CopyString(index == FieldTitle ? L"PreAuth PLAP" : L"Connect", &field->pszLabel);
        if (FAILED(hr)) { CoTaskMemFree(field); return hr; }
        *result = field;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetCredentialCount(DWORD* count, DWORD* defaultIndex, BOOL* autoLogon) override {
        if (!count || !defaultIndex || !autoLogon) return E_POINTER;
        *count = credential_ ? 1 : 0;
        *defaultIndex = CREDENTIAL_PROVIDER_NO_DEFAULT;
        *autoLogon = FALSE;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetCredentialAt(DWORD index, ICredentialProviderCredential** result) override {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (index != 0 || !credential_) return E_INVALIDARG;
        return credential_->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(result));
    }
private:
    ~PreAuthPLAPProvider() {
        if (credential_) credential_->Release();
        ModuleRelease();
    }
    std::atomic<ULONG> refs_{1};
    PreAuthPLAPCredential* credential_ = nullptr;
};
}
HRESULT CreateProvider(REFIID iid, void** result) noexcept {
    auto provider = new (std::nothrow) PreAuthPLAPProvider;
    if (!provider) return E_OUTOFMEMORY;
    const HRESULT hr = provider->QueryInterface(iid, result);
    provider->Release();
    return hr;
}
