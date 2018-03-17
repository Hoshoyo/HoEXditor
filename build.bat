@echo off

set build_type=debug

set floc=..\..\src\

set exename=hoex

set files=/Tp %floc%main.cpp

set include_dirs= -I..\..\include -Isrc
set link_libraries_release= kernel32.lib libcmt.lib libvcruntime.lib libucrt.lib libcpmt.lib opengl32.lib user32.lib gdi32.lib shell32.lib Comdlg32.lib ../../lib/freetype.lib
set link_libraries_debug= kernel32.lib libcmtd.lib libvcruntimed.lib libucrtd.lib libcpmtd.lib opengl32.lib user32.lib gdi32.lib shell32.lib Comdlg32.lib ../../lib/freetype.lib

set compiler_flags_release= /Oi /O2 /Zi /nologo /Fe%exename%.exe
set compiler_flags_debug= /Oi /Od /Zi /nologo /Fe%exename%.exe

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
	if not exist copy_mem.obj (
		call ml64 /c /Cx /nologo %floc%asm\copy_mem.asm
	)
	call cl %compiler_flags_release% %include_dirs% %files% /link %linker_flags_release% %link_libraries_release% copy_mem.obj stb_image.obj
)

if %build_type% == debug (
	if not exist copy_mem.obj (
		call ml64 /c /Cx /nologo /Zi %floc%asm\copy_mem.asm
	)
	call cl %compiler_flags_debug% %include_dirs% %files% /link %linker_flags_debug% %link_libraries_debug% copy_mem.obj stb_image.obj
)

popd
popd

:eof
