--[[
	This is a macro set that I'm personally using.
	Demonstrates various macro uses.
]]
print("hello!");

--[[
	At some point it occurred to me that I don't need Caps Lock
	so it is now a conveniently large system mute toggle key.
]]
mute = hotkey.add{
	name = "ToggleMute",
	key = VK_F16,
	func = dofile("examples/micmute/micmute.lua")
}

--[[
	Allows to press Alt+Q instead of Alt+Left to go back
]]
hotkey.add{
	name = "Back",
	mods = MOD_ALT,
	key = VK_Q,
	func = function()
		send.keys(VK_MENU, VK_LEFT)
	end
}

--[[
	These allow to press Alt+PageUp/Alt+PageDown for volume controls
]]
hotkey.add{
	name = "VolUp",
	mods = MOD_ALT,
	key = VK_PRIOR,
	func = function()
		send.keys(VK_VOLUME_UP)
	end
}
hotkey.add{
	name = "VolDown",
	mods = MOD_ALT,
	key = VK_NEXT,
	func = function()
		send.keys(VK_VOLUME_DOWN)
	end
}

--[[
	If your mouse has too many additional buttons,
	you could map a few of them to scroll on repeat
]]
hotkey.add{ name="ScrollUp", key = VK_F20, func = function() mouse.scroll(-3) end }
hotkey.add{ name="ScrollDown", key = VK_F21, func = function() mouse.scroll(3) end }


--[[
	The modifier key used for the rest of the macros here.
	I have my Right Alt mapped to be F13 instead.
]]
local myMod = VK_F13

--[[
	The following bunch of macros allow to enter both Russian and Ukrainian symbols
	on a Russian keyboard layout, sparing me of guessing how many times I need to press
	Ctrl+Shift to get to the English layout or looking at the tray area for it.
	
	Similar can be applied to any language pairs that share most symbols.
	
	Layout codes can be found at
	https://docs.microsoft.com/en-us/windows-hardware/manufacture/desktop/windows-language-pack-default-values
]]
local function addModSendRu(key, lower, upper)
	for i = 0, 1 do
		local c = i > 0 and upper or lower
		hotkey.add{
			name = "MultiKey(\"" .. lower .. "\",\"" .. upper .. "\")",
			mods = MOD_SHIFT * i,
			layout = 0x419,
			modkey = myMod,
			key = key,
			func = function()
				send.text(c)
			end
		}
	end
end
addModSendRu(VK_S, "і", "І") -- ы
addModSendRu(VK_OEM_3, "`", "~")
addModSendRu(VK_OEM_6, "ї", "Ї") -- [
addModSendRu(VK_OEM_7, "є", "Є") -- '
addModSendRu(VK_8, "₽", "₴")
addModSendRu(VK_2, "’", "́")

--[[
	A couple additional symbols for fast entry
]]
local function addModSendText(opt, text)
	opt.modkey = myMod
	opt.func = function()
		send.text(text)
	end
	hotkey.add(opt)
end
addModSendText({ key = VK_OEM_MINUS }, "—")
addModSendText({ key = VK_OEM_PLUS }, "·")
addModSendText({ key = VK_OEM_COMMA }, "≤")
addModSendText({ key = VK_OEM_PERIOD }, "≥")
addModSendText({ key = VK_OEM_5 }, "➜")

--[[
	These are mostly mapping Alt+<key> combinations that
	are desirable to work with Right Alt too
]]
function addModSendKeys(opt, ...)
	local keys = {...}
	opt.modkey = myMod
	opt.func = function()
		send.keys(table.unpack(keys))
	end
	hotkey.add(opt)
end
addModSendKeys({ key = VK_LEFT }, VK_MENU, VK_LEFT)
addModSendKeys({ key = VK_RIGHT }, VK_MENU, VK_RIGHT)
addModSendKeys({ key = VK_SNAPSHOT }, VK_MENU, VK_SNAPSHOT)
