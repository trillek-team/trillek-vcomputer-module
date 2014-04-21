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

class AlEngine {
public:
    AlEngine();

    ~AlEngine();

    bool Init();
    void Shutdown();

    void Tone(vm::word_t freq);

    void SineSynth (float f);
    void SqrSynth (float f);

    void Test();

    void MasterGain(float gain);
    float MasterGain() const;

private:

    float gain;

    bool initiated;
    bool buff_created;
    bool source_created;

    ALCdevice* device;
    ALCcontext* context;

    // Buffers hold sound data.
    ALuint beep_buff;

    // Sources are points emitting sound.
    ALuint beep_source;

    vm::word_t beep_freq;

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

