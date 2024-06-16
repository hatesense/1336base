#include "../typesforblacks.h"
#include <tlhelp32.h>
#include <limits.h>

#include "minhook.h"
#include "buffer.h"
#include "trampoline.h"


#define INITIAL_HOOK_CAPACITY   32
#define INITIAL_THREAD_CAPACITY 128
#define INVALID_HOOK_POS UINT_MAX
#define ALL_HOOKS_POS    UINT_MAX
#define ACTION_DISABLE      0
#define ACTION_ENABLE       1
#define ACTION_APPLY_QUEUED 2
#define THREAD_ACCESS \
    (THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION | THREAD_SET_CONTEXT)

struct hookEntry {
    LPVOID m_target;
    LPVOID m_detour;
    LPVOID m_trampoline;
    u8 m_backup[8];
    u8 m_patchAbove : 1;
    u8 m_isEnabled : 1;
    u8 m_queueEnable : 1;
    u32 m_iP : 4;
    u8 m_oldIps[8];
    u8 m_newIps[8];
};
struct frozenThreads {
    LPDWORD m_items;
    u32 m_capacity;
    u32 m_size;
};
volatile LONG g_isLocked{};
HANDLE g_heap{};
struct mHook {
    hookEntry* m_items;
    u32 m_capacity;
    u32 m_size;
};
mHook g_hooks;
static u32 findHookEntry(void* target) {
    for (u32 i{}; i < g_hooks.m_size; ++i) {
        if ((ULONG_PTR)target == (ULONG_PTR)g_hooks.m_items[i].m_target) {
            return i;
        }
    }

    return INVALID_HOOK_POS;
}
static hookEntry* addHookEntry() {
    if (g_hooks.m_items == NULL) {
        g_hooks.m_capacity = INITIAL_HOOK_CAPACITY;
        g_hooks.m_items = (hookEntry*)HeapAlloc(g_heap, 0, g_hooks.m_capacity * sizeof(hookEntry));
        if (g_hooks.m_items == NULL) {
            return NULL;
        }
    }
    else if (g_hooks.m_size >= g_hooks.m_capacity) {
        auto p{ (hookEntry*)HeapReAlloc(g_heap, 0, g_hooks.m_items, (g_hooks.m_capacity * 2) * sizeof(hookEntry)) };
        if (p == NULL) {
            return NULL;
        }
        g_hooks.m_capacity *= 2;
        g_hooks.m_items = p;
    }
    return &g_hooks.m_items[g_hooks.m_size++];
}
static void deleteHookEntry(u32 pos) {
    if (pos < g_hooks.m_size - 1) {
        g_hooks.m_items[pos] = g_hooks.m_items[g_hooks.m_size - 1];
    }
    g_hooks.m_size--;
    if (g_hooks.m_capacity / 2 >= INITIAL_HOOK_CAPACITY && g_hooks.m_capacity / 2 >= g_hooks.m_size) {
        auto p{ (hookEntry*)HeapReAlloc(g_heap, 0, g_hooks.m_items, (g_hooks.m_capacity / 2) * sizeof(hookEntry)) };
        if (p == NULL) {
            return;
        }
        g_hooks.m_capacity /= 2;
        g_hooks.m_items = p;
    }
}
static DWORD_PTR findOldIP(hookEntry* hook, DWORD_PTR ip) {
    if (hook->m_patchAbove && ip == ((DWORD_PTR)hook->m_target - sizeof(JMP_REL))) {
        return (DWORD_PTR)hook->m_target;
    }
    for (u32 i{}; i < hook->m_iP; ++i) {
        if (ip == ((DWORD_PTR)hook->m_trampoline + hook->m_newIps[i])) {
            return (DWORD_PTR)hook->m_target + hook->m_oldIps[i];
        }
    }
    if (ip == (DWORD_PTR)hook->m_detour) {
        return (DWORD_PTR)hook->m_target;
    }
    return 0;
}
static DWORD_PTR findNewIP(hookEntry* hook, DWORD_PTR ip) {
    for (u32 i{}; i < hook->m_iP; ++i) {
        if (ip == ((DWORD_PTR)hook->m_target + hook->m_oldIps[i])) {
            return (DWORD_PTR)hook->m_trampoline + hook->m_newIps[i];
        }
    }
    return 0;
}
static void processThreadIPs(HANDLE thread, UINT pos, UINT action) {
    CONTEXT c;
    DWORD64* ipC{ &c.Rip };
    u32 count;
    c.ContextFlags = CONTEXT_CONTROL;
    if (!GetThreadContext(thread, &c)) {
        return;
    }
    if (pos == ALL_HOOKS_POS) {
        pos = 0;
        count = g_hooks.m_size;
    }
    else {
        count = pos + 1;
    }
    for (; pos < count; ++pos) {
        hookEntry* hook{ &g_hooks.m_items[pos] };
        bool enable{};
        DWORD_PTR ip;

        switch (action) {
        case ACTION_DISABLE:
            enable = FALSE;
            break;
        case ACTION_ENABLE:
            enable = TRUE;
            break;
        default:
            enable = hook->m_queueEnable;
            break;
        }
        if (hook->m_isEnabled == enable) {
            continue;
        }
        if (enable) {
            ip = findNewIP(hook, *ipC);
        }
        else {
            ip = findOldIP(hook, *ipC);
        }
        if (ip != 0) {
            *ipC = ip;
            SetThreadContext(thread, &c);
        }
    }
}
static bool enumerateThreads(frozenThreads* threads) {
    bool succeeded{};
    HANDLE s32{ CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0) };
    if (s32 != INVALID_HANDLE_VALUE) {
        THREADENTRY32 te;
        te.dwSize = sizeof(THREADENTRY32);
        if (Thread32First(s32, &te)) {
            succeeded = true;
            do {
                if (te.dwSize >= (FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(DWORD)) && te.th32OwnerProcessID == GetCurrentProcessId() && te.th32ThreadID != GetCurrentThreadId()) {
                    if (threads->m_items == NULL) {
                        threads->m_capacity = INITIAL_THREAD_CAPACITY;
                        threads->m_items = (LPDWORD)HeapAlloc(g_heap, 0, threads->m_capacity * sizeof(DWORD));
                        if (threads->m_items == NULL) {
                            succeeded = false;
                            break;
                        }
                    }
                    else if (threads->m_size >= threads->m_capacity) {
                        threads->m_capacity *= 2;
                        auto p{ (LPDWORD)HeapReAlloc(g_heap, 0, threads->m_items, threads->m_capacity * sizeof(DWORD)) };
                        if (p == NULL) {
                            succeeded = false;
                            break;
                        }
                        threads->m_items = p;
                    }
                    threads->m_items[threads->m_size++] = te.th32ThreadID;
                }
                te.dwSize = sizeof(THREADENTRY32);
            } while (Thread32Next(s32, &te));
            if (succeeded && GetLastError() != ERROR_NO_MORE_FILES) {
                succeeded = false;
            }
            if (!succeeded && threads->m_items != NULL) {
                HeapFree(g_heap, 0, threads->m_items);
                threads->m_items = NULL;
            }
        }
        CloseHandle(s32);
    }
    return succeeded;
}
static hookStatus freeze(frozenThreads* threads, UINT pos, UINT action) {
    hookStatus status{};
    threads->m_items = NULL;
    threads->m_capacity = 0;
    threads->m_size = 0;
    if (!enumerateThreads(threads)) {
        status = MH_ERROR_MEMORY_ALLOC;
    }
    else if (threads->m_items != NULL) {
        for (u32 i{}; i < threads->m_size; ++i) {
            HANDLE thread{ OpenThread(THREAD_ACCESS, FALSE, threads->m_items[i]) };
            if (thread != NULL) {
                SuspendThread(thread);
                processThreadIPs(thread, pos, action);
                CloseHandle(thread);
            }
        }
    }
    return status;
}
static void unfreeze(frozenThreads* threads) {
    if (threads->m_items != NULL) {
        for (u32 i{}; i < threads->m_size; ++i) {
            HANDLE thread{ OpenThread(THREAD_ACCESS, FALSE, threads->m_items[i]) };
            if (thread != NULL) {
                ResumeThread(thread);
                CloseHandle(thread);
            }
        }
        HeapFree(g_heap, 0, threads->m_items);
    }
}
static hookStatus enableHookLL(u32 pos, bool enable) {
    hookEntry* hook{ &g_hooks.m_items[pos] };
    DWORD oldProtect{};
    SIZE_T patchSize{ sizeof(JMP_REL) };
    LPBYTE patchTarget{ (LPBYTE)hook->m_target };
    if (hook->m_patchAbove) {
        patchTarget -= sizeof(JMP_REL);
        patchSize += sizeof(JMP_REL_SHORT);
    }
    if (!VirtualProtect(patchTarget, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return MH_ERROR_MEMORY_PROTECT;
    }
    if (enable) {
        PJMP_REL jmp{ (PJMP_REL)patchTarget };
        jmp->opcode = 0xE9;
        jmp->operand = (u32)((LPBYTE)hook->m_detour - (patchTarget + sizeof(JMP_REL)));
        if (hook->m_patchAbove) {
            PJMP_REL_SHORT shortJmp{ (PJMP_REL_SHORT)hook->m_target };
            shortJmp->opcode = 0xEB;
            shortJmp->operand = (u8)(0 - (sizeof(JMP_REL_SHORT) + sizeof(JMP_REL)));
        }
    }
    else {
        if (hook->m_patchAbove) {
            memcpy(patchTarget, hook->m_backup, sizeof(JMP_REL) + sizeof(JMP_REL_SHORT));
        }
        else {
            memcpy(patchTarget, hook->m_backup, sizeof(JMP_REL));
        }
    }
    VirtualProtect(patchTarget, patchSize, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), patchTarget, patchSize);
    hook->m_isEnabled = enable;
    hook->m_queueEnable = enable;
    return MH_OK;
}
static hookStatus enableAllHooksLL(bool enable) {
    hookStatus status{};
    u32 first{ INVALID_HOOK_POS };
    for (u32 i{}; i < g_hooks.m_size; ++i) {
        if (g_hooks.m_items[i].m_isEnabled != enable) {
            first = i;
            break;
        }
    }
    if (first != INVALID_HOOK_POS) {
        frozenThreads threads;
        status = freeze(&threads, ALL_HOOKS_POS, enable ? ACTION_ENABLE : ACTION_DISABLE);
        if (status == MH_OK) {
            for (u32 i{ first }; i < g_hooks.m_size; ++i) {
                if (g_hooks.m_items[i].m_isEnabled != enable) {
                    status = enableHookLL(i, enable);
                    if (status != MH_OK) {
                        break;
                    }
                }
            }
            unfreeze(&threads);
        }
    }
    return status;
}
static void enterSpinLock(void) {
    u32 spinCount{};
    while (InterlockedCompareExchange(&g_isLocked, TRUE, FALSE) != FALSE) {
        if (spinCount < 32) {
            Sleep(0);
        }
        else {
            Sleep(1);
        }
        spinCount++;
    }
}
static void leaveSpinLock(void) {
    InterlockedExchange(&g_isLocked, false);
}
namespace minhook {
    hookStatus WINAPI initialize() {
        hookStatus status{};
        enterSpinLock();
        if (g_heap == NULL) {
            g_heap = HeapCreate(0, 0, 0);
            if (g_heap != NULL) {
                InitializeBuffer();
            }
            else {
                status = MH_ERROR_MEMORY_ALLOC;
            }
        }
        else {
            status = MH_ERROR_ALREADY_INITIALIZED;
        }
        leaveSpinLock();
        return status;
    }
    hookStatus WINAPI uninitialize() {
        hookStatus status{};
        enterSpinLock();
        if (g_heap != 0) {
            status = enableAllHooksLL(false);
            if (status == MH_OK) {
                UninitializeBuffer();
                HeapFree(g_heap, 0, g_hooks.m_items);
                HeapDestroy(g_heap);
                g_heap = 0;
                g_hooks.m_items = 0;
                g_hooks.m_capacity = 0;
                g_hooks.m_size = 0;
            }
        }
        else {
            status = MH_ERROR_NOT_INITIALIZED;
        }
        leaveSpinLock();
        return status;
    }
    hookStatus WINAPI createHook(void* target, void* detour, void** og) {
        hookStatus status{};
        enterSpinLock();
        if (g_heap != NULL) {
            if (IsExecutableAddress(target) && IsExecutableAddress(detour)) {
                u32 pos{ findHookEntry(target) };
                if (pos == INVALID_HOOK_POS) {
                    void* buffer{ AllocateBuffer(target) };
                    if (buffer != NULL) {
                        TRAMPOLINE ct;
                        ct.pTarget = target;
                        ct.pDetour = detour;
                        ct.pTrampoline = buffer;
                        if (createTrampolineFunction(&ct)) {
                            hookEntry* hook{ addHookEntry() };
                            if (hook != NULL) {
                                hook->m_target = ct.pTarget;
                                hook->m_detour = ct.pRelay;
                                hook->m_trampoline = ct.pTrampoline;
                                hook->m_patchAbove = ct.patchAbove;
                                hook->m_isEnabled = FALSE;
                                hook->m_queueEnable = FALSE;
                                hook->m_iP = ct.nIP;
                                memcpy(hook->m_oldIps, ct.oldIPs, ARRAYSIZE(ct.oldIPs));
                                memcpy(hook->m_newIps, ct.newIPs, ARRAYSIZE(ct.newIPs));
                                if (ct.patchAbove) {
                                    memcpy(hook->m_backup, (LPBYTE)target - sizeof(JMP_REL), sizeof(JMP_REL) + sizeof(JMP_REL_SHORT));
                                }
                                else {
                                    memcpy(hook->m_backup, target, sizeof(JMP_REL));
                                }
                                if (og != NULL) {
                                    *og = hook->m_trampoline;
                                }
                            }
                            else {
                                status = MH_ERROR_MEMORY_ALLOC;
                            }
                        }
                        else {
                            status = MH_ERROR_UNSUPPORTED_FUNCTION;
                        }
                        if (status != MH_OK) {
                            FreeBuffer(buffer);
                        }
                    }
                    else {
                        status = MH_ERROR_MEMORY_ALLOC;
                    }
                }
                else {
                    status = MH_ERROR_ALREADY_CREATED;
                }
            }
            else {
                status = MH_ERROR_EXECUTABLE_NOT;
            }
        }
        else {
            status = MH_ERROR_NOT_INITIALIZED;
        }
        leaveSpinLock();
        return status;
    }
    hookStatus WINAPI removeHook(void* target) {
        hookStatus status{};
        enterSpinLock();
        if (g_heap != NULL) {
            u32 pos{ findHookEntry(target) };
            if (pos != INVALID_HOOK_POS) {
                if (g_hooks.m_items[pos].m_isEnabled) {
                    frozenThreads threads;
                    status = freeze(&threads, pos, ACTION_DISABLE);
                    if (status == MH_OK) {
                        status = enableHookLL(pos, FALSE);
                        unfreeze(&threads);
                    }
                }
                if (status == MH_OK) {
                    FreeBuffer(g_hooks.m_items[pos].m_trampoline);
                    deleteHookEntry(pos);
                }
            }
            else {
                status = MH_ERROR_NOT_CREATED;
            }
        }
        else {
            status = MH_ERROR_NOT_INITIALIZED;
        }
        leaveSpinLock();
        return status;
    }
    static hookStatus enableHook(void* target, bool enable) {
        hookStatus status{};
        enterSpinLock();
        if (g_heap != 0) {
            if (target == 0) {
                status = enableAllHooksLL(enable);
            }
            else {
                u32 pos{ findHookEntry(target) };
                if (pos != INVALID_HOOK_POS) {
                    if (g_hooks.m_items[pos].m_isEnabled != enable) {
                        frozenThreads threads;
                        status = freeze(&threads, pos, ACTION_ENABLE);
                        if (status == MH_OK) {
                            status = enableHookLL(pos, enable);
                            unfreeze(&threads);
                        }
                    }
                    else {
                        status = enable ? MH_ERROR_ENABLED : MH_ERROR_DISABLED;
                    }
                }
                else {
                    status = MH_ERROR_NOT_CREATED;
                }
            }
        }
        else {
            status = MH_ERROR_NOT_INITIALIZED;
        }
        leaveSpinLock();
        return status;
    }
    hookStatus WINAPI enableHook(void* target) {
        return enableHook(target, true);
    }
    hookStatus WINAPI disableHook(void* target) {
        return enableHook(target, false);
    }
    static hookStatus queueHook(void* target, bool queueEnable) {
        hookStatus status{};
        enterSpinLock();
        if (g_heap != 0) {
            if (target == 0) {
                for (u32 i{}; i < g_hooks.m_size; ++i) {
                    g_hooks.m_items[i].m_queueEnable = queueEnable;
                }
            }
            else {
                u32 pos{ findHookEntry(target) };
                if (pos != INVALID_HOOK_POS) {
                    g_hooks.m_items[pos].m_queueEnable = queueEnable;
                }
                else {
                    status = MH_ERROR_NOT_CREATED;
                }
            }
        }
        else {
            status = MH_ERROR_NOT_INITIALIZED;
        }
        leaveSpinLock();
        return status;
    }
    hookStatus WINAPI queueEnableHook(void* target) {
        return queueHook(target, true);
    }
    hookStatus WINAPI queueDisableHook(void* target) {
        return queueHook(target, false);
    }
    hookStatus WINAPI applyQueued() {
        hookStatus status{};
        UINT first{ INVALID_HOOK_POS };
        enterSpinLock();
        if (g_heap != 0) {
            for (u32 i{}; i < g_hooks.m_size; ++i) {
                if (g_hooks.m_items[i].m_isEnabled != g_hooks.m_items[i].m_queueEnable) {
                    first = i;
                    break;
                }
            }
            if (first != INVALID_HOOK_POS) {
                frozenThreads threads;
                status = freeze(&threads, ALL_HOOKS_POS, ACTION_APPLY_QUEUED);
                if (status == MH_OK) {
                    for (u32 i{ first }; i < g_hooks.m_size; ++i) {
                        hookEntry* hook{ &g_hooks.m_items[i] };
                        if (hook->m_isEnabled != hook->m_queueEnable) {
                            status = enableHookLL(i, hook->m_queueEnable);
                            if (status != MH_OK) {
                                break;
                            }
                        }
                    }
                    unfreeze(&threads);
                }
            }
        }
        else {
            status = MH_ERROR_NOT_INITIALIZED;
        }
        leaveSpinLock();
        return status;
    }
    const char* WINAPI statusToString(hookStatus status) {
#define MH_ST2STR(x)    \
    case x:             \
        return #x;

        switch (status) {
            MH_ST2STR(MH_UNKNOWN)
                MH_ST2STR(MH_OK)
                MH_ST2STR(MH_ERROR_ALREADY_INITIALIZED)
                MH_ST2STR(MH_ERROR_NOT_INITIALIZED)
                MH_ST2STR(MH_ERROR_ALREADY_CREATED)
                MH_ST2STR(MH_ERROR_NOT_CREATED)
                MH_ST2STR(MH_ERROR_ENABLED)
                MH_ST2STR(MH_ERROR_DISABLED)
                MH_ST2STR(MH_ERROR_EXECUTABLE_NOT)
                MH_ST2STR(MH_ERROR_UNSUPPORTED_FUNCTION)
                MH_ST2STR(MH_ERROR_MEMORY_ALLOC)
                MH_ST2STR(MH_ERROR_MEMORY_PROTECT)
                MH_ST2STR(MH_ERROR_MODULE_NOT_FOUND)
                MH_ST2STR(MH_ERROR_NOT_FOUND_FUNCTION)
        }

#undef MH_ST2STR
        return "(unknown)";
    }
}