#include "fluidsynthPlugin.h"

#include <clap/helpers/plugin.hxx>
#include <clap/helpers/host-proxy.hxx>

#include <iostream>

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
   "FluidSynth.clap",
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
        m_synth(nullptr),
        m_pluginPath(pluginPath),
        m_window(nullptr),
        m_webview(nullptr)
{
    #ifdef _WIN32
    char const *appdata = getenv("LOCALAPPDATA");
    m_pluginPresetDirs.push_back(std::filesystem::path("C:/Program Files/Common Files/Sounds/Banks"));
    if(appdata)
    {
        std::filesystem::path ubanks = std::filesystem::path(appdata) / "Sounds/Banks";
        m_pluginPresetDirs.push_back(ubanks);
    }
    #elif defined(__APPLE__)
    char const *home = getenv("HOME");
    m_pluginPresetDirs.push_back(std::filesystem::path("/Library/Audio/Sounds/Banks"));
    if(home)
    {
        std::filesystem::path ubanks = std::filesystem::path(home) / "Library/Audio/Sounds/Banks";
        m_pluginPresetDirs.push_back(ubanks);
    }
    #else
    char const *home = getenv("HOME");
    m_pluginPresetDirs.push_back(std::filesystem::path("/usr/share/sounds/sf2"));
    m_pluginPresetDirs.push_back(std::filesystem::path("/usr/local/share/sounds/sf2"));
    m_pluginPresetDirs.push_back(std::filesystem::path(home) / "Documents/sounds/sf2");
    #endif
    for(auto x : m_pluginPresetDirs)
    {
        m_sfontPath = x / "default.sf2";
        if(std::filesystem::exists(m_sfontPath))
            break;
    }
    if(getenv("FLUIDSYNTH_CLAP_DEBUG"))
        m_verbosity = 2;
    else
        m_verbosity = 0;
}

FluidsynthPlugin::~FluidsynthPlugin()
{
    if(m_verbosity > 0)
        std::cerr << "~FluidsynthPlugin\n";
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
    // NB: this happens during dumpPlugs
    if(m_verbosity > 0)
        std::cerr << "fluid init\n";
    return true;
}

bool 
FluidsynthPlugin::activate(double sampleRate, uint32_t minFrameCount, 
                    uint32_t maxFrameCount) noexcept
{
    if(!m_settings)
    {
        if(!std::filesystem::exists(m_sfontPath))
        {
            std::cerr << "Can't find default soundfont " << m_sfontPath 
                << ".\nPlease select one using the host's preset loader.\n";
        }

        // https://www.fluidsynth.org/api/settings_synth.html
        m_settings = new_fluid_settings();
        if(m_verbosity > 1)
        {
            // per-channel config, per-note details
            fluid_settings_setint(m_settings, "synth.verbose", 1);
        }
        m_synth = new_fluid_synth(m_settings);
        m_fontId = fluid_synth_sfload(m_synth, 
                        m_sfontPath.generic_string().c_str(), 
                        1/*reset*/);
        if(m_verbosity > 0)
            std::cerr << "fluid font " << m_sfontPath << " id:" << m_fontId << "\n";
    }

    if(m_verbosity > 0)
    {
        std::cerr << "fluid activate " << sampleRate << " " 
            << minFrameCount << "-" << maxFrameCount << "\n";
    }
    fluid_settings_setnum(m_settings, "synth.sample-rate", sampleRate);
    return true;
}

void 
FluidsynthPlugin::deactivate() noexcept 
{
    if(m_verbosity > 0)
        std::cerr << "fluid deactivate\n";
}

bool 
FluidsynthPlugin::startProcessing() noexcept
{
    if(m_verbosity > 0)
        std::cerr << "fluid start processing\n";
    return true;
}

