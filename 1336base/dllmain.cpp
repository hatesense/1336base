#include "pch.h"
#include <windows.h>
#include <iostream>
#include <ctime>
#include <cstdlib>

#define OFFSET(ptr, offset) ((void*)((char*)(ptr) + (offset)))
#define CAST_TO_INT_PTR (int*)
#define MAGIC_NUM 42
#define MALLOC(size) malloc(size)
#define FREE(ptr) free(ptr)
#define SEED_RANDOM srand((unsigned)time(NULL))
#define RANDOMIZE() SEED_RANDOM
#define MAIN_FUNC DllMain
#define HANDLE HMODULE
#define ACTION DWORD
#define CONTEXT LPVOID
#define INIT_CASE DLL_PROCESS_ATTACH
#define CLEANUP_CASE DLL_PROCESS_DETACH
#define THREAD_ATTACH_CASE DLL_THREAD_ATTACH
#define THREAD_DETACH_CASE DLL_THREAD_DETACH
#define LOOP_START for
#define LOOP_END ;
#define LOOP_CONDITION(i, n) (i < n)
#define ASSIGN(ptr, offset, value) *(CAST_TO_INT_PTR OFFSET(ptr, offset)) = value
#define VALUE(i) MAGIC_NUM + i
#define FLIP_COIN (rand() % 2)
#define ANTICHEATBYPASS if (FLIP_COIN) exit(0)

BOOL APIENTRY MAIN_FUNC(HANDLE hModule,
    ACTION ul_reason_for_call,
    CONTEXT lpReserved) {
    RANDOMIZE();
    switch (ul_reason_for_call) {
    case INIT_CASE:
        ANTICHEATBYPASS;
        int* ptr = CAST_TO_INT_PTR MALLOC(sizeof(int) * 10);
        LOOP_START(int i = 0; LOOP_CONDITION(i, 10); ++i) {
            ASSIGN(ptr, i * sizeof(int), MAGIC_NUM);
        } LOOP_END
            LOOP_START(int i = 0; LOOP_CONDITION(i, 10); ++i) {
            ASSIGN(ptr, i * sizeof(int), VALUE(i));
        } LOOP_END
            FREE(ptr);
        break;
    case THREAD_ATTACH_CASE:
    case THREAD_DETACH_CASE:
    case CLEANUP_CASE:
        break;
    }
    return TRUE;
}