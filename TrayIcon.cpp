#include "stdafx.h"
#include "LuaMacros.h"
#include "StringConv.h"
constexpr int CWM_TRAY = (WM_APP + 1);
static INT_PTR CALLBACK TrayDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == CWM_TRAY) {
		//trace(L"TRAY! %d %d", wParam, lParam);
		switch (lParam) {
			case WM_LBUTTONUP:
				if (IsWindowVisible(LuaMacros::consoleWindow)) {
					ShowWindow(LuaMacros::consoleWindow, SW_HIDE);
				} else {
					ShowWindow(LuaMacros::consoleWindow, SW_SHOW);
				}
				break;
		}
	}
	return 0;
}
static HWND trayWindow{};
static NOTIFYICONDATAW trayIconData{};
static HICON defaultIcon;

namespace LuaHICON {
	constexpr const char* metaName = "HICON";
	struct Impl {
		HICON icon;
		const char* name;
	};

	static Impl* cast(lua_State* L, int index) {
		luaL_checktype(L, index, LUA_TUSERDATA);
		auto v = (Impl*)luaL_checkudata(L, index, metaName);
		if (v == nullptr) luaL_typeerror(L, index, metaName);
		return v;
	}

	static int push(lua_State* L, HICON icon, const char* name) {
		auto v = (Impl*)lua_newuserdata(L, sizeof(Impl));
		v->icon = icon;
		v->name = _strdup(name);
		luaL_getmetatable(L, metaName);
		lua_setmetatable(L, -2);
		return 1;
	}

	static int gc(lua_State* L) {
		auto v = cast(L, 1);
		if (!v) return 0;
		printf("Destroyed Icon(%d)\n", (int)v->icon);
		if (v->icon != 0) {
			DestroyIcon(v->icon);
			free((void*)v->name);
			v->icon = 0;
		}
		return 0;
	}

	static int tostring(lua_State* L) {
		auto v = cast(L, 1);
		if (!v) return 0;
		lua_pushfstring(L, "HICON(h=%I, name=\"%s\")", (lua_Integer)v->icon, v->name);
		return 1;
	}

	static int load(lua_State* L) {
		auto path = lua_tostring(L, 1);
		auto wpath = StringConv.proc(path);
		HICON icon;
		if (path) {
			icon = (HICON)LoadImageW(NULL, wpath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE|LR_DEFAULTSIZE);
		} else icon = NULL;
		if (icon) {
			push(L, icon, path);
		} else lua_pushnil(L);
		return 1;
	}

	luaL_Reg metaRegs[] = {
		{"__gc", gc},
		{"__tostring", tostring},
		{nullptr, nullptr}
	};
	static void init(lua_State* L) {
		// set up the metatable:
		luaL_newmetatable(L, metaName);
		luaL_setfuncs(L, metaRegs, 0);
		lua_pop(L, 1);
	}
}

static void initIcon() {
	trayIconData = {};
	trayIconData.uVersion = NOTIFYICON_VERSION_4;
	trayIconData.cbSize = sizeof(trayIconData);
	trayIconData.uID = 1;
	trayIconData.hWnd = trayWindow;
	trayIconData.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE;
	trayIconData.uCallbackMessage = CWM_TRAY;
	wcscpy_s(trayIconData.szTip, L"LuaMacros\n(Click to toggle console)");
	trayIconData.hIcon = defaultIcon;
	Shell_NotifyIconW(NIM_ADD, &trayIconData);
}

static int seticon(lua_State* q) {
	auto icon = LuaHICON::cast(q, 1);
	if (!icon) {
		lua_pushboolean(q, false);
		return 1;
	}
	lua_getglobal(q, "tray");
	lua_pushstring(q, "__icon");
	lua_pushvalue(q, -3);
	lua_rawset(q, -3);
	lua_pop(q, 1);

	trayIconData.hIcon = icon->icon;
	lua_pushboolean(q, Shell_NotifyIconW(NIM_MODIFY, &trayIconData));
	return 1;
}
static int settext(lua_State* q) {
	auto text = lua_tostring(q, 1);
	if (text) {
		wcscpy_s(trayIconData.szTip, StringConv.proc(text));
		lua_pushboolean(q, Shell_NotifyIconW(NIM_MODIFY, &trayIconData));
	} else lua_pushboolean(q, false);
	return 1;
}

void init_trayicon_lualib(lua_State* q) {
	defaultIcon = (HICON)GetClassLongPtr(LuaMacros::consoleWindow, GCL_HICON);
	trayWindow = CreateDialogW(LuaMacros::hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, TrayDlgProc);
	initIcon();

	LuaHICON::init(q);
	lua_newtable(q);
	luaM_rawset_str_cfunc(q, "__loadicon", LuaHICON::load);
	luaM_rawset_str_cfunc(q, "seticon", seticon);
	luaM_rawset_str_cfunc(q, "settext", settext);
	lua_setglobal(q, "tray");
	luaL_dostring(q, R"lua(
tray.loadicon = function(path)
-- relative path?
if not (path:match("^/") or path:match("^.:") or path:match("^\\")) then
	local base = debug.getinfo(2, "S").source:sub(2)
	local dir = base:match("(.*[/\\])")
	if dir then
		--print(dir, path)
		path = dir .. path
	end
end
return tray.__loadicon(path)
end
)lua");
}
void cleanup_trayicon_lualib() {
	Shell_NotifyIconW(NIM_DELETE, &trayIconData);
}