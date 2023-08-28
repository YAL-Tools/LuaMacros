#include "stdafx.h"
#include <vector>
#include <map>
#include <string>
static int keyboardLayout;

struct hotkey;
struct modKey {
	std::vector<UINT> keys{};
	bool pressed;
	std::vector<hotkey*> hotkeys{};
	bool check() {
		for (auto& kc : keys) {
			if (!KeyCheck(kc)) return false;
		}
		return true;
	}
};
static std::vector<modKey*> modKeys;

struct layoutData {
	int layout;
	std::vector<hotkey*> hotkeys{};
};
static std::map<int, layoutData*> layoutMap;

namespace HotkeyRef {
	constexpr const char* metaName = "Hotkey";
	struct Impl {
		int id;
		const char* name;
	};

	static Impl* cast(lua_State* L, int index) {
		luaL_checktype(L, index, LUA_TUSERDATA);
		auto v = (Impl*)luaL_checkudata(L, index, metaName);
		if (v == nullptr) luaL_typeerror(L, index, metaName);
		return v;
	}
}

struct hotkey {
	int id = 0;
	std::vector<UINT> mkKeys{};
	modKey* mkRef = nullptr;
	UINT mods = 0;
	UINT key = 0;
	int layout = 0;
	layoutData* layoutRef = nullptr;
	HotkeyRef::Impl* ref = nullptr;
	bool bound = false;
	bool enabled = true;
	bool exists = true;
	
	void bind() {
		if (bound) return;
		bound = RegisterHotKey(0, id, mods, key);
		if (!bound) {
			auto hr = GetLastError();
			auto name = ref ? ref->name : nullptr;
			if (name == nullptr) name = "?";
			auto extra = "";
			if (hr == 0x581) {
				extra = " (already registered)";
			}
			trace("Failed to bind hotkey \"%s\" (id=%d, mods=0x%x, key=0x%x), hresult=0x%x%s",
				name, id, mods, key, hr, extra);
		} else {
			//trace("Bound hotkey %d for vk=0x%x", id, key);
		}
	}
	void unbind() {
		if (!bound) return;
		//trace("Unbinding hotkey %d for vk=0x%x", id, key);
		UnregisterHotKey(0, id);
		bound = false;
	}
	void autoBind() {
		//trace("layout %x/%x refp=%d", layout, keyboardLayout, mkRef ? mkRef->pressed : -1);
		if (enabled
			&& (layout == 0 || keyboardLayout == layout)
			&& (mkRef == nullptr || mkRef->pressed)
		) {
			bind();
		} else unbind();
	}

	void reset() {
		bound = false;
		enabled = true;
		exists = true;
	}
	void remove() {
		exists = false;
		enabled = false;
		unbind();
		if (mkRef) {
			auto& hotkeys = mkRef->hotkeys;
			for (auto i = 0u; i < hotkeys.size(); i++) {
				if (hotkeys[i] == this) {
					hotkeys.erase(hotkeys.begin() + i);
					break;
				}
			}
			mkRef = nullptr;
		}
		if (layoutRef) {
			auto& hotkeys = layoutRef->hotkeys;
			for (auto i = 0u; i < hotkeys.size(); i++) {
				if (hotkeys[i] == this) {
					hotkeys.erase(hotkeys.begin() + i);
					break;
				}
			}
			layoutRef = nullptr;
		}
		if (ref) {
			ref->id = 0;
			ref = nullptr;
		}
	}
};
static std::vector<hotkey*> hotkeys;

static hotkey* resolve(lua_State* q, int index) {
	auto hkr = HotkeyRef::cast(q, 1);
	if (!hkr || hkr->id == 0) {
		return nullptr;
	}
	return hotkeys[hkr->id - 1];
}

namespace HotkeyRef {
	static Impl* push(lua_State* L, int id, const char* name) {
		auto v = (Impl*)lua_newuserdata(L, sizeof(Impl));
		v->id = id;
		v->name = _strdup(name ? name : "?");
		luaL_getmetatable(L, metaName);
		lua_setmetatable(L, -2);
		return v;
	}

