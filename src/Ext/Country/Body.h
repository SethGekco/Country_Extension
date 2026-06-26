#pragma once

#include <HouseTypeClass.h>
#include <SuperWeaponTypeClass.h>
#include <BuildingTypeClass.h>
#include <CCINIClass.h>
#include <map>
#include <vector>

class HouseClass;

class HouseTypeExt
{
public:
    struct ExtData
    {
        double SW_RechargeRatio = 1.0;
        std::vector<SuperWeaponTypeClass*> InheritSuperWeapons;
        std::vector<BuildingTypeClass*> InheritLimboObjects;

        void LoadFromINI(HouseTypeClass* pThis, CCINIClass* pINI);
    };

    static std::map<HouseTypeClass*, ExtData*> Map;

    static ExtData* FindOrCreate(HouseTypeClass* pType);
    static ExtData* Find(HouseTypeClass* pType);
    static double GetSWRechargeRatio(HouseClass* pHouse);
};
