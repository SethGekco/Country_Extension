// CountryTypeExt — Hooks.cpp
//
// Hooks wired here:
//
//   1. CountryTypeClass::ReadINI intercept
//      Calls HouseTypeExt::ExtData::LoadFromINI after the vanilla read so our
//      tags are parsed from the same rules(md).ini pass.
//
//   2. SuperWeapon.Ratio= — SW recharge timer arming hook
//      Address: 0x6BCCF0  (SuperWeaponClass::Recharge, the call to
//      CDTimerClass::Start that arms the countdown after a SW fires or at
//      game start).
//      *** ADDRESS MUST BE VERIFIED AGAINST GAMEMD.EXE BEFORE SHIPPING ***
//
//   3. Inherit.SuperWeapon= — SW availability grant hook
//      Address: STUB — needs disassembly of the SW availability update loop
//      (the function that iterates pHouse->Supers and sets IsPresent/IsCharged).
//      Marked TODO; skeleton is present so you can fill the address in.

#include "Body.h"

#include <HouseClass.h>
#include <HouseTypeClass.h>
#include <SuperWeaponClass.h>
#include <SuperWeaponTypeClass.h>
#include <CCINIClass.h>
#include <Macros.h>   // DEFINE_HOOK, GET, R, etc.

// ═════════════════════════════════════════════════════════════════════════════
// Hook 1 — CountryTypeClass INI loading
//
// The vanilla engine reads each country's section via
// CountryTypeClass::ReadINI (or a thin wrapper that calls it for each entry
// in [Countries]).  We hook the tail of that function to run our own parse
// immediately after the vanilla fields have been set.
//
// Address 0x507E27 is a PLACEHOLDER.
// To find the real address:
//   - In Ghidra/IDA, search for xref to the string "VeteranInfantry" inside
//     a function that also touches HouseTypeClass fields.  That function is
//     CountryTypeClass::ReadINI (or the caller loop).  Hook its RET or the
//     last instruction before it returns.
// ═════════════════════════════════════════════════════════════════════════════
// Verified: FUN_00511850 (CountryTypeClass::ReadINI)
// EBX = HouseTypeClass* pThis  (set at 00511861: MOV EBX, ECX)
// ESI = CCINIClass* pINI       (set at 0051185e: MOV ESI, [EBP+8])
// Hook at 0051214f (MOV AL,1) — 5 bytes, just before POP EDI/ESI/EBX epilogue.
// ESI is reloaded several times during the function but is always pINI.
DEFINE_HOOK(0x51214F, CountryTypeClass_ReadINI_Ext, 5)
{
    GET(HouseTypeClass*, pThis, EBX);
    GET(CCINIClass*,     pINI,  ESI);

    if (auto* pExt = HouseTypeExt::ExtMap.Find(pThis))
        pExt->LoadFromINI(pINI);

    return 0; // continue to original epilogue
}

// ═════════════════════════════════════════════════════════════════════════════
// Hook 2 — SuperWeapon.Ratio=
//
// Target: the point where a SuperWeaponClass arms its recharge countdown.
// In the disassembly this appears as a call to CDTimerClass::Start with the
// recharge duration in frames (from SuperWeaponTypeClass::RechargeTime *
// some scaling).  We intercept after the duration is computed but before
// Start() is called, multiply by the country ratio, then let Start() run.
//
// The pattern to look for in Ghidra:
//   - Function that is called both at game-start SW init AND after SW fires
//   - Contains a read of SuperWeaponTypeClass::RechargeTime (offset into the
//     type pointer stored in SuperWeaponClass)
//   - Calls CDTimerClass::Start on this->RechargeTimer
//
// KNOWN REFERENCE: Ares hooks SW recharge via a function at approximately
// 0x6BCCF0.  Cross-reference that address in your gamemd.exe — if it matches
// a Start() call for the SW timer, that is your target.
//
// Two plausible hook strategies:
//
//   A) Hook the CDTimerClass::Start call site inside SuperWeaponClass::Recharge
//      and redirect the duration argument before Start executes.
//      Advantage: single hook site, affects both initial arm and re-arm.
//
//   B) Hook SuperWeaponClass::Update (the per-frame tick that calls Recharge
//      once the SW is available) and scale the timer after the fact.
//      Disadvantage: harder to keep in sync with the display timer.
//
// Strategy A is strongly preferred.  Skeleton below uses Strategy A.
//
// ─────────────────────────────────────────────────────────────────────────────
// Register state at the hook site (Strategy A, call-site hook):
//   ECX  = pointer to CDTimerClass (this->RechargeTimer inside SuperWeaponClass)
//   EDX  = duration in frames (the value about to be passed to Start)
//   ESI  = pointer to SuperWeaponClass (inferred — verify in disasm)
//
// IMPORTANT: the exact registers depend on the calling convention at this
// callsite.  Verify with your disassembler before finalising.
// ═════════════════════════════════════════════════════════════════════════════

