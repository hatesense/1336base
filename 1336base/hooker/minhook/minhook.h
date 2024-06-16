#pragma once

#include <windows.h>

enum hookStatus : int {
    MH_UNKNOWN = -1,
    MH_OK = 0,
    MH_ERROR_ALREADY_INITIALIZED,
    MH_ERROR_NOT_INITIALIZED,
    MH_ERROR_ALREADY_CREATED,
    MH_ERROR_NOT_CREATED,
    MH_ERROR_ENABLED,
    MH_ERROR_DISABLED,
    MH_ERROR_EXECUTABLE_NOT,
    MH_ERROR_UNSUPPORTED_FUNCTION,
    MH_ERROR_MEMORY_ALLOC,
    MH_ERROR_MEMORY_PROTECT,
    MH_ERROR_MODULE_NOT_FOUND,
    MH_ERROR_NOT_FOUND_FUNCTION
};

namespace minhook {
    hookStatus WINAPI initialize();
    hookStatus WINAPI uninitialize();
    hookStatus WINAPI createHook(void* target, void* detour, void** og);
    hookStatus WINAPI removeHook(void* pTarget);
    hookStatus WINAPI enableHook(void* pTarget);
    hookStatus WINAPI disableHook(void* pTarget);
    hookStatus WINAPI queueEnableHook(void* pTarget = 0);
    hookStatus WINAPI queueDisableHook(void* pTarget = 0);
    hookStatus WINAPI applyQueued();
    const char* WINAPI statusToString(hookStatus status);
}
