@echo off
mkdir build
copy .\lib\*.dll .\build\
pushd build
cl /I \INCLUDE /I..\inc\ /Zi ..\src\main.c ..\src\rx.c ..\src\gui.c ..\src\complex.c ..\src\dsp.c Shell32.lib ..\lib\SDL2.lib ..\lib\SDL2main.lib ..\lib\glew32.lib /link /subsystem:CONSOLE
popd
