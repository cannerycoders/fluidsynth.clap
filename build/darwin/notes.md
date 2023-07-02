## build notes osx

## dyld headeaches

Conclusion is to statically link fluid (into our open-source plugin).
Homebrew produces static libs for glib, etc, but not sndfile, etc.

So we diable both readline and sndfile for our plugin.

## Signing and Notarization

https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci

spctl misleading:

https://developer.apple.com/forums/thread/723090