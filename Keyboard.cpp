#include "stdafx.h"
#include <vector>

static void setInputChar(INPUT* q, WORD k, bool up = false) {
	q->type = INPUT_KEYBOARD;
	q->ki.wVk = 0;
	q->ki.wScan = k;
	q->ki.time = 0;
	q->ki.dwFlags = (up ? KEYEVENTF_KEYUP : 0) | KEYEVENTF_UNICODE;
	q->ki.dwExtraInfo = NULL;
}

static int keyCheck(lua_State* q) {
	auto r = KeyCheck((int)lua_tointeger(q, 1));
	lua_pushboolean(q, r);
	return 1;
}
static int keyPress(lua_State* q) {
	std::vector<int> keys{};
	if (!luaM_args_to_int_vector(q, keys)) return 0;
	auto n = keys.size();

	std::vector<INPUT> inputs{}; inputs.resize(n);
	for (auto i = 0u; i < n; i++) setInputChar(&inputs[i], keys[i], false);
	SendInput(n, inputs.data(), sizeof(INPUT));
	return 0;
}
static int keyRelease(lua_State* q) {
	std::vector<int> keys{};
	if (!luaM_args_to_int_vector(q, keys)) return 0;
	auto n = keys.size();

	std::vector<INPUT> inputs{}; inputs.resize(n);
	for (auto i = 0u; i < n; i++) setInputChar(&inputs[i], keys[i], true);
	SendInput(n, inputs.data(), sizeof(INPUT));
	return 0;
}

static int getlayout(lua_State* q) {
	auto fgw = GetForegroundWindow();
	auto fgp = fgw ? GetWindowThreadProcessId(fgw, NULL) : 0;
	if (fgp) {
		lua_pushinteger(q, (int)(GetKeyboardLayout(fgp)));
	} else lua_pushnil(q);
	return 1;
}

void init_keyboard_lualib(lua_State* q) {
	lua_newtable(q);
	luaM_rawset_str_cfunc(q, "check", keyCheck);
	luaM_rawset_str_cfunc(q, "press", keyPress);
	luaM_rawset_str_cfunc(q, "release", keyRelease);
	luaM_rawset_str_cfunc(q, "getlayout", getlayout);
	lua_setglobal(q, "keyboard");
}