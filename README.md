# LuaMacros (working title)

This is the tool that I use for keyboard macros.

It can do the usual things you'd expect from this kind of tool (set hotkeys, press keys, run files...) but also a handful of things specific to my workflow:

- Set hotkeys to only trigger on a specific keyboard layout.  
  (for example, to add macros for entering language-specific characters)
- Set hotkeys to only trigger if an additional key is held.  
  (effectively allowing for custom modifier keys, aided by ability to remap keys via registry or [SharpKeys](https://github.com/randyrants/sharpkeys)).
- Mute/unmute default communications microphone

See [API](https://github.com/YellowAfterlife/LuaMacros/wiki/API) for all additional functions that the tool supports!

## How to

On startup, the tool will run `main.lua`, or, failing that, `examples/main.lua`.

That's where you put your code.

If you need to inspect output (e.g. for Lua errors or your `print` outputs),
you can do so by clicking on the application's icon in the tray area.

Application can be closed by closing the aforementioned output window.

## Examples

* `main.lua`: Basic example demonstrating macros and tray icon manipulation.
* `micmute/micmute.lua`: Returns a function to toggle mute for communications microphone.
* `yal.lua`: The macro script that I'm using on my end - has a handful more macros and advanced uses.

## History

* 2018: After switching to a keyboard without multimedia keys, I bind the ever-growing number of macros using an AutoIt script.
* 2019: At some point the AutoIt script grows unbearably convoluted (lack of proper objects nor inline functions doesn't help) and I rewrite the thing to be a [slightly less cursed-looking](https://user-images.githubusercontent.com/731492/123823227-aae30280-d905-11eb-988a-1ee394faad47.png) C++ program instead.  
  At some point I make the valuable addition of a system-wide microphone mute toggle key.
* 2021: I eventually grow tired of dealing with C++ and reorganize everything to be Lua-scriptable instead.

## Credits

- Tool by YellowAfterlife
- Licensed under [LGPLv3](https://www.gnu.org/licenses/lgpl-3.0.en.html)
- Uses [Lua](https://www.lua.org/) 5.4.3
- Uses a few icons from famfamfam's ["Silk"](http://www.famfamfam.com/lab/icons/silk/) set.