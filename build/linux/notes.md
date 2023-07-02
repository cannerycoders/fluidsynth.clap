# linux build notes

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
