// CountryTypeExt — Hooks.cpp
//
// Hooks wired here:
//
//   1. CountryTypeClass::ReadINI intercept (0x51214F)
//      Verified: FUN_00511850, hooks MOV AL,1 epilogue.
//      EBX = HouseTypeClass* pThis, ESI = CCINIClass* pINI.
//
//   2. SuperWeapon.Ratio= (0x6CC20C)
//      Verified: inside FUN_006cc1e0 (SW recharge timer arming).
//      Hooks after RechargeTime is loaded into ESI.
//      EDI = SuperWeaponClass* pSW (Owner confirmed at +0x2c via SuperClass.h).
//
//   3. Inherit.SuperWeapon= — STUB, address not yet determined.

#include "Body.h"

#include <HouseClass.h>
#include <HouseTypeClass.h>
#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <CCINIClass.h>
#include <Macros.h>

DEFINE_HOOK(0x51214F, CountryTypeClass_ReadINI_Ext, 5)
{
    GET(HouseTypeClass*, pThis, EBX);
    GET(CCINIClass*,     pINI,  ESI);

    if (auto* pExt = HouseTypeExt::ExtMap.Find(pThis))
        pExt->LoadFromINI(pINI);

    return 0;
}

DEFINE_HOOK(0x6CC20C, SuperWeapon_Recharge_ApplyCountryRatio, 4)
{
    GET(SuperWeaponClass*, pSW, EDI);
    GET(int, rechargeTime, ESI);

    HouseClass* pOwner = pSW->Owner;

    if (pOwner)
    {
        const double ratio = HouseTypeExt::GetSWRechargeRatio(pOwner);
        if (ratio != 1.0)
        {
            int scaled = static_cast<int>(rechargeTime * ratio + 0.5);
            if (scaled < 1) scaled = 1;
            R->ESI(scaled);
        }
    }

    return 0;
}

/*
DEFINE_HOOK(0x00000000, HouseClass_UpdateSWAvailability_InheritGrant, 6)
{
    GET(HouseClass*,       pHouse, ???);
    GET(SuperWeaponClass*, pSuper, ???);

    auto* pTypeExt = HouseTypeExt::ExtMap.Find(pHouse->Type);
    if (pTypeExt)
    {
        auto* pSWType = pSuper->Type;
        for (auto* pInherit : pTypeExt->InheritSuperWeapons)
        {
            if (pInherit == pSWType)
            {
                pSuper->IsPresent = true;
                return 0x00000000;
            }
        }
    }

    return 0;
}
*/
