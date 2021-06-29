#include "stdafx.h"

static void setInputKey(INPUT* q, WORD vk, bool up = false) {
	q->type = INPUT_KEYBOARD;
	q->ki.wVk = vk;
	q->ki.wScan = 0;
	q->ki.time = 0;
	q->ki.dwFlags = up ? KEYEVENTF_KEYUP : 0;
	q->ki.dwExtraInfo = NULL;
}
static void setInputChar(INPUT* q, WORD k, bool up = false) {
	q->type = INPUT_KEYBOARD;
	q->ki.wVk = 0;
	q->ki.wScan = k;
	q->ki.time = 0;
	q->ki.dwFlags = (up ? KEYEVENTF_KEYUP : 0) | KEYEVENTF_UNICODE;
	q->ki.dwExtraInfo = NULL;
}

static int sleepTime = 5;

static int sendKeys(lua_State* q) {
	std::vector<int> keys{};
	if (!luaM_args_to_int_vector(q, keys)) return 0;
	auto n = keys.size();

	std::vector<INPUT> inputs{}; inputs.resize(n);
	for (auto i = 0u; i < n; i++) setInputKey(&inputs[i], keys[i], false);
	SendInput(n, inputs.data(), sizeof(INPUT));

	Sleep(sleepTime);
	for (auto i = 0u; i < keys.size(); i++) inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(n, inputs.data(), sizeof(INPUT));
	return 0;
}

static int sendChars(lua_State* q) {
	std::vector<int> keys{};
	if (!luaM_args_to_int_vector(q, keys)) return 0;
	auto n = keys.size();

	std::vector<INPUT> inputs{}; inputs.resize(n);
	for (auto i = 0u; i < n; i++) setInputChar(&inputs[i], keys[i], false);
	SendInput(n, inputs.data(), sizeof(INPUT));

	Sleep(sleepTime);
	for (auto i = 0u; i < keys.size(); i++) inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(n, inputs.data(), sizeof(INPUT));
	return 0;
}

void init_send_lualib(lua_State* q) {
	lua_newtable(q);
	luaM_rawset_str_cfunc(q, "keys", sendKeys);
	luaM_rawset_str_cfunc(q, "char", sendChars);
	lua_setglobal(q, "send");
	luaM_procStatus(q, luaL_dostring(q, R"lua(--
send.text = function(str, delay)
local codes = {}
delay = delay or 5
for p, c in utf8.codes(str) do
    send.char(c)
    if (delay > 0) then sleep(delay) end
end
end)lua"));
}

