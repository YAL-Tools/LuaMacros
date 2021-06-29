#include "stdafx.h"
#include <vector>
void trace(const char* format, ...) {
	static CHAR buf[1025];
	va_list argList;
	va_start(argList, format);
	int n = wvsprintfA(buf, format, argList);
	va_end(argList);

	auto len = strlen(buf);
	buf[len] = '\n';
	buf[++len] = 0;

	printf("%s", buf);
	OutputDebugStringA(buf);
}
void traceW(const wchar_t* format, ...) {
	static WCHAR buf[1024];
	va_list argList;
	va_start(argList, format);
	int n = wvsprintfW(buf, format, argList);
	va_end(argList);

	auto len = lstrlenW(buf);
	buf[len] = '\n';
	buf[++len] = 0;

	wprintf(L"%s", buf);
	OutputDebugStringW(buf);
}

std::string luaM_get_cwd(lua_State* q) {
	lua_Debug info;
	lua_getinfo(q, "S", &info);
	return info.source;
}
void luaM_setglobal_cfunc(lua_State* q, const char* name, lua_CFunction fn) {
	lua_pushcfunction(q, fn);
	lua_setglobal(q, name);
}

void luaM_rawset_str_cfunc(lua_State* q, const char* field, lua_CFunction fn) {
	lua_pushstring(q, field);
	lua_pushcfunction(q, fn);
	lua_rawset(q, -3);
}
void luaM_rawset_str_bool(lua_State* q, const char* field, bool val) {
	lua_pushstring(q, field);
	lua_pushboolean(q, val);
	lua_rawset(q, -3);
}
void luaM_rawset_str_int(lua_State* q, const char* field, lua_Integer val) {
	lua_pushstring(q, field);
	lua_pushinteger(q, val);
	lua_rawset(q, -3);
}

int luaM_rawget_str_int(lua_State* q, const char* field, int def) {
	lua_pushstring(q, field);
	lua_rawget(q, -2);
	int isnum = 0;
	int r = (int)lua_tointegerx(q, -1, &isnum);
	lua_pop(q, 1);
	return isnum ? r : def;
}

void luaM_procStatus(lua_State* L, int status) {
	if (status != LUA_OK) {
		std::string text = lua_tostring(L, -1);
		luaL_traceback(L, L, text.c_str(), 0);
		text = lua_tostring(L, -1);
		text = "Error: " + text;
		lua_pop(L, 2);
		trace("%s", text.c_str());
	}
}
void luaM_dumpStack(lua_State* L, const char* header) {
	if (header) trace("%s:", header);
	int top = lua_gettop(L);
	static std::string s{};
	for (int i = 1; i <= top; i++) {
		s = std::to_string(i);
		s += "\t";
		s += luaL_typename(L, i);
		s += "\t";
		switch (lua_type(L, i)) {
			case LUA_TNUMBER:
				s += std::to_string(lua_tonumber(L, i));
				break;
			case LUA_TSTRING:
				s += lua_tostring(L, i);
				break;
			case LUA_TBOOLEAN:
				s += lua_toboolean(L, i) ? "true" : "false";
				break;
			case LUA_TNIL:
				s += "nil";
				break;
			default:
				s += std::to_string((long long)lua_topointer(L, i));
				break;
		}
		trace("%s", s.c_str());
	}
}

bool luaM_args_to_int_vector(lua_State* q, std::vector<int>& keys) {
	auto n = lua_gettop(q);
	keys.resize(n);
	for (int i = 1; i <= n; i++) {
		int ok = 0;
		int key = (int)lua_tointegerx(q, i, &ok);
		if (ok) {
			keys[i - 1] = key;
		} else {
			luaL_typeerror(q, i, "index");
			return false;
		}
	}
	return true;
}