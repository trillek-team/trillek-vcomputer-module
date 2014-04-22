#pragma once
/*!
 * \brief       OpenAL Stuff of the test/toy emulator
 * \file        AlEngine.hpp
 * \copyright   The MIT License (MIT)
 *
 * OpenAL Stuff of the test/toy emulator.
 */

#include "config_main.hpp"

#include <memory>

#ifdef OPENAL_ENABLE

#include "Types.hpp"

const static unsigned AL_BUFFERS = 2;

class AlEngine {
public:
    AlEngine();

    ~AlEngine();

    bool Init();
    void Shutdown();

    void Tone(vm::word_t freq);

    void Update();

    void SineSynth (float f);
    void SqrSynth (float f);

    void Test();

    void MasterGain(float gain);
    float MasterGain() const;

private:

    float gain;             //! Master volumen

    bool initiated;         //! Flag to know if we properly initialized all
    bool buff_created;      //! Audio Buffers created
    bool source_created;    //! Audio Source created

    ALCdevice* device;      //! OpenAL device
    ALCcontext* context;    //! OpenAL context

    ALuint beep_buff[AL_BUFFERS];   //! Buffers with sound data
    unsigned play_buff;     //! Index to the buffer being played

    ALuint beep_source;     //! Source point of the sound

    vm::word_t beep_freq;   //! Beep freq

    // Position of the source sound.
    const static ALfloat SourcePos[];

    // Velocity of the source sound.
    const static ALfloat SourceVel[];

    // Position of the listener.
    const static ALfloat ListenerPos[];

    // Velocity of the listener.
    const static ALfloat ListenerVel[];

    // Orientation of the listener. (first 3 elements are "at", second 3 are "up")
    const static ALfloat ListenerOri[];

};

#endif

