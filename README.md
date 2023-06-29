# fluidsynth.clap plugin 

- [intro](#intro)
- [install with fluidsynth](#install-with-fluidsynth)
- [presets, soundfonts](#presets-soundfonts)
- [state save/restore](#state-saverestore)
- [parameters](#parameters)
- [see also](#see-also)
- [devinstall](#devinstall)
  - [win32](#win32)
  - [linux](#linux)
  - [macos](#macos)
- [implementation notes](#implementation-notes)
  - [todo](#todo)
- [buyer beware](#buyer-beware)
- [license](#license)

## intro

`fluidsynth.clap` is a simple, open source, "headless" (no GUI) bridge to 
the [fluidsynth soundfont synthesizer](https://fluidsynth.org) in the 
form of a [CLAP plugin](https://github.com/free-audio/clap).

## install with fluidsynth

In our releases section you can find a .zip file that includes 
both the fluidsynth.clap plugin _and_ the fluidsynth runtime 
components.  Unzip the file into one of these standard CLAP plugin
locations.  

| platform | type   | typical path                       | semantic           |
| :------- | :----- | :--------------------------------- | :----------------- |
| windows  | system | c:/Program Files/Common Files/CLAP | COMMONPROGRAMFILES |
| windows  | user   | $HOME/AppData/Local/Programs/CLAP  | LOCALAPPDATA       |
| macos    | system | /Library/Audio/Plug-Ins/CLAP       |                    |
| macos    | user   | $HOME/Library/Audio/Plug-Ins/CLAP  |                    |
| linux    | system | /usr/lib/clap                      |                    |
| linux    | user   | $HOME/.clap                        |                    |

If you already have a fluidsynth installation or wish to build your own 
plugin see [below](#devinstall).

## presets, soundfonts

To make any sound using fluidsynth you must load a soundfont file. 
A default soundfont may not be available on your system but soundfont 
files can easily be found on the internet. The go-to starter soundfont 
is `FluidR3_GM.sf2`. `fluidsynth.clap` looks for a default soundfont 
according to these platform specific conventions:

| platform | location                                               |
| :------- | :----------------------------------------------------- |
| Windows  | C:/Program Files/Common Files/Sounds/Banks/default.sf2 |
| MacOS    | /Library/Audio/Sounds/Banks/default.sf2                |
| Linux    | /usr/share/sounds/sf2/default.sf2                      |

To override these defaults, we employ CLAP's _preset extension_ to allow you 
to request an alternate soundfont file.  In other words, `.sf2` files *are* 
the preset files compatible with this plugin.  To load a soundfont you should 
be able to request a preset load via your favorite CLAP host application.

## state save/restore

We support the `clap.state` extension but since the state includes
the filepath to the active soundfont state files may not be
terribly portable.

## parameters

In typical usage, `fluidsynth` exposes a large collection of "canned" instrument
sounds that are selected by the instrument index. Thus, very little user-interface
is required. Just select instrument/bank indices in your sound font.

The [General Midi standard](https://cannerycoders.com/docs/fiddle/reference/midiGM1.html)
defines a standard mapping between an index and an instrument. Soundfonts
with `GM` in their name purport to follow this convention.


| id range | description                             | value range | default |
| :------- | :-------------------------------------- | :---------- | :------ |
| 0        | gain                                    | 0-10        | 0.2     |
| 1        | reverb                                  | 0,1         | 1       |
| 2        | roomsize                                | 0-1.2       | 0.2     |
| 3        | damping                                 | 0-1         | 0.0     |
| 4        | width                                   | 0-100       | .5      |
| 5        | reverblevel                             | 0-1         | 0.9     |
| 6        | chorus                                  | 0,1         | 1       |
| 7        | chorusNR                                | 0-99        | 3       |
| 8        | choruslevel                             | 0-10        | 2       |
| 9        | chorusspeed                             | 0-1         | .3      |
| 10       | chorusdepth                             | 0-256       | 8       |
| 11       | chorusmod                               | 0,1         | 1       |
| 32-47    | program associated with midi chans 0-15 | 0-127       | 0       |
| 48-63    | bank associated with midi chans 0-15    | 0-127       | 0       |

More details on these settings can be found [in the fluidsynth docs](https://www.fluidsynth.org/api/settings_synth.html).

## see also

[soundfont wikipedia](https://en.wikipedia.org/wiki/SoundFont) |
[CLAP plugins](https://github.com/free-audio/clap) |
[fluidsynth](https://fluidsynth.org) |
[musical artifacts](https://musical-artifacts.com/artifacts?formats=sf2&tags=soundfont)

## devinstall

This plugin has dependencies on the `dlls` that implement fluidsynth, 
obtaining them depends upon the platform (OS plus architecture).
Our intention is to make the entire collection of .clap and .dll/.so files
available via our releases tags. This means that in order to use this
plugin you need only download the latest .zip file and decompress it
into a standard CLAP location (system or user).

Depending on your CLAP host and platform security settings, you may still need
to add the location of `fluidsynth.clap` to `PATH`, and/or 
`LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH`.

If you wish to build or download fluidsynth yourself, please proceed to
the platform-specific instructions that follow.


### win32

Windows users can download a pre-compiled collection of fluidsynth
runtime components [here](https://github.com/FluidSynth/fluidsynth/releases).
The .zip file is organized in linux fashion with `bin`, `lib` and `include`
directories.  In order for fluidsynth.clap to find the .dll files you may
need to add the install location to your PATH.

Another approach is to install vcpkg and then build fluidsynth yourself.

```sh
git clone https://github/microsoft/vcpkg (etc)
vcpkg install fluidsynth`
```


### linux 
Linux users have the easiest job because fluidsynth is widely available as 
a linux package.  For example on debian: 

```sh
sudo apt-get install fluidsynth fluid-soundfont-gm
```

And to obtain the compile/API support:

```sh
sudo apt-get install libfluidsynth-dev`
```

### macos

MacOS users either obtain the fluidsynth dlls with this fluidsynth plugin 
distribution or build/install them following the developer procedure below.

```sh
brew install fluidsynth`
```

## implementation notes

FluidsynthPlugin derives from clap/helpers/plugin.hh which
provides methods/routes for common extensions. Since we 
provide a single plugin, factory functions are implemented 
in `dllMain.cpp`. This is where the primary dll entrypoint,
`clap_plugin_entry` is found.

We currently rely on a home-grown build system, [jsmk](https://github.com/dbadb/jsmk),
to compile and link our plugin code. This includes per-platform 
logic for locating the required fluidsynth libraries.  Since there are
so few files in our implementation it should be straightforward to build 
using your favorite build system.

### todo

* Most fluidsynth effects parameter changes are thread-safe but may cause
glitches when performed during audio processing.  Value changes
could (should?) trigger a reactivate request.

## buyer beware

We offer no warranties, guarantees or commitments. Watch out for sharp
edges, this is not a toy, though it may appear so.

## license

Sourcecode provided herein is subject to the MIT license.  

Binaries provided via releases may include components with different licenses.

Fluidsynth is GPL2, license found [here](https://github.com/FluidSynth/fluidsynth/blob/master/LICENSE).
Fluidsynth relies on other components subject to their own licenses.

The Clap plugin API is MIT licensed, licenses found [here](https://github.com/free-audio/clap/blob/main/LICENSE)
and [here](https://github.com/free-audio/clap-helpers/blob/main/LICENSE)
