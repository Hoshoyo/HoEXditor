@echo off
setlocal EnableDelayedExpansion

set build_type=debug

set floc=..\..\src\

set exename=hoex

rem set files=%floc%main.c %floc%text.c %floc%memory.c %floc%util.c %floc%font_rendering.c %floc%editor.c %floc%text_manager.c %floc%text_events.c %floc%os_dependent.c %floc%interface.c %floc%interface_events.c %floc%input.c %floc%console.c %floc%dialog.c
set files=main.c text.c memory.c util.c font_rendering.c editor.c text_manager.c text_events.c os_dependent.c interface.c interface_events.c input.c console.c dialog.c
set object_files=
set src_files=

set argCount=0
for %%x in (%*) do (
set /A argCount+=1
set "argVec[!argCount!]=%%~x"
)
for /L %%i in (1,1,%argCount%) do (
rem echo %%i- "!argVec[%%i]!"
set src_files=!src_files!!argVec[%%i]!
)
set /a insert=0

for %%A in (%files%) do (
for /L %%i in (1,1,%argCount%) do (
if !argVec[%%i]! == %%A (
set /a insert=1
)
)
if not !insert! == 1 (
set str=%%A
set str=!str:.c=.obj!
set str=!str:%floc%=!

set object_files=!object_files!!str!
)
set /a insert=0
)

set src_with_path=
for %%A in (%src_files%) do set src_with_path=!src_with_path!%floc%%%A

set include_dirs= -I..\..\include -Isrc
set link_libraries_release= kernel32.lib libcmt.lib libvcruntime.lib libucrt.lib libcpmt.lib opengl32.lib user32.lib gdi32.lib shell32.lib
set link_libraries_debug= kernel32.lib libcmtd.lib libvcruntimed.lib libucrtd.lib libcpmtd.lib opengl32.lib user32.lib gdi32.lib shell32.lib

set compiler_flags_release= /O2 /Zi /nologo /Fe%exename%.exe
set compiler_flags_debug= /Od /Zi /nologo /Fe%exename%.exe

set linker_flags_release= /MACHINE:X64 /incremental:no /NODEFAULTLIB
set linker_flags_debug= /MACHINE:X64 /incremental:no /NODEFAULTLIB

if NOT EXIST bin mkdir bin
pushd bin
if NOT EXIST %build_type% mkdir %build_type%
pushd %build_type%

rem compile stb_truetype once
if not exist stb_truetype.obj (
cl /c /Zi /O2 /nologo ..\..\include\stb_truetype.c /link /NODEFAULTLIB
)
rem compile stb_image once
if not exist stb_image.obj (
cl /c /Zi /O2 /nologo ..\..\include\stb_image.c /link /NODEFAULTLIB
)

if %build_type% == release (
rem call ml64 /c /Cx /nologo %floc%asm\copy_mem.asm
call cl %compiler_flags_release% %include_dirs% %src_with_path% /link %linker_flags_release% %link_libraries_release% copy_mem.obj stb_truetype.obj stb_image.obj %object_files%
)

if %build_type% == debug (
rem call ml64 /c /Cx /nologo /Zi %floc%asm\copy_mem.asm
call cl %compiler_flags_debug% %include_dirs% %src_with_path% /link %linker_flags_debug% %link_libraries_debug% copy_mem.obj stb_truetype.obj stb_image.obj %object_files%
)

popd
popd

:eof