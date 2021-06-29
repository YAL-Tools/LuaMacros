del /Q LuaMacros.zip
cmd /C 7z a LuaMacros.zip examples
cd Release
cmd /C 7z a ..\LuaMacros.zip LuaMacros.exe
pause