	static int gc(lua_State* L) {
		auto v = cast(L, 1);
		if (!v) return 0;
		if (v->name) {
			trace("Hotkey(id=%d, name=\"%s\") has been recycled.", v->id, v->name);
			v->id = 0;
			free((void*)v->name);
			v->name = nullptr;
		}
		return 0;
	}

	static int tostring(lua_State* L) {
		auto v = cast(L, 1);
		if (!v) return 0;
		lua_pushfstring(L, "Hotkey(id=%I, name=\"%s\")", (lua_Integer)v->id, v->name);
		return 1;
	}

	static int destroy(lua_State* q) {
		auto hk = resolve(q, 1);
		if (hk && hk->exists) {
			hk->remove();
			lua_pushboolean(q, true);
		} else lua_pushboolean(q, false);
		return 1;
	}

	static int getenabled(lua_State* q) {
		auto hk = resolve(q, 1);
		if (hk) {
			lua_pushboolean(q, hk->enabled);
		} else lua_pushnil(q);
		return 1;
	}
	static int setenabled(lua_State* q) {
		auto hk = resolve(q, 1);
		if (hk) {
			hk->enabled = lua_toboolean(q, 2);
			hk->autoBind();
			lua_pushboolean(q, hk->enabled);
		} else lua_pushnil(q);
		return 1;
	}
	static int getactive(lua_State* q) {
		auto hk = resolve(q, 1);
		if (hk) {
			lua_pushboolean(q, hk->bound);
		} else lua_pushnil(q);
		return 1;
	}

	luaL_Reg metaRegs[] = {
		{"__gc", gc},
		{"__tostring", tostring},
		{"remove", destroy},
		{"getactive", getactive},
		{"getenabled", getenabled},
		{"setenabled", setenabled},
		{nullptr, nullptr}
	};
	static void init(lua_State* L) {
		// set up the metatable:
		luaL_newmetatable(L, metaName);
		luaL_setfuncs(L, metaRegs, 0);
		lua_pop(L, 1);
	}
}

static void updateKeyboardLayout() {
	auto lastLayout = keyboardLayout;
	auto fgw = GetForegroundWindow();
	auto fgp = fgw ? GetWindowThreadProcessId(fgw, NULL) : 0;
	keyboardLayout = fgp ? (unsigned long long)(GetKeyboardLayout(fgp)) >> 16 : 0;
		
	if (keyboardLayout != lastLayout) {
		auto oldPair = layoutMap.find(lastLayout);
		if (oldPair != layoutMap.end()) {
			for (auto hk : oldPair->second->hotkeys) hk->autoBind();
		}
		//trace("layout %x", keyboardLayout);
		auto newPair = layoutMap.find(keyboardLayout);
		if (newPair != layoutMap.end()) {
			for (auto hk : newPair->second->hotkeys) hk->autoBind();
		}
	}
}
void update_hotkeys() {
	updateKeyboardLayout();

	for (auto& mk : modKeys) {
		auto wasPressed = mk->pressed;
		mk->pressed = mk->check();
		if (wasPressed != mk->pressed) {
			for (auto hk : mk->hotkeys) hk->autoBind();
		}
	}
}
void handle_hotkey_message(lua_State *q, MSG& msg) {
	auto hk = (int)msg.wParam;
	lua_getglobal(q, "__hotkey_funcs");
	lua_rawgeti(q, -1, hk);
	if (lua_isfunction(q, -1)) {
		lua_newtable(q);
		luaM_rawset_str_bool(q, "alt", (msg.lParam & MOD_ALT) != 0);
		luaM_rawset_str_bool(q, "control", (msg.lParam & MOD_CONTROL) != 0);
		luaM_rawset_str_bool(q, "shift", (msg.lParam & MOD_SHIFT) != 0);
		luaM_rawset_str_bool(q, "win", (msg.lParam & MOD_WIN) != 0);
		luaM_procStatus(q, lua_pcall(q, 1, 0, 0));
		lua_pop(q, 1);
	} else {
		lua_pop(q, 2);
	}
}

