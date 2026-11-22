#include "Provider.h"

#include <new>

HINSTANCE g_hInstance = nullptr;

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) g_hInstance = instance;
    return TRUE;
}

namespace {
const GUID CLSID_MinimoProvider = {0xe2e9a68f, 0x4d2a, 0x4b6e, {0x9e, 0xbe, 0x0c, 0x2f, 0x19, 0xe9, 0x2d, 0x23}};

// The static factory holds a reference for the lifetime of the DLL.
class CMinimoFactory final : public IClassFactory {
public:
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        *ppv = nullptr;
        if (riid != IID_IUnknown && riid != IID_IClassFactory) return E_NOINTERFACE;
        *ppv = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }
    IFACEMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG) Release() override { return InterlockedDecrement(&_cRef); }

    IFACEMETHODIMP CreateInstance(IUnknown* outer, REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        *ppv = nullptr;
        if (outer) return CLASS_E_NOAGGREGATION;
        auto* provider = new (std::nothrow) CMinimoProvider;
        if (!provider) return E_OUTOFMEMORY;
        const HRESULT hr = provider->QueryInterface(riid, ppv);
        provider->Release();
        return hr;
    }
    IFACEMETHODIMP LockServer(BOOL) override { return S_OK; }

private:
    LONG _cRef = 1;
};
}

extern "C" __declspec(dllexport) HRESULT __stdcall DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;
    if (clsid != CLSID_MinimoProvider) return CLASS_E_CLASSNOTAVAILABLE;
    static CMinimoFactory factory;
    return factory.QueryInterface(riid, ppv);
}

extern "C" __declspec(dllexport) HRESULT __stdcall DllCanUnloadNow() {
    // Remains loaded: object and server lock counts are not tracked globally.
    return S_FALSE;
}
