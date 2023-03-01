#include "stdafx.h"
static int gettext(lua_State* q) {
	// Try opening the clipboard
	do {
		if (!OpenClipboard(nullptr)) break;

		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData == nullptr) break;

		const char* text = static_cast<char*>(GlobalLock(hData));
		if (text == nullptr) break;
		
		lua_pushstring(q, text);

		GlobalUnlock(hData);

		CloseClipboard();

		return 1;
	} while (false);
	lua_pushnil(q);
	return 1;
}
void init_clipboard_lualib(lua_State* q) {
	lua_newtable(q);
	luaM_rawset_str_cfunc(q, "gettext", gettext);
	lua_setglobal(q, "clipboard");
}