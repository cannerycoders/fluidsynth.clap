# linux build notes

* `mkdir _built`
* `cd _built`
* `cmake ../../../fluidsynth` (git remote synced to desired tag (tags/v2.3.3))
* `make`

`ldd src/libfluidsynth.so`
  linux-vdso.so.1 (0x00007fffe70c6000)
  libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f92de85a000)
  libgomp.so.1 => /lib/x86_64-linux-gnu/libgomp.so.1 (0x00007f92de81a000)
  libglib-2.0.so.0 => /lib/x86_64-linux-gnu/libglib-2.0.so.0 (0x00007f92de6e9000)
  libsndfile.so.1 => /lib/x86_64-linux-gnu/libsndfile.so.1 (0x00007f92de666000)
  libpulse-simple.so.0 => /lib/x86_64-linux-gnu/libpulse-simple.so.0 (0x00007f92de65f000)
  libpulse.so.0 => /lib/x86_64-linux-gnu/libpulse.so.0 (0x00007f92de60b000)
  libasound.so.2 => /lib/x86_64-linux-gnu/libasound.so.2 (0x00007f92de50c000)
  libjack.so.0 => /lib/x86_64-linux-gnu/libjack.so.0 (0x00007f92de4bb000)
  libdbus-1.so.3 => /lib/x86_64-linux-gnu/libdbus-1.so.3 (0x00007f92de466000)
  libreadline.so.8 => /lib/x86_64-linux-gnu/libreadline.so.8 (0x00007f92de40f000)
  libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f92de242000)
  libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f92de0fe000)
  libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f92de0e2000)
  libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f92ddf0e000)
  /lib64/ld-linux-x86-64.so.2 (0x00007f92de964000)
  libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f92ddf08000)
  libpcre.so.3 => /lib/x86_64-linux-gnu/libpcre.so.3 (0x00007f92dde95000)
  libFLAC.so.8 => /lib/x86_64-linux-gnu/libFLAC.so.8 (0x00007f92dde56000)
  libvorbis.so.0 => /lib/x86_64-linux-gnu/libvorbis.so.0 (0x00007f92dde29000)
  libvorbisenc.so.2 => /lib/x86_64-linux-gnu/libvorbisenc.so.2 (0x00007f92ddd7c000)
  libopus.so.0 => /lib/x86_64-linux-gnu/libopus.so.0 (0x00007f92ddd21000)
  libogg.so.0 => /lib/x86_64-linux-gnu/libogg.so.0 (0x00007f92ddd14000)
  libpulsecommon-14.2.so => /usr/lib/x86_64-linux-gnu/pulseaudio/libpulsecommon-14.2.so (0x00007f92ddc90000)
  librt.so.1 => /lib/x86_64-linux-gnu/librt.so.1 (0x00007f92ddc86000)
  libsystemd.so.0 => /lib/x86_64-linux-gnu/libsystemd.so.0 (0x00007f92ddbcf000)
  libtinfo.so.6 => /lib/x86_64-linux-gnu/libtinfo.so.6 (0x00007f92ddba0000)
  libxcb.so.1 => /lib/x86_64-linux-gnu/libxcb.so.1 (0x00007f92ddb75000)
  libwrap.so.0 => /lib/x86_64-linux-gnu/libwrap.so.0 (0x00007f92ddb69000)
  libasyncns.so.0 => /lib/x86_64-linux-gnu/libasyncns.so.0 (0x00007f92ddb61000)
  liblzma.so.5 => /lib/x86_64-linux-gnu/liblzma.so.5 (0x00007f92ddb37000)
  libzstd.so.1 => /lib/x86_64-linux-gnu/libzstd.so.1 (0x00007f92dda5c000)
  liblz4.so.1 => /lib/x86_64-linux-gnu/liblz4.so.1 (0x00007f92dda39000)
  libgcrypt.so.20 => /lib/x86_64-linux-gnu/libgcrypt.so.20 (0x00007f92dd919000)
  libXau.so.6 => /lib/x86_64-linux-gnu/libXau.so.6 (0x00007f92dd914000)
  libXdmcp.so.6 => /lib/x86_64-linux-gnu/libXdmcp.so.6 (0x00007f92dd70e000)
  libnsl.so.2 => /lib/x86_64-linux-gnu/libnsl.so.2 (0x00007f92dd6f1000)
  libresolv.so.2 => /lib/x86_64-linux-gnu/libresolv.so.2 (0x00007f92dd6d7000)
  libgpg-error.so.0 => /lib/x86_64-linux-gnu/libgpg-error.so.0 (0x00007f92dd6b1000)
  libbsd.so.0 => /lib/x86_64-linux-gnu/libbsd.so.0 (0x00007f92dd69a000)
  libtirpc.so.3 => /lib/x86_64-linux-gnu/libtirpc.so.3 (0x00007f92dd66a000)
  libmd.so.0 => /lib/x86_64-linux-gnu/libmd.so.0 (0x00007f92dd65b000)
  libgssapi_krb5.so.2 => /lib/x86_64-linux-gnu/libgssapi_krb5.so.2 (0x00007f92dd608000)
  libkrb5.so.3 => /lib/x86_64-linux-gnu/libkrb5.so.3 (0x00007f92dd52e000)
  libk5crypto.so.3 => /lib/x86_64-linux-gnu/libk5crypto.so.3 (0x00007f92dd4fe000)
  libcom_err.so.2 => /lib/x86_64-linux-gnu/libcom_err.so.2 (0x00007f92dd4f8000)
  libkrb5support.so.0 => /lib/x86_64-linux-gnu/libkrb5support.so.0 (0x00007f92dd4e9000)
  libkeyutils.so.1 => /lib/x86_64-linux-gnu/libkeyutils.so.1 (0x00007f92dd4e0000)
  
## details
* default fluidsynth for debian 11 library name: 2.3.7, 
    * header is missing entrypoints for setting chorus, reverb, etc.
    * fluidsynth --version reads runtime version: __2.1.7__
* github source builds `libfluidsynth.so.3.2.1`
    * says latest tag is _2.3.3_
    * this matches build/include/fluidsynth/version.h
    * ldd on file points to standard, easily installed glib, etc.
      so the plan is to take libfluidsynth from the cmake-based build,
      and link our .clap plugin against that with the right rpath
      settings.
    * we copy the minimal built pieces into parent's _prebuilt sudir
      following the structure of the windows download. Headers come
      from 2 diverse places. (public are in fluidsynth/include/fluidsynth),
      driver is in build/include/fluidsynth.h, also version.h)
    * need to name it `libfluidsynth.so.3` otherwise we'll need
      the usual symlink gibberish.

Summary - currently we won't build a static lib, but ship a custom/modern 
  fluidlib with dynamic links to other components. Customer can install
  any-old (recent) fluidsynth to get the dependencies or install libsndfile.

config.h is here for reference, currently no used.

`ldd .../FluidSynth.clap` shows success/fail, watch out for cwd errors.
`readelf -d FluidSynth.clap` shows the details
