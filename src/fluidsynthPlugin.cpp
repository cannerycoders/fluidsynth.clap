#include "fluidsynthPlugin.h"

#include <clap/helpers/plugin.hxx>
#include <clap/helpers/host-proxy.hxx>

#include <iostream>

/* minimal lifecycle 
    fluid init
    fluid start processing
    fluid-synth input event1
    (repeat)
    (issue quit)
    ####### STOPING ENGINE #########
    fluid stop processing
    fluid deactivate
        ####### ENGINE STOPPED #########
*/

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
   "FluidSynth CLAP plugin.",
   s_features, 
};

FluidsynthPlugin::FluidsynthPlugin(
    char const *pluginPath, clap_host const *host) :
        Plugin(&s_descriptor, host),
        m_settings(nullptr),
        m_synth(nullptr)
{
    std::cerr << "fluid:pluginPath:" << pluginPath << "\n";
    m_sfont = "C:/sf2/FluidR3_GM.sf2";
}

FluidsynthPlugin::~FluidsynthPlugin()
{
    if(m_synth)
    {
        delete_fluid_synth(m_synth);
        delete_fluid_settings(m_settings);
    }
}

/* clap plugin -------------------------------------------------------------- */
bool
FluidsynthPlugin::init() noexcept
{
    std::cerr << "fluid init\n";
    if(!m_settings)
    {
        m_settings = new_fluid_settings();
        fluid_settings_setint(m_settings, "synth.verbose", 5);
        m_synth = new_fluid_synth(m_settings);
        m_fontId = fluid_synth_sfload(m_synth, m_sfont.c_str(), 1/*reset*/);
        std::cerr << "fluid font " << m_sfont << " id:" << m_fontId << "\n";
    }
    return true;
}

bool 
FluidsynthPlugin::activate(double sampleRate, uint32_t minFrameCount, 
                    uint32_t maxFrameCount) noexcept
{
    assert(m_settings && m_synth);
    std::cerr << "fluid activate " << sampleRate << " " 
            << minFrameCount << "-" << maxFrameCount << "\n";
    fluid_settings_setnum(m_settings, "synth.sample-rate", sampleRate);
    return true;
}

void 
FluidsynthPlugin::deactivate() noexcept 
{
    std::cerr << "fluid deactivate\n";
}

bool 
FluidsynthPlugin::startProcessing() noexcept
{
    std::cerr << "fluid start processing\n";
    return true;
}

void 
FluidsynthPlugin::stopProcessing() noexcept
{
    std::cerr << "fluid stop processing\n";
}

clap_process_status 
FluidsynthPlugin::process(const clap_process *process) noexcept
{
    const uint32_t nframes = process->frames_count;
    const uint32_t nev = process->in_events->size(process->in_events);
    uint32_t ev_index = 0;
    uint32_t next_ev_frame = nev > 0 ? 0 : nframes;
    float *lsamples = &(process->audio_outputs[0].data32[0][0]);
    float *rsamples = &(process->audio_outputs[0].data32[1][0]);
    for(uint32_t i=0; i < nframes; ) 
    {
        /* handle every events that happens at the frame "i" */
        while(ev_index < nev && next_ev_frame == i) 
        {
            const clap_event_header_t *hdr = process->in_events->get(
                                                process->in_events, ev_index);
            if(hdr->time != i) 
            {
                next_ev_frame = hdr->time;
                break;
            }
            this->processEvent(hdr);
            ++ev_index;
            if (ev_index == nev) 
            {
                // we reached the end of the event list
                next_ev_frame = nframes;
                break;
            }
        }
        /* process every sample until the next event */
        int sframes = next_ev_frame - i;
        float *out_lp = lsamples + i;
        float *out_rp = rsamples + i;
        int err = fluid_synth_write_float(m_synth, sframes, 
                                out_lp, 0, 1, 
                                out_rp, 0, 1);
        if(err == FLUID_FAILED)
            std::cerr << "Problem writing sframes\n";

        i += sframes;
    }
    return CLAP_PROCESS_CONTINUE;
}

void
FluidsynthPlugin::processEvent(const clap_event_header_t *hdr) 
{
    if(hdr->space_id == CLAP_CORE_EVENT_SPACE_ID) 
    {
        switch (hdr->type) 
        {
        case CLAP_EVENT_NOTE_ON: 
            {
                const clap_event_note_t *ev = (const clap_event_note_t *)hdr;
                int ivel = (int) (127 * ev->velocity);
                fluid_synth_noteon(m_synth, ev->channel, ev->key, ivel);
                std::cerr << "fluid: note on " << ev->key << " " << ivel << "\n";
                break;
            }

        case CLAP_EVENT_NOTE_OFF: 
            {
                const clap_event_note_t *ev = (const clap_event_note_t *)hdr;
                fluid_synth_noteoff(m_synth, ev->channel, ev->key);
                std::cerr << "fluid: note off\n";
                break;
            }

        case CLAP_EVENT_NOTE_CHOKE: 
            {
                const clap_event_note_t *ev = (const clap_event_note_t *)hdr;
                std::cerr << "TODO: handle note choke\n";
                break;
            }

        case CLAP_EVENT_NOTE_EXPRESSION: 
            {
                const clap_event_note_expression_t *ev = (const clap_event_note_expression_t *)hdr;
                std::cerr << "TODO: handle note expression\n";
                break;
            }

        case CLAP_EVENT_PARAM_VALUE: 
            {
                const clap_event_param_value_t *ev = (const clap_event_param_value_t *)hdr;
                std::cerr << "TODO: handle parameter change\n";
                break;
            }

        case CLAP_EVENT_PARAM_MOD: 
            {
                const clap_event_param_mod_t *ev = (const clap_event_param_mod_t *)hdr;
                std::cerr << "TODO: handle parameter modulation\n";
                break;
            }

        case CLAP_EVENT_TRANSPORT: 
            {
                const clap_event_transport_t *ev = (const clap_event_transport_t *)hdr;
                std::cerr << "TODO: handle transport event\n";
                break;
            }

        case CLAP_EVENT_MIDI: 
            {
                const clap_event_midi_t *ev = (const clap_event_midi_t *)hdr;
                std::cerr << "TODO: handle MIDI event\n";
                break;
            }

        case CLAP_EVENT_MIDI_SYSEX: 
            {
                const clap_event_midi_sysex_t *ev = (const clap_event_midi_sysex_t *)hdr;
                std::cerr << "TODO: handle MIDI sysex event\n";
                break;
            }

        case CLAP_EVENT_MIDI2: 
            {
                const clap_event_midi2_t *ev = (const clap_event_midi2_t *)hdr;
                std::cerr << "TODO: handle MIDI2 event\n";
                break;
            }
        }
   }
}

void 
FluidsynthPlugin::reset() noexcept 
{
    std::cerr << "fluid reset\n";
}

void 
FluidsynthPlugin::onMainThread() noexcept
{
    std::cerr << "on main thread\n";
}

const void *
FluidsynthPlugin::extension(const char *id) noexcept
{
    // last-case handler for extensions
    return nullptr;
}