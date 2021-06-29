#pragma once
#include <SDKDDKVer.h>
#include <Windows.h>
#include <string>
#include <vector>

extern "C" {
	#include "Lua/lua.h"
	#include "Lua/lualib.h"
	#include "Lua/lauxlib.h"
}

extern CHAR trace_bufA[1024];
extern WCHAR trace_bufW[1024];
void trace(const char* format, ...);
void traceW(const wchar_t* format, ...);

#define KeyCheck(key) ((GetAsyncKeyState(key) & ~1) != 0)

std::string luaM_get_cwd(lua_State* q);
void luaM_setglobal_cfunc(lua_State* q, const char* name, lua_CFunction fn);
void luaM_rawset_str_cfunc(lua_State* q, const char* field, lua_CFunction fn);
void luaM_rawset_str_bool(lua_State* q, const char* field, bool val);

int luaM_rawget_str_int(lua_State* q, const char* field, int def = 0);

void luaM_procStatus(lua_State* L, int status);
void luaM_dumpStack(lua_State* L, const char* header = nullptr);
bool luaM_args_to_int_vector(lua_State* q, std::vector<int>& keys);
