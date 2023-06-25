#pragma once

#include <clap/helpers/plugin.hh>
#include <clap/helpers/misbehaviour-handler.hh>
#include <clap/helpers/checking-level.hh>

class FluidsynthPlugin : public clap::helpers::Plugin<
        clap::helpers::MisbehaviourHandler::Terminate,
        clap::helpers::CheckingLevel::Maximal>
{
public:
    static const clap_plugin_descriptor_t s_descriptor;

    FluidsynthPlugin(char const *pluginPath, clap_host const *host);
};