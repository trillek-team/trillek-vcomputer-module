/*!
 * \brief       OpenAL Stuff of the test/toy emulator
 * \file        AlEngine.cpp
 * \copyright   The MIT License (MIT)
 *
 * OpenAL Stuff of the test/toy emulator.
 */

#include "AlEngine.hpp"


#include <cstdio>

#include <cassert>

#ifdef OPENAL_ENABLE

// Position of the source sound.
const ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };

// Velocity of the source sound.
const ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };

// Position of the listener.
const ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };

// Velocity of the listener.
const ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };

// Orientation of the listener. (first 3 elements are "at", second 3 are "up")
const ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };

AlEngine::AlEngine () :
    gain(1.0f), initiated(false), device(nullptr), context(nullptr) {
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
    initiated = true;
    return true;
}

void AlEngine::Shutdown () {
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

bool AlEngine::Update () {
    return true;
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

