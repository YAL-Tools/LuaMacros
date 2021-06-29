#include "stdafx.h"
#include <string>
namespace LuaAPI {
    static int luaB_print(lua_State* L) {
        int n = lua_gettop(L);  /* number of arguments */
        int i;
        static std::string out{};
        out = "";
        for (i = 1; i <= n; i++) {  /* for each argument */
            size_t l;
            const char* s = luaL_tolstring(L, i, &l);  /* convert it to string */
            if (i > 1) out.append("\t");
            out.append(s, l);
            lua_pop(L, 1);  /* pop result */
        }
        out.append("\n");

        static std::wstring wide{};
        auto len = MultiByteToWideChar(CP_UTF8, 0, out.c_str(), -1, nullptr, 0);
        wide.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, out.c_str(), -1, (wchar_t*)wide.data(), len);
        wprintf(L"%s", wide.c_str());
        OutputDebugStringW(wide.c_str());
        return 0;
    }
    static int _sleep(lua_State* L) {
        Sleep((int)lua_tointeger(L, 1));
        lua_pop(L, 1);
        return 0;
    }
    void init(lua_State* q);
}

#define lua_namespaces \
    X(init_vk_lualib)\
    X(init_mouse_lualib)\
    X(init_send_lualib)\
    X(init_keyboard_lualib)\
    X(init_hotkey_lualib)\
    X(init_micmute_lualib)\
    X(init_trayicon_lualib)

#define X(name) void name(lua_State* q);
lua_namespaces
#undef X

void LuaAPI::init(lua_State* q) {
    luaM_setglobal_cfunc(q, "print", luaB_print);
    luaM_setglobal_cfunc(q, "sleep", _sleep);
    #define X(name) name(q);
    lua_namespaces
    #undef X
}
#undef lua_namespaces