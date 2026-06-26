#pragma once

// CountryTypeExt — Body.h
// Extension data for CountryTypeClass (INI [Countries] entries).
//
// Features scaffolded here:
//   SuperWeapon.Ratio=   per-country recharge time multiplier (this file, fully wired)
//   Inherit.SuperWeapon= country-granted SWs without a building (struct only, hook in Hooks.cpp)
//   Inherit.LimboObjects= country-granted limbo buildings at start (struct placeholder, TBD)

#include <HouseTypeClass.h>
#include <SuperWeaponTypeClass.h>
#include <BuildingTypeClass.h>

#include <Utilities/Container.h>     // Extension<T>, ExtMap<T,Ext>
#include <Utilities/TemplateDef.h>   // Nullable<>, ValueableVector<>

// ─────────────────────────────────────────────────────────────────────────────
// Forward declarations
// ─────────────────────────────────────────────────────────────────────────────
class HouseClass;

// ─────────────────────────────────────────────────────────────────────────────
// HouseTypeExt  (CountryTypeClass extension)
// ─────────────────────────────────────────────────────────────────────────────
class HouseTypeExt
{
public:
    // ── Extension data struct ────────────────────────────────────────────────
    // One instance per CountryTypeClass, lifetime managed by ExtMap.
    class ExtData final : public Extension<HouseTypeClass>
    {
    public:
        // ── SuperWeapon.Ratio= ───────────────────────────────────────────────
        // Multiplier applied to every SW's RechargeTime for houses of this
        // country.  1.0 = no change, 0.5 = recharges twice as fast.
        // Defaults to 1.0 (neutral).
        Valueable<double> SW_RechargeRatio;

        // ── Inherit.SuperWeapon= ─────────────────────────────────────────────
        // SWs that this country always has available, regardless of whether the
        // providing building has been constructed.  Analogous to SW.AlwaysGranted
        // but scoped to a single country.
        // Hook for the availability update is in Hooks.cpp.
        ValueableVector<SuperWeaponTypeClass*> InheritSuperWeapons;

        // ── Inherit.LimboObjects= ────────────────────────────────────────────
        // BuildingTypes to spawn into the house's limbo tracker at game start,
        // as if a LimboDelivery SW had already fired.
        // NOTE: implementation deferred pending Phobos interop review.
        ValueableVector<BuildingTypeClass*> InheritLimboObjects;

        // ── Constructor / destructor ─────────────────────────────────────────
        explicit ExtData(HouseTypeClass* pOwner)
            : Extension<HouseTypeClass>(pOwner)
            , SW_RechargeRatio { 1.0 }      // neutral: no change to recharge speed
            , InheritSuperWeapons {}
            , InheritLimboObjects {}
        { }

        virtual ~ExtData() = default;

        // ── Extension interface ──────────────────────────────────────────────
        virtual void LoadFromINI(CCINIClass* pINI) override;
        virtual void LoadFromStream(PhobosStreamReader& Stm) override;
        virtual void SaveToStream(PhobosStreamWriter& Stm) override;

    private:
        template <typename T>
        bool Serialize(T& Stm)
        {
            return Stm
                .Process(SW_RechargeRatio)
                .Process(InheritSuperWeapons)
                .Process(InheritLimboObjects)
                .Success();
        }
    };

    // ── Static helpers exposed to hooks in other files ───────────────────────

    // Returns the SW recharge ratio for the country that owns pHouse.
    // Returns 1.0 safely if pHouse or its type has no ext data.
    static double GetSWRechargeRatio(HouseClass* pHouse);

    // ExtMap and registration boilerplate
    static ExtContainer ExtMap;
    static bool LoadGlobals(PhobosStreamReader& Stm);
    static bool SaveGlobals(PhobosStreamWriter& Stm);
};
