/*!
 * \brief       OpenAL Stuff of the test/toy emulator
 * \file        alengine.cpp
 * \copyright   The MIT License (MIT)
 *
 * OpenAL Stuff of the test/toy emulator.
 */

#include "al_engine.hpp"

#include <cmath>
#include <cstdio>

#include <cassert>

#ifdef OPENAL_ENABLE
namespace AlEngine {

    volatile trillek::Word beep_freq;   /// Beep freq
    double offset;      /// Signal phase
    double sign;        /// Square wave sign

    // Blip Buffer stuff
    Blip_Buffer blipbuf;                       //! Blip Buffer
    Blip_Synth<blip_good_quality,20> synth;    //! Synthetizer of Blip Buffer
    // to generate a signal, use  synth.update (time in blip_buffer clock rate cycles, amplitude) with amplite <= 20/2
    ALuint StreamCB (void* userdata, ALubyte* data, ALuint bytes);


    // Position of the source sound.
    const ALfloat AlEngine::SourcePos[] = { 0.0, 0.0, 0.0 };

    // Velocity of the source sound.
    const ALfloat AlEngine::SourceVel[] = { 0.0, 0.0, 0.0 };

    // Position of the listener.
    const ALfloat AlEngine::ListenerPos[] = { 0.0, 0.0, 0.0 };

    // Velocity of the listener.
    const ALfloat AlEngine::ListenerVel[] = { 0.0, 0.0, 0.0 };

    // Orientation of the listener. (first 3 elements are "at", second 3 are "up")
    const ALfloat AlEngine::ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };

    AlEngine::AlEngine () :
        gain(1.0f), initiated(false), source_created(false), stream(nullptr)  {

    }

    AlEngine::~AlEngine () {
        // RAII !
        Shutdown();
    }

    bool AlEngine::Init () {
        if (!alureInitDevice(NULL, NULL)) {
            std::fprintf(stderr, "Failed to open OpenAL device: %s\n", alureGetErrorString());
            return false;
        }

        alListenerf(AL_GAIN, gain);

        // Set the source of the audio
        alGenSources(1, &beep_source);
        if (alGetError() != AL_NO_ERROR) {
            Shutdown();
            return false;
        }

        source_created = true;
        alSourcef (beep_source, AL_PITCH,    1.0f     );
        alSourcef (beep_source, AL_GAIN,     1.0f     );
        alSourcefv(beep_source, AL_POSITION, SourcePos);
        alSourcefv(beep_source, AL_VELOCITY, SourceVel);
        alSourcei (beep_source, AL_LOOPING,  false    );

        // Set listener parameters
        alListenerfv(AL_POSITION,    ListenerPos);
        alListenerfv(AL_VELOCITY,    ListenerVel);
        alListenerfv(AL_ORIENTATION, ListenerOri);

        // Init Blip Buffer with a buffer of 500ms
        if ( blipbuf.set_sample_rate( SR, 1000 / 4 ) ) {
            std::fprintf(stderr, "Failed to create Blip Buffer! Our of Memory\n");
            Shutdown();
            return false;
        }
        blipbuf.clock_rate( blipbuf.sample_rate() );

        blipbuf.bass_freq(300); // Equalization like a TV speaker
        synth.treble_eq( -8.0f ); // Synthetize Equalization

        synth.volume (0.30);
        synth.output (&blipbuf);

        beep_freq = 0;
        offset = 0;
        sign = 1;

        stream = alureCreateStreamFromCallback (StreamCB, nullptr, AL_FORMAT_MONO16, SR, SR/2, 0, nullptr);
        if (!stream) {
            Shutdown();
            return false;
        }

        initiated = true;
        return true;
    }

    void AlEngine::Shutdown () {
        if (source_created) {
            alDeleteSources(1, &beep_source);
            source_created = false;
        }
        if (stream) {
            alureDestroyStream(stream, 0, nullptr);
            stream = nullptr;
        }

        alureShutdownDevice();
        initiated = false;
        std::fprintf(stderr, "OpenAL closed\n");
    }

    void AlEngine::Tone(trillek::Word freq) {
        if (initiated) {
            beep_freq = freq;
        }
    }

    void AlEngine::Update () {
        if (initiated) {
            alureUpdate();
        }
    }

    void AlEngine::Play () {
        if (initiated) {
            if (!alurePlaySourceStream(beep_source, stream, AL_BUFFERS, 0, nullptr, nullptr)) {
                std::fprintf(stderr, "Failed to play stream: %s\n", alureGetErrorString());
            }
        }
    }

    void AlEngine::Pause () {
        if (initiated) {
            alurePauseSource(beep_source);
        }
    }

    void AlEngine::Stop () {
        if (initiated) {
            alureStopSource(beep_source, AL_FALSE);
        }
    }

    void AlEngine::Test() {
        if (initiated) {
            beep_freq = 937;
            std::fprintf(stderr, "OpenAL Test begin\n");
            if (!alurePlaySourceStream(beep_source, stream, 3, 0, nullptr, nullptr)) {
                std::fprintf(stderr, "Failed to play stream: %s\n", alureGetErrorString());
            }
            alureUpdate();
            alureSleep(0.1f);
            alureUpdate();
            alureSleep(0.1f);
            alureUpdate();
            alureSleep(0.1f);
            alureUpdate();
            alureSleep(0.1f);
            alureUpdate();
            alureSleep(0.1f);

            alureStopSource(beep_source, AL_FALSE);
            std::fprintf(stderr, "OpenAL Test end\n");
        }
    }

    void AlEngine::MasterGain (float gain) {
        assert (gain >= 0);
        if (initiated) {
            alListenerf(AL_GAIN, gain);
        }
        this->gain = gain;
    }

    float AlEngine::MasterGain () const {
        return gain;
    }

    // Callback called when Alure needs more data to ffed a buffer
    ALuint StreamCB (void* userdata, ALubyte *data, ALuint bytes) {
        size_t lenght = bytes / 2;  // Lenght in "samples"

        if (beep_freq > 0) {
            double period = SR / (2* beep_freq);  // How many samples need to do a half cycle.

            unsigned const amplitude = 9;
            while (offset < lenght) {
                sign = - sign;
                synth.update (offset, amplitude * sign);
                offset += period;
            }
            blipbuf.end_frame(lenght);
            offset -= lenght; // adjust time to new frame
        } else {
            blipbuf.end_frame(lenght);
            offset = 0;
        }

        ALuint out = 2 * blipbuf.read_samples ((blip_sample_t*) data, lenght ); // return bytes!
        //std::fprintf(stderr, "Generating! %u\n", out);
        return out;
    }


} // End of Namespace AlEngine

#endif

