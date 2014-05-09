#pragma once
/*!
 * \brief       OpenAL/Alure Stuff of the test/toy emulator
 * \file        al_engine.hpp
 * \copyright   The MIT License (MIT)
 *
 * OpenAL Stuff of the test/toy emulator.
 */

#include "config_main.hpp"

#include <memory>

#ifdef OPENAL_ENABLE

#ifdef __APPLE__
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif
#include <AL/alure.h> // Check if in MacOS or Win needs other stuff here


#include "types.hpp"

#include "Blip_Buffer.h"

namespace AlEngine {

    const static unsigned AL_BUFFERS = 4;
    const static unsigned SR = 44100;               //! Sampling rate

    class AlEngine {
    public:
        AlEngine();

        ~AlEngine();

        bool Init();
        void Shutdown();

        void Tone(vm::word_t freq);

        void Update();

        void Play();
        void Pause();
        void Stop();

        void Test();

        void MasterGain(float gain);
        float MasterGain() const;

    private:


        float gain;             /// Master volumen

        bool initiated;         /// Flag to know if we properly initialized all
        bool source_created;    /// Flag to know if we create sound source

        ALuint beep_source;     /// Source point of the sound

        alureStream* stream;    /// Alure Sound stream

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

} // End of Namespace AlEngine

#endif

