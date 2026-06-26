// CountryExtension.cpp
// DLL entry point. Registers the HouseTypeExt ExtMap so Syringe
// hooks and INI loading work correctly.

#include "CountryExtension.h"
#include "Ext/Country/Body.h"

#include <Utilities/Patch.h>

// ── ExtMap instantiation is in Body.cpp ─────────────────────────────────────
// This file only handles DLL lifetime and hook registration.

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        // Register the HouseTypeClass extension map so Syringe can
        // locate ext data by pointer at runtime.
        HouseTypeExt::ExtMap.Register();
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
