#include "PreAuthPLAP.h"

namespace {
class ClassFactory final : public IClassFactory {
public:
    ClassFactory() noexcept { ModuleAddRef(); }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) override {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (iid != IID_IUnknown && iid != IID_IClassFactory) return E_NOINTERFACE;
        *result = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++refs_; }
    ULONG STDMETHODCALLTYPE Release() override {
        const ULONG refs = --refs_;
        if (!refs) delete this;
        return refs;
    }
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* outer, REFIID iid, void** result) override {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (outer) return CLASS_E_NOAGGREGATION;
        return CreateProvider(iid, result);
    }
    HRESULT STDMETHODCALLTYPE LockServer(BOOL lock) override {
        if (lock) ModuleAddRef(); else ModuleRelease();
        return S_OK;
    }
private:
    ~ClassFactory() { ModuleRelease(); }
    std::atomic<ULONG> refs_{1};
};
}
HRESULT CreateClassFactory(REFIID iid, void** result) noexcept {
    auto factory = new (std::nothrow) ClassFactory;
    if (!factory) return E_OUTOFMEMORY;
    const HRESULT hr = factory->QueryInterface(iid, result);
    factory->Release();
    return hr;
}
