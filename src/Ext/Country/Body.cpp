#include "Body.h"

#include <HouseClass.h>
#include <Utilities/Debug.h>

std::map<HouseTypeClass*, HouseTypeExt::ExtData*> HouseTypeExt::Map;

HouseTypeExt::ExtData* HouseTypeExt::FindOrCreate(HouseTypeClass* pType)
{
    if (!pType) return nullptr;
    auto it = Map.find(pType);
    if (it != Map.end()) return it->second;
    auto* pExt = new ExtData();
    Map[pType] = pExt;
    return pExt;
}

HouseTypeExt::ExtData* HouseTypeExt::Find(HouseTypeClass* pType)
{
    if (!pType) return nullptr;
    auto it = Map.find(pType);
    return (it != Map.end()) ? it->second : nullptr;
}

void HouseTypeExt::ExtData::LoadFromINI(HouseTypeClass* pThis, CCINIClass* pINI)
{
    const char* pSection = pThis->ID;
    if (!pSection || !pINI->GetSection(pSection))
        return;

    double ratio = pINI->ReadDouble(pSection, "SuperWeapon.Ratio", SW_RechargeRatio);
    if (ratio > 0.0)
        SW_RechargeRatio = ratio;

    char buffer[256] = {};
    if (pINI->ReadString(pSection, "Inherit.SuperWeapon", "", buffer, sizeof(buffer)) > 0)
    {
        InheritSuperWeapons.clear();
        char* ctx = nullptr;
        char* tok = strtok_s(buffer, ",", &ctx);
        while (tok)
        {
            while (*tok == ' ') ++tok;
            auto* pSWType = SuperWeaponTypeClass::Find(tok);
            if (pSWType) InheritSuperWeapons.push_back(pSWType);
            tok = strtok_s(nullptr, ",", &ctx);
        }
    }

    char buffer2[256] = {};
    if (pINI->ReadString(pSection, "Inherit.LimboObjects", "", buffer2, sizeof(buffer2)) > 0)
    {
        InheritLimboObjects.clear();
        char* ctx = nullptr;
        char* tok = strtok_s(buffer2, ",", &ctx);
        while (tok)
        {
            while (*tok == ' ') ++tok;
            auto* pBldType = BuildingTypeClass::Find(tok);
            if (pBldType) InheritLimboObjects.push_back(pBldType);
            tok = strtok_s(nullptr, ",", &ctx);
        }
    }

    Debug::Log("[CountryExt] Loaded %s: SW.Ratio=%.2f InheritSW=%zu LimboObjects=%zu\n",
        pSection, SW_RechargeRatio,
        InheritSuperWeapons.size(), InheritLimboObjects.size());
}

double HouseTypeExt::GetSWRechargeRatio(HouseClass* pHouse)
{
    if (!pHouse) return 1.0;
    auto* pType = pHouse->Type;
    if (!pType) return 1.0;
    auto* pExt = Find(pType);
    if (!pExt) return 1.0;
    return (pExt->SW_RechargeRatio > 0.0) ? pExt->SW_RechargeRatio : 1.0;
}
