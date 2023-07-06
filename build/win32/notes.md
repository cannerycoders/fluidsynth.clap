# windows build notes

If claphost doesn't load the .dll, it's probably the case that it doesn't do
this:

https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setdlldirectorya

Now, the workaround is theoretically to ensure that the FluidSynth.clap dir
is in the PATH.

There is also the gflags tool, but I haven't had much success with it 
Output only visible in the debugger.

https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/gflags#where-to-get-gflags

`gflags -i clap-host.exe +sls`

Current Registry Settings for clap-host.exe executable are: 00000002
    sls - Show Loader Snaps
