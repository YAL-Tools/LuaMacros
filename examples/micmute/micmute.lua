--[[
	Returns you a function to toggle mute for Communications microphone.
	Also shows mute status on the tray icon!
	
	Used like
	hotkey.add{
		mods = MOD_ALT,
		key = VK_F1,
		func = dofile("examples/micmute/micmute.lua")
	}
]]
local icon = {
	muted = tray.loadicon("muted.ico"),
	unmuted = tray.loadicon("unmuted.ico"),
}

local function sync(val)
	if (val == nil) then val = micmute.get() end
	tray.seticon(val and icon.muted or icon.unmuted)
end
sync()

return function()
	local r = micmute.toggle()
	if (r ~= nil) then sync(r) end
end