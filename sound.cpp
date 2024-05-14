#include "PlutoPilot.h"
#include "Peripheral.h"
#include "Print.h"
#include "Control.h"
#include "Althold.h"
#include "Led.h"
#include "portaudio.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>

#define SAMPLE_RATE (44100)
#define NUM_CHANNELS (1)
#define FRAMES_PER_BUFFER (64)
#define SOUND_DETECTION_INTERVAL (2)  // Interval for sound detection in seconds

float normalized_rms = 0.0;  // Global variable to store normalized sound level

int audioCallback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *userData)
{
    float *input = (float *)inputBuffer;
    float rms = 0.0;

    for (int i = 0; i < framesPerBuffer; i++)
    {
        rms += (*input) * (*input);
        input++;
    }

    rms = sqrt(rms / framesPerBuffer);

    normalized_rms = std::min(1.0f, std::max(0.0f, rms / 0.1f)); 
    return paContinue;
}

void plutoInit()
{
    ADC.Init(Pin13); 

    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        Print.monitor("PortAudio error: ");
        Print.monitor(Pa_GetErrorText(err));
        Print.monitor("\n");
    }

    PaStream *stream;
    err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, 0, paFloat32, SAMPLE_RATE,
                               FRAMES_PER_BUFFER, audioCallback, NULL);
    if (err != paNoError)
    {
        Print.monitor("PortAudio error: ");
        Print.monitor(Pa_GetErrorText(err));
        Print.monitor("\n");
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        Print.monitor("PortAudio error: ");
        Print.monitor(Pa_GetErrorText(err));
        Print.monitor("\n");
    }
}

void onPilotStart()
{
    Control.disableFlightStatus(true);
}

void plutoPilot()
{
    static std::chrono::steady_clock::time_point lastSoundDetectionTime = std::chrono::steady_clock::now();

    auto currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastSoundDetectionTime).count();
    if (elapsedTime >= SOUND_DETECTION_INTERVAL)
    {
        lastSoundDetectionTime = currentTime;

        int16_t sensorValue13 = (ADC.Read(Pin13) * 340) / 4096;

        Print.monitor("ADC13:", sensorValue13);
        Print.monitor("\n");

        if (sensorValue13 >= 15)
        {
            ledOp(L_LEFT, ON);
            ledOp(L_RIGHT, OFF);

            // Take off
            Control.arm();
            Althold.setRelativeAltholdHeight(normalized_rms * 100); // Adjust altitude based on normalized sound level
        }
    }
}

void onPilotFinish()
{
    Control.disableFlightStatus(false);

    PaError err = Pa_Terminate();
    if (err != paNoError)
    {
        Print.monitor("PortAudio error: ");
        Print.monitor(Pa_GetErrorText(err));
        Print.monitor("\n");
    }
}
