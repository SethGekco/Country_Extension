// ─────────────────────────────────────────────────────────────────────────────
// Registration snippet for your main extension dispatcher file
// (the equivalent of Phobos's Phobos.Ext.cpp, or your project's top-level
// dispatcher that wires all ExtMap pairs together).
//
// Add these lines alongside your existing Ext registrations.
// ─────────────────────────────────────────────────────────────────────────────

// At the top of your dispatcher file, add:
#include "Ext/Country/Body.h"

// Inside your ExtContainer registration block (wherever you do
// ExtMap.Register() for other type extensions), add:
//
//   HouseTypeExt::ExtMap.Register();
//
// Inside your INI-load dispatcher (the function that calls LoadFromINI for
// each ext), add the HouseTypeClass pass:
//
//   HouseTypeExt::ExtMap.LoadAllFromINI(pINI);
//   // This iterates all CountryTypeClass instances and calls
//   // ExtData::LoadFromINI on each one.
//
// Inside your save/load handlers:
//
//   HouseTypeExt::ExtMap.SaveStatic(Stm);   // in save
//   HouseTypeExt::ExtMap.LoadStatic(Stm);   // in load

// ─────────────────────────────────────────────────────────────────────────────
// rulesmd.ini usage reference
// ─────────────────────────────────────────────────────────────────────────────
//
// [Americans]
// SuperWeapon.Ratio=0.75        ; Americans recharge SWs 25% faster
// Inherit.SuperWeapon=          ; (empty — no inherited SWs for Americans)
// Inherit.LimboObjects=         ; (empty — no limbo buildings for Americans)
//
// [Russians]
// SuperWeapon.Ratio=1.0         ; default — no change
// Inherit.SuperWeapon=NukeSpecial  ; Russia always has the nuke SW available
// Inherit.LimboObjects=         ; (pending Phobos interop review)
//
// [YuriCountry]
// SuperWeapon.Ratio=1.5         ; Yuri's SWs recharge 50% slower
// Inherit.SuperWeapon=YuriPsychicDom,MasterMind  ; example list
// Inherit.LimboObjects=GHOSTSTLTH  ; example limbo building at start
