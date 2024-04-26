#include "fluidsynthPlugin.h"
#include <clap/clap.h>
#include <clap/factory/preset-discovery.h>

#include <string>
#include <cassert>
#include <iostream>
#include <string.h> // strstr on linux

static std::string g_pluginPath;

static bool 
clap_init(const char *plugin_path) 
{
    g_pluginPath = plugin_path;
    return true;
}

static void 
clap_deinit(void) 
{
    g_pluginPath.clear();
}

static uint32_t 
clap_get_plugin_count(const clap_plugin_factory *) 
{ 
    return 1;
}

static const clap_plugin_descriptor *
clap_get_plugin_descriptor(const clap_plugin_factory *,
                        uint32_t index) 
{
    return &FluidsynthPlugin::s_descriptor;
}

static const clap_plugin *
clap_create_plugin(const clap_plugin_factory *, 
    const clap_host *host, const char *plugin_id) 
{
    auto plugin = new FluidsynthPlugin(g_pluginPath.c_str(), host);
    return plugin->clapPlugin();
}

static const clap_plugin_factory g_clap_plugin_factory = 
{
    clap_get_plugin_count,
    clap_get_plugin_descriptor,
    clap_create_plugin, // destroy happens via plugin._plugin.destroy
};

const void *clap_get_factory(const char *factory_id)
{
    if(!strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID))
        return &g_clap_plugin_factory;
    else
    if(!strcmp(factory_id, CLAP_PRESET_DISCOVERY_FACTORY_ID))
        return nullptr;
    else
        return nullptr;
}

// This symbol will be resolved by the host
CLAP_EXPORT const clap_plugin_entry clap_entry = 
{
    CLAP_VERSION,
    clap_init,
    clap_deinit,
    clap_get_factory,
};