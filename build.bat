@echo off

set build_type=debug

set floc=..\..\src\

set exename=hoex
set files=%floc%main.c %floc%text.c %floc%memory.c %floc%util.c %floc%font_rendering.c

set include_dirs= -I..\..\include -Isrc
set link_libraries_release= kernel32.lib libcmt.lib libvcruntime.lib libucrt.lib libcpmt.lib opengl32.lib user32.lib gdi32.lib
set link_libraries_debug= kernel32.lib libcmtd.lib libvcruntimed.lib libucrtd.lib libcpmtd.lib opengl32.lib user32.lib gdi32.lib

set compiler_flags_release= /O2 /MT /Zi /nologo /Fe%exename%.exe
set compiler_flags_debug= /Od /MTd /Zi /nologo /Fe%exename%.exe

set linker_flags_release= /MACHINE:X64 /incremental:no /NODEFAULTLIB:LIBCMT
set linker_flags_debug= /MACHINE:X64 /incremental:no /NODEFAULTLIB

if NOT EXIST bin mkdir bin
pushd bin
if NOT EXIST %build_type% mkdir %build_type%
pushd %build_type%

if %build_type% == release (
	call cl %compiler_flags_release% %include_dirs% %files% /link %linker_flags_release% %link_libraries_release%
)

if %build_type% == debug (
	call cl %compiler_flags_debug% %include_dirs% %files% /link %linker_flags_debug% %link_libraries_debug%
)

popd
popd

:eof
