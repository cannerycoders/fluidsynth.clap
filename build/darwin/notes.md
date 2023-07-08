## build notes osx

## dyld headeaches

Conclusion is to statically link fluid (into our open-source plugin).
Homebrew produces static libs for glib, etc, but not sndfile, etc.

So we diable both readline and sndfile for our plugin.

## Signing and Notarization

https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci

spctl misleading:

https://developer.apple.com/forums/thread/723090


## choc/gui 

* claphost provides an NSView as "parent window"
* webview requires a non-zero size to be set (still haven't figured out resizing)
* webview doens't seem to support external links.
* webview worked in debugger but not outside of debugger
    * https://developer.apple.com/forums/thread/116359  (need entitlement)
    * https://developer.apple.com/forums/thread/99105 (sandboxing?)
    (status: didn't get over this humb with clap host)
* fluidsynth crashes onda (due to a duplicate webview instance?), show stopped.

claphost -p /Library/Audio/Plug-Ins/CLAP/FluidSynth.clap 


