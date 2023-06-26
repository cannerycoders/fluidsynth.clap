#pragma once

#include <fluidsynth.h>

#include <clap/helpers/plugin.hh>
#include <clap/helpers/misbehaviour-handler.hh>
#include <clap/helpers/checking-level.hh>
#include <string>

/* ---------------------------------------------------------------------- */
class FluidsynthPlugin : public clap::helpers::Plugin<
        clap::helpers::MisbehaviourHandler::Terminate,
        clap::helpers::CheckingLevel::Maximal>
{
public:
    static const clap_plugin_descriptor_t s_descriptor;

    FluidsynthPlugin(char const *pluginPath, clap_host const *host);
    ~FluidsynthPlugin();

    bool init() noexcept override;
    bool activate(double sampleRate, uint32_t minFrameCount, 
                    uint32_t maxFrameCount) noexcept override;
    void deactivate() noexcept override; 
    bool startProcessing() noexcept override;
    void stopProcessing() noexcept override;
    clap_process_status process(const clap_process *process) noexcept override; 
    void reset() noexcept override;
    void onMainThread() noexcept override;
    const void *extension(const char *id) noexcept override;

    /* audio ports ------------------------------------------------------ */
    bool implementsAudioPorts() const noexcept override { return true; }
    uint32_t audioPortsCount(bool isInput) const noexcept override 
        { if(isInput) return 0; else return 1; }
    bool audioPortsInfo(uint32_t index, bool isInput, clap_audio_port_info *info) const noexcept override 
    {
        if(isInput || index > 0) return false;
        info->id = 0;
        snprintf(info->name, sizeof(info->name), "%s", "Fluid outport");
        info->channel_count = 2;
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = CLAP_INVALID_ID;
        return true;
    }

    //------------------------//
    // clap_plugin_note_ports //
    //------------------------//
    bool implementsNotePorts() const noexcept override { return true; }
    uint32_t notePortsCount(bool isInput) const noexcept override
        { 
            return isInput ? 1 : 0;
        }
    bool notePortsInfo(uint32_t index, bool isInput, clap_note_port_info *info) const noexcept override
    {
        if(isInput)
        {
            snprintf(info->name, sizeof(info->name), "%s", "Fluid noteport");
            info->supported_dialects = CLAP_NOTE_DIALECT_CLAP;
            info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
            return true;
        }
        else
            return false;
    }

private:
    void processEvent(const clap_event_header_t *hdr);

private:
    fluid_settings_t *m_settings;
    fluid_synth_t *m_synth;
    int m_fontId;
    std::string m_sfont;
    int m_verbosity;

};
