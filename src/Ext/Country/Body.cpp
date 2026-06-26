// CountryTypeExt — Body.cpp
// Defines the ExtMap, LoadFromINI, and stream serialization for HouseTypeExt.

#include "Body.h"

#include <HouseClass.h>
#include <CCINIClass.h>

// ─────────────────────────────────────────────────────────────────────────────
// ExtMap instantiation
// One ExtData per CountryTypeClass, keyed by pointer.
// ─────────────────────────────────────────────────────────────────────────────
ExtContainer HouseTypeExt::ExtMap;

// ─────────────────────────────────────────────────────────────────────────────
// INI loading
// Called by the hook that intercepts CountryTypeClass::ReadINI (see Hooks.cpp).
// pINI is the rules(md).ini file being parsed.
// ─────────────────────────────────────────────────────────────────────────────
void HouseTypeExt::ExtData::LoadFromINI(CCINIClass* pINI)
{
    // The section name matches the country's INI ID, e.g. [Americans].
    const char* pSection = this->OwnerObject()->ID;
    if (!pSection || !pINI->GetSection(pSection))
        return;

    INI_EX exINI(pINI);

    // SuperWeapon.Ratio=  (float, default 1.0)
    // Values below 1.0 speed up recharging; above 1.0 slow it down.
    // Clamped to > 0 at application time in the hook, not here.
    SW_RechargeRatio.Read(exINI, pSection, "SuperWeapon.Ratio");

    // Inherit.SuperWeapon=  (list of SuperWeaponType IDs)
    InheritSuperWeapons.Read(exINI, pSection, "Inherit.SuperWeapon");

    // Inherit.LimboObjects=  (list of BuildingType IDs)
    // Parsed but not yet acted on; hook implementation is deferred.
    InheritLimboObjects.Read(exINI, pSection, "Inherit.LimboObjects");
}

// ─────────────────────────────────────────────────────────────────────────────
// Save / load (savegame support)
// ─────────────────────────────────────────────────────────────────────────────
void HouseTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
    Extension<HouseTypeClass>::LoadFromStream(Stm);
    this->Serialize(Stm);
}

void HouseTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
    Extension<HouseTypeClass>::SaveToStream(Stm);
    this->Serialize(Stm);
}

// ─────────────────────────────────────────────────────────────────────────────
// Global save / load (called by the main save/load dispatch)
// ─────────────────────────────────────────────────────────────────────────────
bool HouseTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
    return Stm.Success();
}

bool HouseTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
    return Stm.Success();
}

// ─────────────────────────────────────────────────────────────────────────────
// HouseTypeExt::GetSWRechargeRatio
//
// Safe accessor used by the SW timer hook in Hooks.cpp.
// Walks: HouseClass -> Type (CountryTypeClass) -> ExtData -> SW_RechargeRatio.
// Returns 1.0 if anything in that chain is null, so the hook degrades
// gracefully for houses that have no country or no ext entry.
// ─────────────────────────────────────────────────────────────────────────────
double HouseTypeExt::GetSWRechargeRatio(HouseClass* pHouse)
{
    if (!pHouse)
        return 1.0;

    auto* pType = pHouse->Type;
    if (!pType)
        return 1.0;

    auto* pExt = HouseTypeExt::ExtMap.Find(pType);
    if (!pExt)
        return 1.0;

    // Guard against degenerate values: ratio must be strictly positive.
    // A ratio of 0 would make RechargeTime = 0 → instant-fire every frame.
    const double ratio = pExt->SW_RechargeRatio;
    return (ratio > 0.0) ? ratio : 1.0;
}
