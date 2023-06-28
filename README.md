# fluidsynth.clap plugin 

A simple "headless" implementation of the fluidsynth soundfont synthesizer
for CLAP.

## parameters

| id range | description                             | value range | default |
| :------- | :-------------------------------------- | :---------- | :------ |
| 0        | gain                                    | 0-10        | 0.2     |
| 1-16     | program associated with midi chans 0-15 | 0-127       | 0       |
| 17-32    | bank associated with midi chans 0-15    | 0-127       | 0       |

## presets

To make any sound using fluidsynth you must loading a soundfont file. 
A default soundfont may not be available on your system but soundfont 
files can easily be found on the internet. The go-to starter soundfont 
is `FluidR3_GM.sf2`. `fluidsynth.clap` looks for a default soundfont according
to these platform specific conventions:

| platform | location                                               |
| :------- | :----------------------------------------------------- |
| Windows  | C:/Program Files/Common Files/Sounds/Banks/default.sf2 |
| MacOS    | /Library/Audio/Sounds/Banks/default.sf2                |
| Linux    | /usr/share/sounds/sf2/default.sf2                      |

We utilize CLAP's _preset extension_ to control soundfont loading.
In other words, `.sf2` files are the preset files compatible with
this plugin.  To load a soundfont you'll need to request a preset
load via your CLAP host application.

## state save/restore

We support the `clap.state` extension but since the state includes
the filepath to the active soundfont state files may not be
terribly portable.

## todo

* add parameters for chorus and reverb control

## see also

[soundfont wikipedia](https://en.wikipedia.org/wiki/SoundFont) |
[CLAP plugins](https://github.com/free-audio/clap) |
[fluidsynth](https://fluidsynth.org) |
[musical artifacts](https://musical-artifacts.com/artifacts?formats=sf2&tags=soundfont)

## install for users

## install for developers

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