void 
FluidsynthPlugin::stopProcessing() noexcept
{
    if(m_verbosity > 0)
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
            if(ev_index == nev) 
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
        switch(hdr->type) 
        {
        case CLAP_EVENT_NOTE_ON: 
            {
                const clap_event_note_t *ev = (const clap_event_note_t *)hdr;
                int ivel = (int) (127 * ev->velocity);
                fluid_synth_noteon(m_synth, ev->channel, ev->key, ivel);
                if(m_verbosity > 0)
                    std::cerr << "fluid: note on " << ev->key << " " << ivel << "\n";
                break;
            }

        case CLAP_EVENT_NOTE_OFF: 
            {
                const clap_event_note_t *ev = (const clap_event_note_t *)hdr;
                fluid_synth_noteoff(m_synth, ev->channel, ev->key);
                if(m_verbosity > 0)
                    std::cerr << "fluid: note off " << ev->key << "\n";
                break;
            }

        case CLAP_EVENT_NOTE_CHOKE: 
            {
                // const clap_event_note_t *ev = (const clap_event_note_t *)hdr;
                std::cerr << "TODO: handle note choke\n";
                break;
            }

        case CLAP_EVENT_NOTE_EXPRESSION: 
            {
                // const clap_event_note_expression_t *ev = (const clap_event_note_expression_t *)hdr;
                std::cerr << "TODO: handle note expression\n";
                break;
            }

        case CLAP_EVENT_PARAM_VALUE: 
            {
                const clap_event_param_value_t *ev = (const clap_event_param_value_t *)hdr;
                // std::cerr << "fluid.set param " << ev->param_id << "\n";
                this->setParamValue(ev->param_id, ev->value);
                break;
            }

        case CLAP_EVENT_PARAM_MOD: 
            {
                // const clap_event_param_mod_t *ev = (const clap_event_param_mod_t *)hdr;
                std::cerr << "TODO: handle parameter modulation\n";
                break;
            }

        case CLAP_EVENT_TRANSPORT: 
            {
                // const clap_event_transport_t *ev = (const clap_event_transport_t *)hdr;
                std::cerr << "TODO: handle transport event\n";
                break;
            }

        case CLAP_EVENT_MIDI: 
            {
                const clap_event_midi_t *ev = (const clap_event_midi_t *)hdr;
                unsigned char cmd = 0xF0 & ev->data[0];
                int chan = 0x0F & ev->data[0];
                switch(cmd)
                {
                case 0x80:
                    fluid_synth_noteoff(m_synth, chan, ev->data[1]);
                    break;
                case 0x90:
                    fluid_synth_noteon(m_synth, chan, ev->data[1], ev->data[2]);
                    break;
                case 0xa0: // polyphonic aftertouch
                    fluid_synth_key_pressure(m_synth, chan, ev->data[1], ev->data[2]);
                    break;
                case 0xb0: // cc
                    fluid_synth_cc(m_synth, chan, ev->data[1], ev->data[2]);
                    break;
                case 0xc0: // program change
                    fluid_synth_program_change(m_synth, chan, ev->data[1]);
                    break;
                case 0xd0: // _channel_ aftertouch / pressure
                    fluid_synth_channel_pressure(m_synth, chan, ev->data[1]);
                    break;
                case 0xe0:  // pitch wheel
                    {
                        int lsb = ev->data[1] & 0x7F;
                        int msb = ev->data[2] & 0x7F;
                        int pitchbend = lsb + (msb << 7);
                        fluid_synth_pitch_bend(m_synth, chan, pitchbend);
                    }
                    break;
                default:
                    std::cout << "TODO: handle MIDI event 0x" 
                        << std::setfill('0') << std::setw(2)
                        << std::hex << (int) ev->data[0] 
                        << (int) ev->data[1] 
                        << (int) ev->data[2] << "\n";
                    break;
                }
                break; // CLAP_EVENT_MIDI
            }

        case CLAP_EVENT_MIDI_SYSEX: 
            {
                // const clap_event_midi_sysex_t *ev = (const clap_event_midi_sysex_t *)hdr;
                std::cerr << "TODO: handle MIDI sysex event\n";
                break;
            }

        case CLAP_EVENT_MIDI2: 
            {
                // const clap_event_midi2_t *ev = (const clap_event_midi2_t *)hdr;
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

// On soundfonts...
// The Preset (also often referred to as an ”instrument”, a “program”, 
// or a “patch”) is the feature that is visible to the outside. 
// Presets are combined into Banks. 
// Each Bank can hold 128 Presets and these are numbered either 
// from 0 to 127 (or 1 to 128 - SynthFont uses the range 0-127). 
// There can be 128 Banks (numbered 1-128 or 0-127). Hence the 
// total number of Presets in a SoundFont file is large enough. 
// Very few SoundFonts have more than a few Banks in use. Usually 
// these are “variation banks”, .i.e there may be a slightly different 
// Acoustic Piano in Bank 1 Preset 0 (1:0) than in  0:0. 
// Banks 0 to 127 are called the Melodic banks while bank 
// 128 is reserved for Percussion presets. 
// The MIDI standard defines MIDI channel 9 (on the scale 0 to 15) 
// as the Percussion channel and hence all MIDI Programs in this 
// channel will automatically call for a preset in bank 128.
/*static*/ clap_param_info 
FluidsynthPlugin::s_fluidParams[] =
{
    {
        k_Gain,
        CLAP_PARAM_IS_AUTOMATABLE, // flags
        nullptr, 
        "gain",
        "",
        0., 10., .2,
    },

    /* reverb -------------------------- */
    {
        k_Reverb,
        CLAP_PARAM_IS_STEPPED,
        nullptr, 
        "reverb",
        "",
        0., 1., 1,
    },

    {
        k_RevRoomsize,
        0, // not realtime-safe
        nullptr, 
        "roomsize",
        "",
        0., 1.2, .2,
    },

    {
        k_RevDamping,
        0, // not realtime-safe
        nullptr, 
        "damping",
        "",
        0., 1, .2,
    },

    {
        k_RevWidth,
        0, // not realtime-safe
        nullptr, 
        "width",
        "",
        0., 100, 20,
    },

    {
        k_RevLevel,
        0, // not realtime-safe
        nullptr, 
        "reverblevel",
        "",
        0., 1, .9,
    },

    /* chorus ---------------------------- */
    {
        k_Chorus,
        CLAP_PARAM_IS_STEPPED,
        nullptr, 
        "chorus",
        "",
        0., 1., 1,
    },
    {
        k_ChorusNR,
        CLAP_PARAM_IS_STEPPED, // not realtime-safe
        nullptr, 
        "chorusNR",
        "",
        0., 99, 3,
    },
    {
        k_ChorusLevel,
        0, // not realtime-safe
        nullptr, 
        "choruslevel",
        "",
        0., 1, .5,
    },
    {
        k_ChorusSpeed,
        0, // not realtime-safe
        nullptr, 
        "chorusspeed",
        "",
        0., 1, .5,
    },
    {
        k_ChorusDepth,
        0, // not realtime-safe
        nullptr, 
        "chorusdepth",
        "",
        0., 21, 5,
    },
    {
        k_ChorusMod,
        CLAP_PARAM_IS_STEPPED, // not realtime-safe
        nullptr, 
        "chorusmod",
        "",
        0., 1, 0 // sine or triangle
    },

    /* program --------------------------- */
    {
        k_Prog0, // prog0 id
        CLAP_PARAM_IS_STEPPED,  // flags
        nullptr,                // cookie
        "prog0",                // name 
        "",                     // module
        0., 127., 0.,          // program for ch0
    },

    /* bank ------------------------------ */
    {
        k_Bank0, // bank0 id
        CLAP_PARAM_IS_STEPPED,  // flags
        nullptr,                // cookie
        "bank0",                // name 
        "",                     // module
        0., 128., 0.,           // bank for ch0
    },
};

uint32_t 
FluidsynthPlugin::paramsCount() const noexcept
{
    assert((k_numParams - 30) == sizeof(s_fluidParams) / sizeof(s_fluidParams[0]));
    return k_numParams;
}

bool 
FluidsynthPlugin::paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept
{
    if(paramIndex < k_indexedParamCount)
    {
        *info = s_fluidParams[paramIndex];
        return true;
    }
    else
    {
        int relIndex = paramIndex - k_indexedParamCount;
        if(relIndex < 16)
        {
            int chan = relIndex;
            *info = s_fluidParams[k_indexedParamCount];
            info->id += chan;
            snprintf(info->name, sizeof(info->name), "prog%d", chan);
        }
        else // if(paramIndex >= k_Bank0 && paramIndex < k_Bank0+16)
        {
            int chan = (relIndex - 16);
            *info = s_fluidParams[k_indexedParamCount+1];
            info->id += chan;
            snprintf(info->name, sizeof(info->name), "bank%d", chan);
        }
        return true;
    }
} 

/// The host can at any time read parameters' value on the [main-thread] using
/// @ref clap_plugin_params.value().
///
/// There are two options to communicate parameter value changes, and they are 
/// not concurrent.
/// - send automation points during clap_plugin.process()
/// - send automation points during clap_plugin_params.flush(), 
//    for parameter changes without processing audio.
bool 
FluidsynthPlugin::paramsValue(clap_id paramid, double *value) noexcept
{
    if(paramid == k_Gain)
        *value = m_gain;
    else
    if(paramid <= k_RevLevel)
    {
        switch(paramid)
        {
        case k_Reverb: // on-off
            *value = 1; // fluid_synth_get_reverb_on(m_synth);
            break; 
        case k_RevRoomsize: // 0-1.2
            fluid_synth_get_reverb_group_roomsize(m_synth, -1, value);
            break; 
        case k_RevDamping:  // 0-1
            fluid_synth_get_reverb_group_damp(m_synth, -1, value);
            break; 
        case k_RevWidth:    // 0-100
            fluid_synth_get_reverb_group_width(m_synth, -1, value);
            break; 
        case k_RevLevel:
            fluid_synth_get_reverb_group_level(m_synth, 1, value);
            break;
        default:
            assert(0);
        }
    }
    else
    if(paramid <= k_ChorusMod)
    {
        switch(paramid)
        {
        case k_Chorus: // on-off
            *value = 1; // fluid_synth_get_reverb_on(m_synth);
            break;
        case k_ChorusNR: // 0-99, voice-count
            {
                int nr;
                fluid_synth_get_chorus_group_nr(m_synth, -1, &nr);
                *value = nr;
            }
            break;
        case k_ChorusLevel: // 0-1
            fluid_synth_get_chorus_group_level(m_synth, -1, value);
            break;
        case k_ChorusSpeed: // Hz (.29 - 5)
            fluid_synth_get_chorus_group_speed(m_synth, -1, value);
            break;
        case k_ChorusDepth: // ms (0 - 21)
            fluid_synth_get_chorus_group_depth(m_synth, -1, value);
            break;
        case k_ChorusMod:  // sine or triangle
            {
                int i;
                fluid_synth_get_chorus_group_type(m_synth, -1, &i);
                *value = i;
            }
            break;
        default:
            assert(0);
        }
    }
    else
    {
        if(m_synth)
        {
            int chan;
            if(paramid >= k_Bank0)
                chan = paramid - k_Bank0;
            else
                chan = paramid - k_Prog0;
            
            int fontId, bank, prog;
            fluid_synth_get_program(m_synth, chan, &fontId, &bank, &prog);
            if(paramid >= k_Bank0)
                *value = bank;
            else
                *value = prog;
        }
        else
            *value = 0;
    }
    return true;
}

void
FluidsynthPlugin::setParamValue(int paramid, double value)
{
    if(paramid == k_Gain)
    {
        m_gain = (float) value;
        if(m_synth)
            fluid_synth_set_gain(m_synth, m_gain);
    }
    else
    if(paramid <= k_RevLevel)
    {
        switch(paramid)
        {
        case k_Reverb: // on-off
            fluid_synth_reverb_on(m_synth, -1, (int) value);
            break; 
        case k_RevRoomsize: // 0-1.2
            fluid_synth_set_reverb_group_roomsize(m_synth, -1, value);
            break; 
        case k_RevDamping:  // 0-1
            fluid_synth_set_reverb_group_damp(m_synth, -1, value);
            break; 
        case k_RevWidth:    // 0-100
            fluid_synth_set_reverb_group_width(m_synth, -1, value);
            break; 
        case k_RevLevel:
            fluid_synth_set_reverb_group_level(m_synth, 1, value);
            break;
        default:
            assert(0);
        }
    }
    else
    if(paramid <= k_ChorusMod)
    {
        switch(paramid)
        {
        case k_Chorus: // on-off
            fluid_synth_chorus_on(m_synth, -1, (int) value);
            break;
        case k_ChorusNR: // 0-99, voice-count
            fluid_synth_set_chorus_group_nr(m_synth, -1, (int) value);
            break;
        case k_ChorusLevel: // 0-1
            fluid_synth_set_chorus_group_level(m_synth, -1, value);
            break;
        case k_ChorusSpeed: // Hz (.29 - 5)
            fluid_synth_set_chorus_group_speed(m_synth, -1, value);
            break;
        case k_ChorusDepth: // ms (0 - 21)
            fluid_synth_set_chorus_group_depth(m_synth, -1, value);
            break;
        case k_ChorusMod:  // sine or triangle
            fluid_synth_set_chorus_group_type(m_synth, -1, (int) value);
            break;
        default:
            assert(0);
        }
    }
    else
    if(paramid >= k_Bank0 && paramid < (k_Bank0 + 16))
    {
        int chan = paramid - k_Bank0;
        int nbank = (int) value;
        int fontId, prog, obank;
        fluid_synth_get_program(m_synth, chan, &fontId, &obank, &prog);
        fluid_synth_program_select(m_synth, chan, fontId,
                                    nbank, prog);
    }
    else
    if(paramid >= k_Prog0 && paramid < k_Bank0)
    {
        int chan = paramid - k_Prog0;
        int nprog = (int) value;
        int fontId, bank, oprog;
        fluid_synth_get_program(m_synth, chan, &fontId, &bank, &oprog);
        fluid_synth_program_select(m_synth, chan, fontId,
                                    bank, nprog);
    }
    else
    {
        std::cerr << "fluid ERROR: invalid parameter change for "
            << paramid << "\n";
    }
}

bool 
FluidsynthPlugin::paramsValueToText(clap_id paramid, double value, char *display, 
    uint32_t size) noexcept
{
    return false;
}

bool 
FluidsynthPlugin::paramsTextToValue(clap_id paramid, const char *display,   
                                   double *value) noexcept
{
    return false;
}

/**
 * this is invoked by host (plugins can request host to flush via CLAP_EXT_PARAMS)
 * (see clap/ext/params.h)
 */
void 
FluidsynthPlugin::paramsFlush(const clap_input_events *in_events, 
                              const clap_output_events *out) noexcept
{
    std::cerr << "fluid paramsFlush\n";
    const uint32_t nev = in_events->size(in_events);
    if(nev > 0)
    {
        for(uint32_t i=0;i<nev;i++)
        {
            const clap_event_header_t *hdr = in_events->get(in_events, i);
            if(hdr->type == CLAP_EVENT_PARAM_VALUE)
            {
                const clap_event_param_value_t *ev = (const clap_event_param_value_t *)hdr;
                this->setParamValue(ev->param_id, ev->value);
            }
        }
    }
}

bool 
FluidsynthPlugin::presetLoadFromLocation(uint32_t location_kind,
    const char *location, const char *load_key) noexcept
{
    if(location_kind == CLAP_PRESET_DISCOVERY_LOCATION_FILE)
    {
        std::filesystem::path fp(location);
        std::string tmp;
        if(!std::filesystem::exists(fp) && fp.is_relative())
        {
            // try our default locations 
            for(auto x : m_pluginPresetDirs)
            {
                std::filesystem::path nfp = x / fp;
                if(std::filesystem::exists(nfp))
                {
                    tmp = nfp.generic_string();
                    location = tmp.c_str();
                    break;
                }
            }
        }
        int id = fluid_synth_sfload(m_synth,  location, 1/*reset*/);
        if(id == FLUID_FAILED)
        {
            std::cerr << "fluidsynth can't load " << location << "\n";
            return false;
        }
        else
        {
            if(m_verbosity)
                std::cerr << "fluidsynth loaded " << location << "\n";
            m_sfontPath = location;
            m_fontId = id;
            return true;
        }
    }
    else
        return false;
}

#define ckIOError(x)  if(x == -1) return false

#define k_newStateVersion 1

bool 
FluidsynthPlugin::stateSave(const clap_ostream *stream) noexcept 
{
    // stash the current soundfont path, gain and 16 prog/bank values
    // \n separates the 18 values.

    std::string sfpath;
    uint16_t vers = k_newStateVersion;
    if(m_sfontPath.filename().generic_string() == "default.sf2")
        sfpath = "default.sf2";
    else
        sfpath = m_sfontPath.generic_string();

    char buf[8];
    strcpy(buf, "FSPG");
    memcpy(buf+5, &vers, 2);
    buf[7] = '\n';
    ckIOError(stream->write(stream,buf, 8));

    ckIOError(stream->write(stream, sfpath.c_str(), sfpath.size()));
    ckIOError(stream->write(stream, "\n", 1));

    ckIOError(stream->write(stream, &m_gain, sizeof(m_gain)));
    ckIOError(stream->write(stream, "\n", 1));

    // todo: effects parameters

    int fontId; // ignored, currently we keep the same font for all channels
    int bank, prog;
    char bp[3];
    bp[2] = '\n';
    for(int i=0;i<16;i++)
    {
        fluid_synth_get_program(m_synth, i, &fontId, &bank, &prog);
        bp[0] = (char) bank;
        bp[1] = (char) prog;
        ckIOError(stream->write(stream, bp, 3));
    }
    return true;
}

bool 
FluidsynthPlugin::stateLoad(const clap_istream *stream) noexcept
{
    std::string sfpath;
    float gain;
    char buf[8];
    ckIOError(stream->read(stream, buf, 8));
    if(strcmp(buf, "FSPG") != 0)
    {
        std::cerr << "FluidsynthPlugin found bogus state\n";
        return false;
    }

    uint16_t vers;
    memcpy(&vers, buf+5, 2);

    if(vers > k_newStateVersion)
    {
        std::cerr << "FluidsynthPlugin found futuristic state (ignored)\n";
        return false;
    }

    while(1 == stream->read(stream, buf, 1) && buf[0] != '\n')
        sfpath.push_back(buf[0]);

    char const *location = sfpath.c_str();
    int id = fluid_synth_sfload(m_synth,  location, 1/*reset*/);
    if(id == FLUID_FAILED)
    {
        std::cerr << "fluidsynth can't load " << location << "\n";
        return false;
    }
    else
        m_fontId = id;
    ckIOError(stream->read(stream, (char *) &gain, sizeof(gain)));
    m_gain = gain;
    ckIOError(stream->read(stream, buf, 1)); // newline

    for(int i=0;i<16;i++)
    {
        ckIOError(stream->read(stream, buf,  3));
        fluid_synth_program_select(m_synth, i, m_fontId,
                                    buf[0], buf[1]);
    }
    return true;
}
