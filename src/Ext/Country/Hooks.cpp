#include "Body.h"

#include <HouseClass.h>
#include <HouseTypeClass.h>
#include <SuperClass.h>
#include <CCINIClass.h>
#include <Helpers/Macro.h>

// Hook 1 — CountryTypeClass::ReadINI
// FUN_00511850, hook at 0x51214F (MOV AL,1 — 5 bytes)
// EBX = HouseTypeClass* pThis, ESI = CCINIClass* pINI
DEFINE_HOOK(0x51214F, CountryTypeClass_ReadINI_Ext, 5)
{
    GET(HouseTypeClass*, pThis, EBX);
    GET(CCINIClass*,     pINI,  ESI);

    if (auto* pExt = HouseTypeExt::FindOrCreate(pThis))
        pExt->LoadFromINI(pThis, pINI);

    return 0;
}

// Hook 2 — SuperWeapon.Ratio=
// FUN_006cc1e0, hook at 0x6CC20C (FILD — 4 bytes)
// EDI = SuperClass* pSW, ESI = RechargeTime (int, frames)
// SuperClass field layout (SuperClass.h + disasm confirmed):
//   +0x24 CustomChargeTime, +0x28 Type, +0x2c Owner, +0x30 RechargeTimer
//DEFINE_HOOK(0x6CC20C, SuperWeapon_Recharge_ApplyCountryRatio, 4)
{
    GET(SuperClass*,  pSW,          EDI);
    GET(int,          rechargeTime, ESI);

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

// Hook 3 — Inherit.SuperWeapon= (STUB, address not yet found)
/*
DEFINE_HOOK(0x00000000, HouseClass_UpdateSWAvailability_InheritGrant, 6)
{
    GET(HouseClass*, pHouse, ???);
    GET(SuperClass*, pSuper, ???);

    auto* pTypeExt = HouseTypeExt::Find(pHouse->Type);
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
