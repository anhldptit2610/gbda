// #include <stdio.h>
// #include <SDL2/SDL.h>

// int main(void) {
//     SDL_Init(SDL_INIT_AUDIO);

//     // the representation of our audio device in SDL:
//     SDL_AudioDeviceID audio_device;

//     // opening an audio device:
//     SDL_AudioSpec audio_spec;
//     SDL_zero(audio_spec);
//     audio_spec.freq = 44100;
//     audio_spec.format = AUDIO_S16SYS;
//     audio_spec.channels = 1;
//     audio_spec.samples = 1024;
//     audio_spec.callback = NULL;

//     audio_device = SDL_OpenAudioDevice(
//         NULL, 0, &audio_spec, NULL, 0);

//     // pushing 3 seconds of samples to the audio buffer:
//     float x = 0;
//     for (int i = 0; i < audio_spec.freq * 3; i++) {
//         x += .010f;

//         // SDL_QueueAudio expects a signed 16-bit value
//         // note: "5000" here is just gain so that we will hear something
//         int16_t sample = sin(x * 4) * 5000;

//         const int sample_size = sizeof(int16_t) * 1;
//         SDL_QueueAudio(audio_device, &sample, sample_size);
//     }

//     // unpausing the audio device (starts playing):
//     SDL_PauseAudioDevice(audio_device, 0);

//     SDL_Delay(6000);

//     SDL_CloseAudioDevice(audio_device);
//     SDL_Quit();

//     return 0;
// }
#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

// Constants for audio settings
#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2
#define AMPLITUDE 32767
#define FREQUENCY 440.0 // Hz
#define BUFFER_SIZE 4096 // Buffer size

// Function to generate PCM samples (sine wave)
void generate_sine_wave(Uint8 *stream, int len) {
    double t = 0.0;
    for (int i = 0; i < len; i += sizeof(int16_t) * NUM_CHANNELS) {
        int16_t sample = (int16_t)(AMPLITUDE * sin(2.0 * M_PI * FREQUENCY * t));

        // Write the sample to the stream (assuming 16-bit signed PCM)
        for (int ch = 0; ch < NUM_CHANNELS; ch++) {
            stream[i + ch * sizeof(int16_t)] = (Uint8)sample;
            stream[i + ch * sizeof(int16_t) + 1] = (Uint8)(sample >> 8);
        }

        t += 1.0 / SAMPLE_RATE;
    }
}

int main() {
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec desired_spec, obtained_spec;
    desired_spec.freq = SAMPLE_RATE;
    desired_spec.format = AUDIO_S16SYS; // Signed 16-bit PCM
    desired_spec.channels = NUM_CHANNELS;
    desired_spec.samples = BUFFER_SIZE; // Buffer size
    desired_spec.callback = NULL;
    desired_spec.userdata = NULL;

    SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &obtained_spec, 0);
    if (audio_device == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        return 1;
    }

    // Start audio playback
    SDL_PauseAudioDevice(audio_device, 0);

    // Generate and queue audio data
    bool done = false;
    while (!done) {
        // handle SDL event
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                done = true; 
        }

        Uint8 buffer[BUFFER_SIZE];
        generate_sine_wave(buffer, BUFFER_SIZE);
        SDL_QueueAudio(audio_device, buffer, BUFFER_SIZE);
        SDL_Delay((BUFFER_SIZE * 1000) / (SAMPLE_RATE * NUM_CHANNELS * sizeof(int16_t)));
    }

    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();

    return 0;
}
