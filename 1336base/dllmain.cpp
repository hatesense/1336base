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
#define DISABLEANTICHEATBYPASS if (FLIP_COIN) exit(0)
#define DISABLEANTICHEAT 10
#define THREAD_WORKING int
#define THREAD_SIZE sizeof

BOOL APIENTRY MAIN_FUNC(HANDLE hModule,
    ACTION ul_reason_for_call,
    CONTEXT lpReserved) {
    RANDOMIZE();
    switch (ul_reason_for_call) {
    case INIT_CASE:
        ANTICHEATBYPASS;
        THREAD_WORKING* ptr = CAST_TO_INT_PTR MALLOC(THREAD_SIZE(THREAD_WORKING) * DISABLEANTICHEAT);
        LOOP_START(THREAD_WORKING i = 0; LOOP_CONDITION(i, DISABLEANTICHEAT); ++i) {
            ASSIGN(ptr, i * THREAD_SIZE(THREAD_WORKING), MAGIC_NUM);
        } LOOP_END
            LOOP_START(THREAD_WORKING i = 0; LOOP_CONDITION(i, DISABLEANTICHEAT); ++i) {
            ASSIGN(ptr, i * THREAD_SIZE(THREAD_WORKING), VALUE(i));
        } LOOP_END
            FREE(ptr);
        break;
    case THREAD_ATTACH_CASE:
    case THREAD_DETACH_CASE:
    case CLEANUP_CASE:
        DISABLEANTICHEATBYPASS;
        break;
    }
    return TRUE;
}