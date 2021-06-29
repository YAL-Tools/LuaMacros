#include "stdafx.h"
static void setInputWheel(INPUT* q, int d, bool h) {
	q->type = INPUT_MOUSE;
	q->mi.dwFlags = (h ? MOUSEEVENTF_HWHEEL : MOUSEEVENTF_WHEEL);
	q->mi.mouseData = d;
	q->mi.dx = 0;
	q->mi.dy = 0;
	q->mi.time = 0;
}
static void setInputMouseButton(INPUT* q, int mf) {
	q->type = INPUT_MOUSE;
	q->mi.dwFlags = mf;
	q->mi.dx = 0;
	q->mi.dy = 0;
	q->mi.time = 0;
	q->mi.mouseData = 0;
}

static int getpos(lua_State* q) {
	POINT p{};
	if (GetCursorPos(&p)) {
		lua_pushinteger(q, p.x);
		lua_pushinteger(q, p.y);
		return 2;
	} else return 0;
}

static int setpos(lua_State* q) {
	bool ok = SetCursorPos((int)lua_tointeger(q, 1), (int)lua_tointeger(q, 2));
	lua_pushboolean(q, ok);
	return 1;
}

static int check(lua_State* q) {
	UINT vk = 0;
	switch (lua_tointeger(q, 1)) {
		case 1: vk = VK_LBUTTON; break;
		case 2: vk = VK_RBUTTON; break;
		case 3: vk = VK_MBUTTON; break;
		case 4: vk = VK_XBUTTON1; break;
		case 5: vk = VK_XBUTTON2; break;
	}
	lua_pushboolean(q, KeyCheck(vk));
	return 1;
}

// (amt, hor = false)
static int scroll(lua_State* q) {
	auto amt = (int)(lua_tonumber(q, 1) * WHEEL_DELTA);
	auto hwheel = lua_gettop(q) > 1 && lua_toboolean(q, 2);
	INPUT input{};
	setInputWheel(&input, amt, hwheel);
	SendInput(1, &input, sizeof(INPUT));
	return 0;
}

// (btn)
static int press(lua_State* q) {
	int flag;
	switch (lua_tointeger(q, 1)) {
		case 1: flag = MOUSEEVENTF_LEFTDOWN; break;
		case 2: flag = MOUSEEVENTF_RIGHTDOWN; break;
		case 3: flag = MOUSEEVENTF_MIDDLEDOWN; break;
		default: return 0;
	}
	INPUT input{};
	setInputMouseButton(&input, flag);
	SendInput(1, &input, sizeof(INPUT));
	return 0;
}

// (btn)
static int release(lua_State* q) {
	int flag;
	switch (lua_tointeger(q, 1)) {
		case 1: flag = MOUSEEVENTF_LEFTUP; break;
		case 2: flag = MOUSEEVENTF_RIGHTUP; break;
		case 3: flag = MOUSEEVENTF_MIDDLEUP; break;
		default: return 0;
	}
	INPUT input{};
	setInputMouseButton(&input, flag);
	SendInput(1, &input, sizeof(INPUT));
	return 0;
}

void init_mouse_lualib(lua_State* q) {
	lua_newtable(q);
	luaM_rawset_str_cfunc(q, "getpos", getpos);
	luaM_rawset_str_cfunc(q, "setpos", setpos);
	luaM_rawset_str_cfunc(q, "check", check);
	luaM_rawset_str_cfunc(q, "press", press);
	luaM_rawset_str_cfunc(q, "release", release);
	luaM_rawset_str_cfunc(q, "scroll", scroll);
	lua_setglobal(q, "mouse");
}