#pragma once
#pragma pack(push, 1)
typedef struct _JMP_REL_SHORT
{
    UINT8  opcode;      // EB xx: JMP +2+xx
    UINT8  operand;
} JMP_REL_SHORT, * PJMP_REL_SHORT;
typedef struct _JMP_REL
{
    UINT8  opcode;      // E9/E8 xxxxxxxx: JMP/CALL +5+xxxxxxxx
    UINT32 operand;     // Relative destination address
} JMP_REL, * PJMP_REL, CALL_REL;
typedef struct _JMP_ABS
{
    UINT8  opcode0;     // FF25 00000000: JMP [+6]
    UINT8  opcode1;
    UINT32 dummy;
    UINT64 address;     // Absolute destination address
} JMP_ABS, * PJMP_ABS;

// 64-bit indirect absolute call.
typedef struct _CALL_ABS
{
    UINT8  opcode0;     // FF15 00000002: CALL [+6]
    UINT8  opcode1;
    UINT32 dummy0;
    UINT8  dummy1;      // EB 08:         JMP +10
    UINT8  dummy2;
    UINT64 address;     // Absolute destination address
} CALL_ABS;

// 32-bit direct relative conditional jumps.
typedef struct _JCC_REL
{
    UINT8  opcode0;     // 0F8* xxxxxxxx: J** +6+xxxxxxxx
    UINT8  opcode1;
    UINT32 operand;     // Relative destination address
} JCC_REL;

// 64bit indirect absolute conditional jumps that x64 lacks.
typedef struct _JCC_ABS
{
    UINT8  opcode;      // 7* 0E:         J** +16
    UINT8  dummy0;
    UINT8  dummy1;      // FF25 00000000: JMP [+6]
    UINT8  dummy2;
    UINT32 dummy3;
    UINT64 address;     // Absolute destination address
} JCC_ABS;

#pragma pack(pop)

typedef struct _TRAMPOLINE
{
    LPVOID pTarget;         // [In] Address of the target function.
    LPVOID pDetour;         // [In] Address of the detour function.
    LPVOID pTrampoline;     // [In] Buffer address for the trampoline and relay function.
    LPVOID pRelay;
    BOOL   patchAbove;      // [Out] Should use the hot patch area?
    UINT   nIP;             // [Out] Number of the instruction boundaries.
    UINT8  oldIPs[8];
    UINT8  newIPs[8];
} TRAMPOLINE, * PTRAMPOLINE;

BOOL createTrampolineFunction(PTRAMPOLINE ct);
