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

    alGenBuffers(1, &beep_buff);
    if (alGetError() != AL_NO_ERROR || ! alIsBuffer(beep_buff)) {
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
    alSourcei (beep_source, AL_LOOPING,  true     );

    //// Set listener parameters
    alListenerfv(AL_POSITION,    ListenerPos);
    alListenerfv(AL_VELOCITY,    ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);

    initiated = true;
    return true;
}

void AlEngine::Shutdown () {
    if (source_created) {
        alDeleteSources(1, &beep_source);
        source_created = false;
    }

    if (buff_created) {
        alDeleteBuffers(1, &beep_buff);
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
        if (freq != beep_freq) {
            ALuint b[1];
            b[0] = beep_buff;

            alSourceStop(beep_source);
            alSourceUnqueueBuffers(beep_source,1, b);

            if (freq > 0 ) {
                SqrSynth(freq);

                alSourceQueueBuffers(beep_source, 1, b);
                alSourcePlay(beep_source);
            }
            beep_freq = freq;
        }
    }
}

void AlEngine::SineSynth(float f) {
    if (initiated) {
        double w = f * 2.0f * 3.14159679f; // Convert to angular freq.
        const double dt = 1.0f / 44100;
        int i;
        double x;
        unsigned char buf[11025]; // 11025 samples @ 44100 Hz of sampling rate -> 250 ms of audio
        for(i = 0; i < 11025; i++) {
            x = (i*dt) * w; // x = t * w
            buf[i] = (unsigned char)(128.0f + 128.f * std::sin(x) );
        }

        alBufferData(beep_buff, AL_FORMAT_MONO8, buf, 11025, 44100);
    }
}

void AlEngine::SqrSynth (float f) {
    const uint32_t sr_2 = 44100 /2; // Nyquist limmit
    const double dt = 1.0f / 44100;
    if (initiated) {
        double w = f * 2.0f * 3.14159679f; // Convert to angular freq.
        int i;
        unsigned char buf[11025]; // 11025 samples @ 44100 Hz of sampling rate -> 250 ms of audio
        for(i = 0; i < 11025; i++) {
            double out =  std::sin( i*dt * w); // Base signal
            // We must avoid add harmonics over Nyquist limit or will be alised and
            // will sound like strange noise mixed with the signal
            if (f*3 < sr_2) { // Third armonic
                out += std::sin(i*dt * w*3) / 3.0;
            }
            else if (f*5 < sr_2) { // Fith armonic
                out += std::sin(i*dt * w*5) / 5.0;
            }
            else if (f*7 < sr_2) { // Seventh armonic
                out += std::sin(i*dt * w*7) / 7.0;
            }
            else if (f*9 < sr_2) { // Ninth armonic
                out += std::sin(i*dt * w*9) / 9.0;
            }
            else if (f*11 < sr_2) { // Eleventh armonic
                out += std::sin(i*dt * w*11) / 11.0;
            }
            else if (f*13 < sr_2) { // Thirteenth armonic
                out += std::sin(i*dt * w*13) / 13.0;
            }
            // Note, for a perfect aquare wave, should be to 49º armonic for a 440Hz note !!
            // This shuld e repalced by a BLIP/BLEP synth or WaveTable Synth

            buf[i] = (unsigned char)(128.0f + 128.f * out);
        }
        alBufferData(beep_buff, AL_FORMAT_MONO8, buf, 11025, 44100);
    }
}

void AlEngine::Test() {
    if (initiated) {
        ALuint b[1];
        b[0] = beep_buff;
        //SineSynth(440.0f); // DO
        SqrSynth(1000.0f);

        alSourceQueueBuffers(beep_source, 1, b);
        alSourcei(beep_source, AL_LOOPING, AL_FALSE);
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

