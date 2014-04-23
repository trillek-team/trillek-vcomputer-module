/*!
 * \brief       OpenAL Stuff of the test/toy emulator
 * \file        AlEngine.cpp
 * \copyright   The MIT License (MIT)
 *
 * OpenAL Stuff of the test/toy emulator.
 */

#include "AlEngine.hpp"

#include <cmath>
#include <cstdio>

#include <cassert>

#ifdef OPENAL_ENABLE

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
    gain(1.0f), initiated(false), device(nullptr), context(nullptr), beep_freq(0) {

    synth.treble_eq( -8.0f ); // Synthetize Equalization

    synth.volume (0.80);
    synth.output (&blipbuf);
}

AlEngine::~AlEngine () {
    // RAII !
    Shutdown();
}

bool AlEngine::Init () {
    device = alcOpenDevice(nullptr);
    if (device == nullptr) {
        return false;
    }

    context = alcCreateContext(device, nullptr);
    if (context == nullptr) {
        Shutdown();
        return false;
    }

	if(!alcMakeContextCurrent(context)) {
        Shutdown();
        return false;
    }

    alListenerf(AL_GAIN, gain);

    alGenBuffers(AL_BUFFERS, beep_buff);
    if (alGetError() != AL_NO_ERROR || ! alIsBuffer(beep_buff[0]) || ! alIsBuffer(beep_buff[1]) ) {
        Shutdown();
        return false;
    }
    buff_created = true;

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

    //// Set listener parameters
    alListenerfv(AL_POSITION,    ListenerPos);
    alListenerfv(AL_VELOCITY,    ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);

    play_buff = 0; //We use initially buffer 0

    // Init Blip Buffer with a buffer of 500ms
    if ( blipbuf.set_sample_rate( SR, 1000 / 4 ) ) {
        Shutdown();
        return false;
    }
    blipbuf.clock_rate( blipbuf.sample_rate() );

    blipbuf.bass_freq(180); // Equalization like a TV speaker
    time = 0;

    initiated = true;
    return true;
}

void AlEngine::Shutdown () {
    if (source_created) {
        alDeleteSources(1, &beep_source);
        source_created = false;
    }

    if (buff_created) {
        alDeleteBuffers(AL_BUFFERS, beep_buff);
        buff_created = false;
    }

    if (context != nullptr) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        context = nullptr;
    }

    if (device != nullptr) {
        alcCloseDevice(device);
        device = nullptr;
    }

    initiated = false;
    std::fprintf(stderr, "OpenAL closed\n");
}

void AlEngine::Tone(vm::word_t freq) {
    if (initiated) {
        ALint availBuffers = 0; // Gets number of buffers played
        alGetSourcei(beep_source, AL_BUFFERS_PROCESSED, &availBuffers);

        if (freq != beep_freq && freq > 0) {
            // Change of tone, we must stop and unqueue all the buffers now!
            alSourceStop(beep_source);
            alGetSourcei(beep_source, AL_BUFFERS_PROCESSED, &availBuffers);
            alSourceUnqueueBuffers(beep_source, availBuffers, beep_buff);

            for (play_buff = 0; play_buff < AL_BUFFERS; play_buff++) {
                SqrSynth(freq);
            }
            play_buff = 0;
            alSourceQueueBuffers(beep_source, AL_BUFFERS, beep_buff);

            alSourcePlay(beep_source);
        } else {
            // Freq 0, so we stops
            alSourceStop(beep_source);
            alGetSourcei(beep_source, AL_BUFFERS_PROCESSED, &availBuffers);
            alSourceUnqueueBuffers(beep_source, availBuffers, beep_buff);
        }

        beep_freq = freq;
    }

}

void AlEngine::Update () {
    if (initiated && beep_freq > 0) {
        ALint availBuffers = 0; // Gets number of buffers played
        alGetSourcei(beep_source, AL_BUFFERS_PROCESSED, &availBuffers);

        // Keeps the tone
        if (availBuffers > 0) {
            // We only swaps buffers when one is procesed
            alSourceUnqueueBuffers(beep_source, availBuffers, beep_buff);

            int i;
            for (i= 0; play_buff < AL_BUFFERS && i < availBuffers; play_buff++, i++) {
                SqrSynth(beep_freq);
            }
            alSourceQueueBuffers(beep_source, i, beep_buff + play_buff);

            // Restart the source if needed
            ALint sState=0;
            alGetSourcei(beep_source, AL_SOURCE_STATE, &sState);
            if (sState!=AL_PLAYING) {
                alSourcePlay(beep_source);
            }
        }
    }
}

void AlEngine::SineSynth(float f) {
    if (initiated) {
        const double w = f * PI2; // Convert to angular freq.
        const double dt = 1.0f / SR; // Time in seconds of a single sample

        unsigned char buf[11025]; // 11025 samples @ 44100 Hz of sampling rate -> 250 ms of audio
        double x;
        for(int i = 0; i < 11025; i++) {
            x = w*(i*dt) + phase; // x = wt
            buf[i] = (unsigned char)(128.0f + 128.f * std::sin(x) );
        }
        phase = x;
        if (phase > PI2 ) {
            phase -= PI2;
        }

        alBufferData(beep_buff[play_buff], AL_FORMAT_MONO8, buf, 11025, SR);
    }
}

void AlEngine::SqrSynth (float f) {
    if (initiated) {
        //const double    dt = 1.0f / SR;
        const unsigned  period = SR / (2*f);    // How many samples need to do a half cycle.
        const unsigned  lenght = 10000; // 10000 time units of samples

        unsigned amplitude = 10;
        while (time < lenght) {
            amplitude = -amplitude;
            synth.update(time, amplitude);
            time += period;
        }
        blipbuf.end_frame(lenght);
        time -= lenght; // adjust time to new frame

        blip_sample_t* samples = new blip_sample_t[lenght + 100];

        int count = blipbuf.read_samples( samples, lenght + 100);
        alBufferData (beep_buff[play_buff], AL_FORMAT_MONO16, samples, count, SR);

        delete[] samples;
    }
}

void AlEngine::Test() {
    if (initiated) {
        play_buff = (play_buff + 1) % 2;
        //SineSynth(440.0f); // LA
        SqrSynth(932.0f); // BIOS beep

        alSourceQueueBuffers(beep_source, 1, beep_buff + play_buff);
        alSourcePlay(beep_source);
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


#endif

