#pragma once

#include <fluidsynth.h>

#include <clap/helpers/plugin.hh>
#include <clap/helpers/misbehaviour-handler.hh>
#include <clap/helpers/checking-level.hh>

#include <choc/platform/choc_Platform.h>
#include <choc/platform/choc_Assert.h>
#include <choc/gui/choc_DesktopWindow.h>
#include <choc/gui/choc_WebView.h>

#include <string>
#include <filesystem> // c++17 dependency
#include <map>

/* ---------------------------------------------------------------------- */
// our instances are created by the factory in dllMain
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

    /* note ports ---------------------------------------------------------- */
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
            info->supported_dialects = CLAP_NOTE_DIALECT_CLAP|CLAP_NOTE_DIALECT_MIDI;
            info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
            return true;
        }
        else
            return false;
    }

    /* -- params ----------------------------------------------------- */
    bool implementsParams() const noexcept override { return true; }
    uint32_t paramsCount() const noexcept override;
    bool paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept override; 
    bool paramsValue(clap_id paramId, double *value) noexcept override;
    bool paramsValueToText(clap_id paramId, double value, char *display, uint32_t size) noexcept override; 
    bool paramsTextToValue(clap_id paramId, const char *display, double *value) noexcept override;
    void paramsFlush(const clap_input_events *in, const clap_output_events *out) noexcept override;

    int paramIndex(uint32_t paramId);

    /* -- presets ----------------------------------------------------- */
    bool implementsPresetLoad() const noexcept override { return true; }
    bool presetLoadFromLocation(uint32_t location_kind,
                                const char *location,
                                const char *load_key) noexcept override;

    /* -- state ----------------------------------------------------- */
    bool implementsState() const noexcept override { return true; }
    bool stateSave(const clap_ostream *stream) noexcept override; 
    bool stateLoad(const clap_istream *stream) noexcept override; 

    /* -- gui ------------------------------------------------------- */
    virtual bool implementsGui() const noexcept override { return false; }
    virtual bool guiIsApiSupported(const char *api, bool isFloating) noexcept override;
    virtual bool guiGetPreferredApi(const char **api, bool *is_floating) noexcept override;
    virtual bool guiCreate(const char *api, bool isFloating) noexcept override; 
    virtual void guiDestroy() noexcept override;
    virtual bool guiSetScale(double scale) noexcept override;
    virtual bool guiShow() noexcept override;
    virtual bool guiHide() noexcept override;
    virtual bool guiGetSize(uint32_t *width, uint32_t *height) noexcept override;
    virtual bool guiCanResize() const noexcept override;
    virtual bool guiGetResizeHints(clap_gui_resize_hints_t *hints) noexcept override;
    virtual bool guiAdjustSize(uint32_t *width, uint32_t *height) noexcept override;
    virtual bool guiSetSize(uint32_t width, uint32_t height) noexcept override;
    virtual void guiSuggestTitle(const char *title) noexcept override;
    virtual bool guiSetParent(const clap_window *window) noexcept override;
    virtual bool guiSetTransient(const clap_window *window) noexcept override;

private:
    void processEvent(const clap_event_header_t *hdr);
    void setParamValue(int paramid, double value);
    uint32_t m_guiSize[2];
    void updateVoices();
    std::string m_voices; // JSON string

private: // gui
    choc::ui::WebView *m_webview = nullptr; // null unless InitWebview
    choc::ui::DesktopWindow *m_window = nullptr;
    using Resource = choc::ui::WebView::Options::Resource;
    using Path = choc::ui::WebView::Options::Path;
    std::optional<Resource> GetResource(Path const &);
    choc::ui::WebView::Options m_webviewOptions;

private:
    fluid_settings_t *m_settings;
    fluid_synth_t *m_synth;
    int m_fontId;
    int m_verbosity;
    std::filesystem::path m_pluginPath;
    std::vector<std::filesystem::path> m_pluginPresetDirs;
    std::filesystem::path m_sfontPath;
    mutable std::map<uint32_t, double> m_paramValues; // initialized during paramsInfo

    enum paramId
    {
        k_Gain = 0,

        k_Reverb, // on-off
        k_RevRoomsize, // 0-1.2
        k_RevDamping,  // 0-1
        k_RevWidth,    // 0-100
        k_RevLevel,    // 0-1

        k_Chorus, // on-off
        k_ChorusNR, // 0-99, voice-count
        k_ChorusLevel, // 0-1
        k_ChorusSpeed, // Hz (.29 - 5)
        k_ChorusDepth, // ms (0 - 21)
        k_ChorusMod,  // sine or triangle
        k_indexedParamCount,

        k_Prog0 = 32,   // programs associated with 16 midi channels

        k_Bank0 = 48,   // banks associated with 16 midi channels

        k_numParams = k_indexedParamCount + 32
    };
    float m_gain = .2f;

    static clap_param_info s_fluidParams[];
};
