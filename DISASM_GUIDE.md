# Address Verification Guide
## CountryTypeExt — Finding Hook Addresses in gamemd.exe

All three hook addresses in `Hooks.cpp` are currently stubs.  This guide
walks through finding each one in Ghidra (or IDA).  Work through them in
order — Hook 1 and Hook 2 are independent; Hook 3 builds on knowledge of
Hook 2's context.

---

## Hook 1 — CountryTypeClass::ReadINI
**Purpose:** Call our `LoadFromINI` after vanilla country fields are parsed.

### Strategy
CountryTypeClass is read during rules loading.  The vanilla parser reads
known tags like `VeteranInfantry=`, `CostUnitsMult=`, etc.  Find that
function, then hook its epilogue (the point just before it returns to the
loop that processes the next country).

### Steps

1. **String search:** In Ghidra's Search → For Strings, search for
   `VeteranInfantry`.  There should be one reference used as an INI key.

2. **Follow the xref:** Double-click the string's address, then open
   "References To" (right-click → References → Show References to Address).
   This leads you to the function that passes `"VeteranInfantry"` to an INI
   read call — that is `CountryTypeClass::ReadINI` (or a wrapper).

3. **Find the RET / epilogue:** Scroll to the end of that function.  The
   last instruction before `RET` is your hook target.  Hook 5 or 6 bytes
   before `RET` with a DEFINE_HOOK that calls our INI extension, then
   returns 0 to let `RET` execute.

4. **Confirm the registers at that point:**
   - `ECX` should be `HouseTypeClass* pThis` (the `__thiscall` receiver).
   - The INI pointer `CCINIClass*` may be on the stack or in `EDX`.
     Check how the function's caller passes it.

5. **Update Body.h / Hooks.cpp:** Replace `0x507E27` with the real address.

---

## Hook 2 — SuperWeapon Recharge Timer Arming
**Purpose:** Multiply the recharge duration by `SuperWeapon.Ratio=` before
the countdown starts.

### Background
When a SW finishes firing, the engine arms a countdown timer
(`CDTimerClass::Start`) with the number of frames until the SW is ready
again.  That duration comes from `SuperWeaponTypeClass::RechargeTime`.
We multiply it by the country ratio at this moment.

### Steps

1. **Find CDTimerClass::Start:** Search for a virtual function on
   `CDTimerClass` that takes an int duration and sets an internal counter.
   It is very short — probably just stores the argument and sets a `Started`
   flag.  Note its address.

2. **Find callers inside SuperWeaponClass:** Use "Find References To" on
   `CDTimerClass::Start`.  Filter for callers that are inside a function
   that also accesses `SuperWeaponTypeClass::RechargeTime`.  That function
   is the recharge arm site.

3. **Identify the call instruction:** Within that function, find the specific
   `CALL CDTimerClass::Start` instruction.  The instruction *before* this
   call sets up the duration argument (loaded into `EDX` for `__fastcall`, or
   pushed onto the stack for `__cdecl`/`__stdcall`).

4. **Confirm the SuperWeaponClass pointer register:** At the call site, one
   register holds `SuperWeaponClass* pSW`.  This is how we reach the owner
   `HouseClass*`.  In many YR functions this ends up in `ESI` or `EBX`.

5. **Choose your hook byte count:** Hook at the instruction that loads the
   duration into the argument register, or at the CALL instruction itself.
   If you hook the CALL you can completely replace it and call Start yourself
   after applying the ratio; if you hook the load you can modify the register
   and let the original CALL proceed (Strategy A — preferred).

6. **Update Hooks.cpp:** Replace `0x6BCCF0` with the real address and
   confirm the register assignments match (`ESI` for pSW, `EDX` for
   duration) — or adjust the GET macros to match what you see.

### Cross-reference with Ares
If you have Ares source access, search for any hook that touches
`SuperWeaponClass` recharge logic.  Ares's AI-delay hooks for SWs are in
the same vicinity and will help confirm you're in the right function.

---

## Hook 3 — SW Availability Update (Inherit.SuperWeapon=)
**Purpose:** Force a SW's IsPresent flag to `true` for houses whose country
lists the SW in `Inherit.SuperWeapon=`.

### Background
The engine maintains SW availability by checking whether the house owns at
least one building that provides each SW.  This check is re-evaluated when
buildings are placed, sold, captured, or destroyed.  We inject a bypass
_before_ that ownership check.

Ares does the same thing for `SW.AlwaysGranted=yes`.  The ideal approach is
to hook the same callsite Ares uses.

### Steps

1. **Find SW availability evaluation:**  Search for xrefs to
   `SuperWeaponClass::IsPresent` (or the write to that field, depending on
   YRpp naming).  The function that writes it in a loop over all SWs is your
   target.

2. **Alternatively, trace from building placement:**  Find
   `BuildingClass::Place` or `BuildingClass::Unlimbo` and trace forward to
   wherever it calls an update that touches SW states.  That call chain leads
   to the availability evaluator.

3. **Locate the Ares AlwaysGranted hook:**  If you have Ares binary loaded
   alongside, Ares's hook for `AlwaysGranted` will appear as an injected
   trampoline at the start of (or inside) this function.  The address Ares
   jumps to is your target — hook the same location and chain after Ares.

4. **Identify what "availability" means in practice:**
   - In vanilla YR, a SW is "present" if the house owns a building with a
     matching `SuperWeapon=` tag.
   - The flag you're setting is what the sidebar and EVA system read.
   - Setting `IsPresent = true` and `IsReady = true` (or their equivalents)
     should be sufficient for the SW to appear and be usable.

5. **Register layout at the hook:**  You need both `HouseClass* pHouse` and
   `SuperWeaponClass* pSuper` in scope.  If the function takes `pHouse` as
   `ECX` (`__thiscall`) and iterates pSuper in `ESI` or `EBX`, document
   that and update the GET macros in the commented skeleton.

6. **Uncomment and update the skeleton** in Hooks.cpp, filling in:
   - The real hook address
   - The registers for `pHouse` and `pSuper`
   - The skip address (the instruction after the vanilla ownership check)

---

## After verification

For each hook, update the address literal in `Hooks.cpp` and remove the
stub comment.  Run a test build with a country that has `SuperWeapon.Ratio=2`
applied and verify the SW takes twice as long to recharge before enabling
the other two features.
