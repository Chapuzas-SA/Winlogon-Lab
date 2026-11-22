#include "PreAuthPLAP.h"

namespace { std::atomic<long> moduleRefs{0}; }
void ModuleAddRef() noexcept { ++moduleRefs; }
void ModuleRelease() noexcept { --moduleRefs; }
long ModuleReferences() noexcept { return moduleRefs.load(); }

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID clsid, REFIID iid, void** result) {
    if (!result) return E_POINTER;
    *result = nullptr;
    if (clsid != CLSID_PreAuthPLAP) return CLASS_E_CLASSNOTAVAILABLE;
    return CreateClassFactory(iid, result);
}
extern "C" HRESULT __stdcall DllCanUnloadNow() {
    return ModuleReferences() == 0 ? S_OK : S_FALSE;
}
