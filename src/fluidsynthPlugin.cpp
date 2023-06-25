#include "fluidsynthPlugin.h"
#include "fluidapi.h"

#include <clap/helpers/plugin.hxx>
#include <clap/helpers/host-proxy.hxx>

static char const *s_features[] = 
{
    CLAP_PLUGIN_FEATURE_INSTRUMENT, 
    CLAP_PLUGIN_FEATURE_STEREO, 
    nullptr 
};

clap_plugin_descriptor_t const FluidsynthPlugin::s_descriptor = 
{
   CLAP_VERSION_INIT,
   "org.fluidsynth",
   "FluidSynth",
   "fluidsynth.org",
   "https://fluidsynth.org",
   "https://fluidsynth.org/documentation",
   "https://fluidsynth.org",
   "2.3.2",
   "Clap version of FluidSynth.",
   s_features, 
};

FluidsynthPlugin::FluidsynthPlugin(
    char const *pluginPath, clap_host const *host) :
        Plugin(&s_descriptor, host)
{}
