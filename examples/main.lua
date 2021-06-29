--[[
	Allows to press Alt+Q instead of Alt+Left to go back
]]
hotkey.add{
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
	mods = MOD_ALT,
	key = VK_PRIOR,
	func = function()
		send.keys(VK_VOLUME_UP)
	end
}
hotkey.add{
	mods = MOD_ALT,
	key = VK_NEXT,
	func = function()
		send.keys(VK_VOLUME_DOWN)
	end
}

--[[
	This is how you do things with tray icon:
]]
local icon = tray.loadicon("emoticon_smile.ico")
tray.seticon(icon)