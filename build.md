# Fluidsynth build notes for headless CLAP plugin

[Fluidsynth Build Notes](https://github.com/FluidSynth/fluidsynth/wiki/BuildingWithCMake)

## Win32/MSVC/jsmk

* need [vcpkg](https://github.com/microsoft/vcpkg) for glib, etc.
    * builds deposited into its `buildtrees` subdir (`downloads`, `installed`, `packages` too)

`vkpkg install fluidsynth:x64-windows`

produced .../vcpkg/packages/fluidsynth_x64-windows/share/fluidsynth/lib/fluidsynth.lib

produced .../vcpkg/packages/fluidsynth_x64-windows/share/fluidsynth/bin/libfluidsynth-3.dll
with dependencies (via ldd):

```
glib-2.0-0.dll => /g/gstreamer/1.0/msvc_x86_64/bin/glib-2.0-0.dll (0x7ffdfe6d0000)
intl-8.dll => /g/gstreamer/1.0/msvc_x86_64/bin/intl-8.dll (0x7ffe1bfd0000)     
pcre2-8-0.dll => /g/gstreamer/1.0/msvc_x86_64/bin/pcre2-8-0.dll (0x7ffe10090000)

ntdll.dll => /c/WINDOWS/SYSTEM32/ntdll.dll (0x7ffe238d0000)
KERNEL32.DLL => /c/WINDOWS/System32/KERNEL32.DLL (0x7ffe22e10000)
KERNELBASE.dll => /c/WINDOWS/System32/KERNELBASE.dll (0x7ffe21260000)
msvcrt.dll => /c/WINDOWS/System32/msvcrt.dll (0x7ffe21cb0000)
ole32.dll => /c/WINDOWS/System32/ole32.dll (0x7ffe216b0000)
msvcp_win.dll => /c/WINDOWS/System32/msvcp_win.dll (0x7ffe21610000)
DSOUND.dll => /c/WINDOWS/SYSTEM32/DSOUND.dll (0x7ffdc4b60000)
ucrtbase.dll => /c/WINDOWS/System32/ucrtbase.dll (0x7ffe20f30000)
advapi32.dll => /c/WINDOWS/System32/advapi32.dll (0x7ffe232d0000)
ResampleDmo.DLL => /c/WINDOWS/SYSTEM32/ResampleDmo.DLL (0x7ffdc4aa0000)        
sechost.dll => /c/WINDOWS/System32/sechost.dll (0x7ffe21b30000)
GDI32.dll => /c/WINDOWS/System32/GDI32.dll (0x7ffe22410000)
OLEAUT32.dll => /c/WINDOWS/System32/OLEAUT32.dll (0x7ffe231f0000)
RPCRT4.dll => /c/WINDOWS/System32/RPCRT4.dll (0x7ffe219c0000)
win32u.dll => /c/WINDOWS/System32/win32u.dll (0x7ffe21050000)
combase.dll => /c/WINDOWS/System32/combase.dll (0x7ffe234a0000)
gdi32full.dll => /c/WINDOWS/System32/gdi32full.dll (0x7ffe20e10000)
USER32.dll => /c/WINDOWS/System32/USER32.dll (0x7ffe22fc0000)
powrprof.dll => /c/WINDOWS/SYSTEM32/powrprof.dll (0x7ffe20540000)
winmmbase.dll => /c/WINDOWS/SYSTEM32/winmmbase.dll (0x7ffe14050000)
powrprof.dll => /c/Windows/System32/powrprof.dll (0x192758c0000)
WS2_32.dll => /c/WINDOWS/System32/WS2_32.dll (0x7ffe22380000)
msdmo.dll => /c/WINDOWS/SYSTEM32/msdmo.dll (0x7ffe199d0000)
WINMM.dll => /c/WINDOWS/SYSTEM32/WINMM.dll (0x7ffe1ac00000)
MSVCP140.dll => /c/WINDOWS/SYSTEM32/MSVCP140.dll (0x7ffe12f90000)
VCRUNTIME140.dll => /c/WINDOWS/SYSTEM32/VCRUNTIME140.dll (0x7ffe136d0000)      
VCRUNTIME140_1.dll => /c/WINDOWS/SYSTEM32/VCRUNTIME140_1.dll (0x7ffe12db0000)  
SHELL32.dll => /c/WINDOWS/System32/SHELL32.dll (0x7ffe22440000)
IMM32.DLL => /c/WINDOWS/System32/IMM32.DLL (0x7ffe21980000)
UMPDC.dll => /c/WINDOWS/SYSTEM32/UMPDC.dll (0x7ffe20520000)
```
