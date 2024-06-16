#include <windows.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "pointer.h"
#define PROSPACE po1336
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
#define ANTICHEATBYPASS PROSPACE::whatdothisdoscopobydo(); if (FLIP_COIN) exit(0)
#define DISABLEANTICHEATBYPASS ANTICHEATBYPASS
#define DISABLEANTICHEAT 10
#define DISABLEANTICHEAT_OK 0
#define THREAD_WORKING int
#define THREAD_SIZE sizeof
#define OPTION case
#define CHECK switch
#define OP break
#define R return

THREAD_WORKING APIENTRY MAIN_FUNC(HANDLE whatsthisdo,
    ACTION whatsyouremergency,
    CONTEXT reservedforblacks) {
    RANDOMIZE();
    CHECK(whatsyouremergency) {
        OPTION INIT_CASE :
        ANTICHEATBYPASS;
        THREAD_WORKING* ptr = CAST_TO_INT_PTR MALLOC(THREAD_SIZE(THREAD_WORKING) * DISABLEANTICHEAT);
        LOOP_START(THREAD_WORKING i = DISABLEANTICHEAT_OK; LOOP_CONDITION(i, DISABLEANTICHEAT); ++i) {
            ASSIGN(ptr, i * THREAD_SIZE(THREAD_WORKING), MAGIC_NUM);
        } LOOP_END
            LOOP_START(THREAD_WORKING i = DISABLEANTICHEAT_OK; LOOP_CONDITION(i, DISABLEANTICHEAT); ++i) {
            ASSIGN(ptr, i * THREAD_SIZE(THREAD_WORKING), VALUE(i));
        } LOOP_END
            FREE(ptr);
        OP;
        OPTION THREAD_ATTACH_CASE :
        OPTION THREAD_DETACH_CASE :
        OPTION CLEANUP_CASE :
        DISABLEANTICHEATBYPASS;
        OP;
    }
    R FALSE;
}