static int add(lua_State* q) {
	hotkey* hk = nullptr;
	for (auto hki : hotkeys) {
		if (!hki->exists) {
			hk = hki;
			hk->reset();
			break;
		}
	}
	if (hk == nullptr) {
		hk = new hotkey();
		hk->id = (int)hotkeys.size() + 1;
		hotkeys.push_back(hk);
	}
	//
	lua_pushstring(q, "modkey");
	lua_rawget(q, -2);
	if (lua_istable(q, -1)) {
		auto n = lua_rawlen(q, -1);
		for (lua_Unsigned i = 1; i <= n; i++) {
			lua_pushinteger(q, (lua_Integer)i);
			lua_rawget(q, -2);
			int isnum = 0;
			int mk = lua_tointegerx(q, -1, &isnum);
			if (isnum) hk->mkKeys.push_back(mk);
			lua_pop(q, 1);
		}
		lua_pop(q, 1);
	} else {
		lua_pop(q, 1);
		auto mk = (UINT)luaM_rawget_str_int(q, "modkey", 0);
		if (mk) hk->mkKeys.push_back(mk);
	}
	hk->mods = (UINT)luaM_rawget_str_int(q, "mods", 0);
	hk->key = (UINT)luaM_rawget_str_int(q, "key", 0);
	hk->layout = luaM_rawget_str_int(q, "layout", 0);

	if (hk->layout != 0) {
		layoutData* lr;
		auto lp = layoutMap.find(hk->layout);
		if (lp == layoutMap.end()) {
			lr = new layoutData();
			lr->layout = hk->layout;
			layoutMap[hk->layout] = lr;
		} else lr = lp->second;
		lr->hotkeys.push_back(hk);
		hk->layoutRef = lr;
	}

	if (!hk->mkKeys.empty()) {
		modKey* mk = nullptr;
		for (auto mki : modKeys) {
			if (mki->keys == hk->mkKeys) { mk = mki; break; }
		}
		if (mk == nullptr) {
			mk = new modKey();
			mk->keys = hk->mkKeys;
			mk->pressed = mk->check();
			modKeys.push_back(mk);
		}
		hk->mkRef = mk;
		mk->hotkeys.push_back(hk);
	} else hk->mkRef = nullptr;

	// store the function so that it doesn't expire:
	lua_getglobal(q, "__hotkey_funcs"); // [props, __hotkey_funcs]
	lua_pushstring(q, "func"); // [props, __hotkey_funcs, "func"]
	lua_rawget(q, -3); // [props, __hotkey_funcs, <func>]
	lua_rawseti(q, -2, hk->id); // [props, __hotkey_funcs]

	lua_pushstring(q, "name");
	lua_rawget(q, 1);
	auto name = lua_tostring(q, -1);

	hk->ref = HotkeyRef::push(q, hk->id, name);

	// store the ref so that our pointer doesn't expire:
	lua_getglobal(q, "__hotkey_refs");
	lua_pushvalue(q, -2);
	lua_rawseti(q, -2, hk->id);
	lua_pop(q, 1);

	hk->autoBind();

	return 1;
}
static int remove(lua_State* q) {
	auto hk = resolve(q, 1);
	if (hk && hk->exists) {
		hk->remove();
		lua_pushboolean(q, true);
	} else lua_pushboolean(q, false);
	return 1;
}
static int bind(lua_State* q) {
	auto hk = resolve(q, 1);
	if (hk) {
		hk->bind();
	}
	return 0;
}
static int unbind(lua_State* q) {
	auto hk = resolve(q, 1);
	if (hk) {
		hk->unbind();
	}
	return 0;
}

void init_hotkey_lualib(lua_State* q) {
	HotkeyRef::init(q);

	lua_newtable(q);
	lua_setglobal(q, "__hotkey_funcs");

	lua_newtable(q);
	lua_setglobal(q, "__hotkey_refs");

	lua_newtable(q);
	luaM_rawset_str_cfunc(q, "add", add);
	luaM_rawset_str_cfunc(q, "remove", remove);
	luaM_rawset_str_cfunc(q, "bind", bind);
	luaM_rawset_str_cfunc(q, "unbind", unbind);
	lua_setglobal(q, "hotkey");
}