// ── Hook 2 ───────────────────────────────────────────────────────────────────
// Verified: inside FUN_006cc1e0 (SW recharge timer arming function).
//
// Context at hook site 006CC206:
//   EDI = SuperWeaponClass* pSW    (set at 006cc1e4: MOV EDI, ECX)
//   EAX = SuperWeaponTypeClass*    (set at 006cc203: MOV EAX, [EDI+0x28])
//   ESI is about to be written with RechargeTime from [EAX+0xb0]
//
// SuperWeaponClass::Owner = pSW + 0x44
//   (confirmed from FUN_006dd8b0 using [ESI+0x44] as house identifier
//    passed to FUN_0068bcc0 throughout the SW execute dispatch)
//
// We hook the 6-byte instruction at 006CC206:
//   8B B0 B0 00 00 00   MOV ESI, dword ptr [EAX + 0xb0]
// After this hook fires, ESI will hold the (possibly scaled) RechargeTime.
// We let the instruction execute normally by returning 0, then scale ESI
// after by writing it back — actually easier to hook AFTER the MOV at
// 006CC20C and scale ESI there before FIMUL uses it at 006CC21A.
//
// Hook at 006CC20C (LAB_006cc20c, 4 bytes: DB 44 24 1C = FILD):
//   At this point ESI already holds RechargeTime (or the per-instance
//   override). EDI = SuperWeaponClass*. Scale ESI here, write back,
//   then fall through to FILD and FIMUL.
DEFINE_HOOK(0x6CC20C, SuperWeapon_Recharge_ApplyCountryRatio, 4)
{
    GET(SuperWeaponClass*, pSW, EDI);
    GET(int, rechargeTime, ESI);

    // Walk pSW -> Owner (HouseClass* at offset +0x44)
    HouseClass* pOwner = reinterpret_cast<HouseClass*>(
        *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pSW) + 0x44)
    );

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

    return 0; // fall through to FILD [ESP+Stack] then FIMUL ESI
}

// ═════════════════════════════════════════════════════════════════════════════
// Hook 3 — Inherit.SuperWeapon= (SW availability grant)
//
// STATUS: STUB — address not yet determined.
//
// What we need to hook:
//   The function (or the loop inside it) that evaluates SW availability for a
//   house.  In vanilla YR this runs whenever a building is placed, sold, or
//   captured, and also once at game start.  It iterates pHouse->Supers
//   (a DynamicVectorClass<SuperWeaponClass*>) and for each entry calls
//   something equivalent to:
//       pSuper->IsPresent = pHouse->OwnsSomethingThatGrantsThisSW(pSuper->Type);
//
//   Ares extends this with SW.AlwaysGranted by injecting a bypass before that
//   check.  We want to do the same, but conditioned on the house's country
//   having the SW listed in Inherit.SuperWeapon=.
//
// Finding the address:
//   In Ghidra, look for a function that:
//     - Is called from building placement/destruction handlers
//     - Iterates over HouseClass::Supers (DVC of SuperWeaponClass*)
//     - Calls SuperWeaponClass::IsPresent setter or writes to its IsPresent field
//     - References BuildingTypeClass::SuperWeapon or similar to test ownership
//
//   Cross-reference with Ares's SW.AlwaysGranted hook for the same callsite.
//
// Once the address is known, the logic is:
//   1. GET(HouseClass*, pHouse, ...)  and  GET(SuperWeaponClass*, pSuper, ...)
//   2. Look up pHouse->Type in HouseTypeExt::ExtMap
//   3. Check if pSuper->Type is in pExt->InheritSuperWeapons
//   4. If yes, force IsPresent = true (or set the availability flag) and return
//      a jump address that skips the vanilla building-ownership check.
//
// Skeleton below is commented out until the address is confirmed.
// ═════════════════════════════════════════════════════════════════════════════

/*
// TODO: fill in verified address before enabling.
DEFINE_HOOK(0x00000000, HouseClass_UpdateSWAvailability_InheritGrant, 6)
{
    GET(HouseClass*,       pHouse, ???); // verify register
    GET(SuperWeaponClass*, pSuper, ???); // verify register

    auto* pTypeExt = HouseTypeExt::ExtMap.Find(pHouse->Type);
    if (pTypeExt)
    {
        auto* pSWType = pSuper->Type;
        for (auto* pInherit : pTypeExt->InheritSuperWeapons)
        {
            if (pInherit == pSWType)
            {
                // Force availability on — same effect as SW.AlwaysGranted
                // for this country specifically.
                pSuper->IsPresent = true;
                // Return the address AFTER the vanilla ownership check so we
                // skip it entirely, analogous to what Ares does for AlwaysGranted.
                return 0x00000000; // ← address of next instruction after vanilla check
            }
        }
    }

    return 0; // no match — let vanilla logic run normally
}
*/
