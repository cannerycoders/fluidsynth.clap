# fluidsynth.clap plugin 

- [intro](#intro)
- [parameters](#parameters)
- [presets, soundfonts](#presets-soundfonts)
- [state save/restore](#state-saverestore)
- [see also](#see-also)
- [install for users](#install-for-users)
  - [Linux](#linux)
  - [Windows](#windows)
  - [MacOS](#macos)
- [install for developers](#install-for-developers)
  - [Windows / VCPKG](#windows--vcpkg)
  - [MacOS / Homebrew](#macos--homebrew)
  - [Linux](#linux-1)
- [implementation notes](#implementation-notes)
  - [todo](#todo)
- [license](#license)

## intro

`fluidsynth.clap` is a simple, open source, "headless" (no GUI) bridge to 
the [fluidsynth soundfont synthesizer](https://fluidsynth.org) in the 
form of a [CLAP plugin](https://github.com/free-audio/clap).

In typical usage, `fluidsynth` exposes a large collection of "canned" instrument
sounds that are selected by the instrument index. Thus very little user-interface
is required. Just select an instrument index in your sound font.
The [General Midi standard](https://cannerycoders.com/docs/fiddle/reference/midiGM1.html)
defines a standard mapping between an index and an instrument. Soundfonts
with `GM` in their name purport to follow this convention.

## parameters

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

More details on these settings can be found [here](https://www.fluidsynth.org/api/settings_synth.html).

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

To override these defaults, we employ CLAP's _preset extension_ to request 
an alternate soundfont file.  In other words, `.sf2` files *are* the preset 
files compatible with this plugin.  To load a soundfont you should be able to 
request a preset load via your favorite CLAP host application.

## state save/restore

We support the `clap.state` extension but since the state includes
the filepath to the active soundfont state files may not be
terribly portable.

## see also

[soundfont wikipedia](https://en.wikipedia.org/wiki/SoundFont) |
[CLAP plugins](https://github.com/free-audio/clap) |
[fluidsynth](https://fluidsynth.org) |
[musical artifacts](https://musical-artifacts.com/artifacts?formats=sf2&tags=soundfont)

## install for users

This plugin has dependencies on the `dll`s that implement fluidsynth and 
obtaining these depend upon the platform (OS plus architecture).

### Linux 
Linux users have the easiest job because fluidsynth is widely available as 
a linux package.  For example on debian: 

> `sudo apt-get install fluidsynth fluid-soundfont-gm`

### Windows
Windows users can download a pre-compiled collection of fluidsynth
runtime components [here](https://github.com/FluidSynth/fluidsynth/releases).
The .zip file is organized in linux fashion with `bin`, `lib` and `include`
directories.

### MacOS
MacOS users either obtain the fluidsynth dlls with this fluidsynth plugin 
distribution or build/install them following the developer procedure below.

## install for developers

### Windows / VCPKG

> `git clone https://github/microsoft/vcpkg` (etc)
> `vcpkg install fluidsynth`

### MacOS / Homebrew

> `brew install fluidsynth`

### Linux

> `sudo apt-get install libfluidsynth-dev`

## implementation notes

FluidsynthPlugin derives from clap/helpers/plugin.hh which
provides methods/routes for common extensions. Since we 
provide a single plugin, factory functions are implemented 
in `dllMain.cpp`. This is where the primary dll entrypoint,
`clap_plugin_entry` is found.

We currently rely on a home-grown build system, [jsmk](https://github.com/dbadb/jsmk),
to compile and link our plugin code. This include with per-platform 
logic for locating the required fluidsynth libraries.  Since there are
so few files it should be straightforward to build using your favorite 
build system.

### todo

* Midi CCs

* Pan control? (via MIDI?)

* Most fluidsynth parameter changes are thread-safe but may cause
glitches when performed during audio processing.  Value changes
could (should?) trigger a reactivate request.

## license

Sourcecode provided herein is subject to the MIT license.  

Fluidsynth is GPL2, license found [here](https://github.com/FluidSynth/fluidsynth/blob/master/LICENSE).
Fluidsynth relies on other components subject to their own licenses.

The Clap plugin API is MIT, licenses found [here](https://github.com/free-audio/clap/blob/main/LICENSE)
and [here](https://github.com/free-audio/clap-helpers/blob/main/LICENSE)



