@echo off
if not exist ..\build mkdir ..\build
pushd ..\build
del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp

cl -nologo -Zi -FC ..\code\win32_platform.cpp /link User32.lib Gdiplus.lib gdi32.lib winmm.lib -incremental:no -opt:ref

del lock.tmp
del *.obj

popd