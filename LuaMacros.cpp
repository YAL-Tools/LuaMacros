#include <objbase.h>
#include "stdafx.h"
#include "LuaMacros.h"
#include <direct.h>

namespace LuaAPI {
    void init(lua_State* q);
}
namespace LuaMacros {
    HINSTANCE hInstance;
    HWND consoleWindow;
}

void init_console_window();
void cleanup_trayicon_lualib();
static void bye() {
    trace("bye!");
    cleanup_trayicon_lualib();
}

void update_hotkeys();
void handle_hotkey_message(lua_State* q, MSG& msg);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow
) {
    LuaMacros::hInstance = hInstance;
    init_console_window();
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    auto co = CoInitialize(NULL);

    wchar_t wd[FILENAME_MAX + 1];
    if (_wgetcwd(wd, FILENAME_MAX)) {
        traceW(L"cwd: %s", wd);
    }
    trace("You can click on the tray icon to show/hide this console window!");

    auto state = luaL_newstate();
    luaL_openlibs(state);
    LuaAPI::init(state);
    trace("Loading main.lua...");
    auto attrib = GetFileAttributesW(L"main.lua");
    if (attrib != INVALID_FILE_ATTRIBUTES) {
        luaM_procStatus(state, luaL_dofile(state, "main.lua"));
    } else {
        trace("main.lua is missing, running example instead...");
        luaM_procStatus(state, luaL_dofile(state, "examples/main.lua"));
    }

    bool active = true;
    MSG msg{};
    while (active) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            //trace(L"msg -> %x", msg.message);
            if (msg.message == WM_QUIT) {
                active = false;
                break;
            }
            if (msg.message == WM_HOTKEY) {
                handle_hotkey_message(state, msg);
            }
        }
        if (!active) break;
        //
        lua_getglobal(state, "update");
        if (lua_isfunction(state, -1)) {
            luaM_procStatus(state, lua_pcall(state, 0, 0, 0));
        } else lua_pop(state, 1);
        //
        update_hotkeys();
        //
        Sleep(30);
    }
    bye();
    CoUninitialize();
    return (int)msg.wParam;
}


