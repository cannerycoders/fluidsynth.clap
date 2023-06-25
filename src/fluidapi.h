#include <clap/clap.h>
#include <fluidsynth.h>

#include <stdio.h>
#include <limits.h>
#include <iostream>

// https://www.fluidsynth.org/api/modules.html
class FluidSynth
{
public:
    // constructor
    FluidSynth(float fs)
    {
        m_srate = fs;
        m_fontId = -1;

        m_settings = new_fluid_settings();
        fluid_settings_setnum(m_settings, "synth.sample-rate", m_srate);

        // https://www.fluidsynth.org/api/CreatingSettings.html

        // midi-bank-select superceded selectPreset(bank, voice, channel)
        // gs: (default) CC0 becomes the bank number, CC32 is ignored.
        // gm: ignores CC0 and CC32 messages.
        // xg: CC32 becomes the bank number, CC0 toggles between melodic or drum channel.
        // mma: bank is calculated as CC0*128+CC32.
        // fluid_settings_setstr(m_settings, "synth.midi-bank-select", "mma");

        m_synth = new_fluid_synth(m_settings);
    }

    ~FluidSynth()
    {
        delete_fluid_synth(m_synth);
        m_synth = NULL;

        delete_fluid_settings(m_settings);
        m_settings = NULL;
    }

    void setVerbosity(int v)
    {
        fluid_settings_setint(m_settings, "synth.verbose", v);
        if(m_synth)
        {
            delete_fluid_synth(m_synth);
            m_synth = NULL;
        }
        m_synth = new_fluid_synth(m_settings);
    }

    #if 0
    // ugen: (0 in, 2 out)
    void tick( SAMPLE *in, SAMPLE *out )
    {
        fluid_synth_write_float(m_synth, 1, out, 0, 0, out+1, 0, 0);
    }
    #endif

    int open(const std::string &sfont)
    {
        m_fontId = fluid_synth_sfload(m_synth, sfont.c_str(), 1);
        return m_fontId;
    }

    void noteOn(int chan, int key, int vel)
    {
        fluid_synth_noteon(m_synth, chan, key, vel);
    }

    void noteOff(int chan, int key)
    {
        fluid_synth_noteoff(m_synth, chan, key);
    }

    void cc(int chan, int cc, int val)
    {
        fluid_synth_cc(m_synth, chan, cc, val);
    }

    void progChange(int chan, int progNum)
    {
        fluid_synth_program_change(m_synth, chan, progNum);
    }

    void setBank(int chan, int bankNum)
    {
        fluid_synth_bank_select(m_synth, chan, bankNum);
    }

    /* selectPreset combines setBank and progChange and offers
     * the advantage of establishing the correct channel type.
     */
    int selectPreset(int bank, int prog, int chan)
    {
        fluid_synth_set_channel_type(m_synth, chan,     
                (bank >= 128) ? CHANNEL_TYPE_DRUM : CHANNEL_TYPE_MELODIC);
        int result = fluid_synth_program_select(m_synth, chan, m_fontId, bank, prog);
        return 0;
    }

    #if 0
    void setTuning(int chan, Chuck_Array8 * tuning)
    {
        bool allChans = false;
        if (chan < 0) {
            allChans = true;
            chan = 0;
        }
        fluid_synth_activate_key_tuning(m_synth, 0, chan, "", &tuning->m_vector[0], false);
        if (allChans) {
            for (chan = 0 ; chan<16 ; chan++) {
                fluid_synth_activate_tuning(m_synth, chan, 0, 0, false);
            }
        } else {
            fluid_synth_activate_tuning(m_synth, chan, 0, chan, false);
        }
    }
    #endif

    #if 0
    void setOctaveTuning(int chan, Chuck_Array8 * tuning)
    {
        bool allChans = false;
        if (chan < 0) {
            allChans = true;
            chan = 0;
        }
        fluid_synth_activate_octave_tuning(m_synth, 0, chan, "", &tuning->m_vector[0], false);
        if (allChans) {
            for (chan = 0 ; chan<16 ; chan++) {
                fluid_synth_activate_tuning(m_synth, chan, 0, 0, false);
            }
        } else {
            fluid_synth_activate_tuning(m_synth, chan, 0, chan, false);
        }
    }
    #endif

    void resetTuning(int chan)
    {
        if (chan < 0) {
            for (chan = 0 ; chan<16 ; chan++) {
                fluid_synth_deactivate_tuning(m_synth, chan, false);
            }
        } else {
            fluid_synth_deactivate_tuning(m_synth, chan, false);
        }
    }

    void tuneNote(int noteNum, double pitch, int chan)
    {
        fluid_synth_tune_notes(m_synth, 0, chan, 1, &noteNum , &pitch, false);
    }

    #if 0
    void tuneNotes(Chuck_Array4 * noteNums, Chuck_Array8 * pitches, int chan)
    {
        /* 
        This ugly hack is required because Chuck_Array4 doesn't actually
        contain 4-byte ints (at least on my 64-bit linux system). So we 
        need to copy the elements into an int array.
        */
        int * noteNumArr;
        noteNumArr = new int[noteNums->size()];
        for (int i = 0; i < noteNums->size(); i++) {
            noteNumArr[i] = (int) noteNums->m_vector[i];
        }
        fluid_synth_tune_notes(m_synth, 0, chan, pitches->size(),
                               noteNumArr, &pitches->m_vector[0], false);

        delete [] noteNumArr;
    }
    #endif

    void setPitchBend(int pitchbend, int chan)
    {
        fluid_synth_pitch_bend(m_synth, chan, pitchbend);
    }

    int getPitchBend(int chan)
    {
        int pitchbend;
        fluid_synth_get_pitch_bend(m_synth, chan, &pitchbend);
        return pitchbend;
    }

private:
    // instance data
    float m_srate;
    fluid_settings_t *m_settings;
    fluid_synth_t *m_synth;
    int m_fontId;
};
