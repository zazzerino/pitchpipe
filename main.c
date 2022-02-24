#include <stdio.h>
#include <math.h>
#include <portaudio.h>
#include <malloc.h>

#define SAMPLE_RATE (44100)

// frequency = sample_rate / table_size

struct sine_data
{
    size_t table_size;
    float *table;
    size_t left_phase;
    size_t right_phase;
};

static int stream_callback(const void *input,
                           void *output,
                           unsigned long frame_count,
                           const PaStreamCallbackTimeInfo *time_info,
                           PaStreamFlags status_flags,
                           void *user_data)
{
    struct sine_data *data = (struct sine_data*) user_data;
    float *out = (float*) output;

    for (size_t i = 0; i < frame_count; i++) {
        *out++ = data->table[data->left_phase];
        *out++ = data->table[data->right_phase];

        if (++data->left_phase >= data->table_size) {
            data->left_phase -= data->table_size;
        }

        if (++data->right_phase >= data->table_size) {
            data->right_phase -= data->table_size;
        }
    }

    return paContinue;
}

static void stream_finished(void *user_data)
{
    puts("stream completed");
}

int main(int argc, char **argv) {
    PaStreamParameters output_params;
    PaStream *stream;
    PaError err;

    const size_t table_size = 100;
    const unsigned int seconds = 4;
    const float volume = 0.8f;

    struct sine_data data;
    data.table_size = table_size;
    data.left_phase = data.right_phase = 0;
    data.table = malloc(sizeof(float) * table_size);

    for (size_t i = 0; i < table_size; i++) {
        float val = (float) sin(((double) i / (double) table_size) * M_PI * 2.0f);
        data.table[i] = val * volume;
    }

    err = Pa_Initialize();
    if (err != paNoError) goto error;

    output_params.device = Pa_GetDefaultOutputDevice();
    if (output_params.device == paNoDevice) {
        fprintf(stderr,"error: no default output device.\n");
        goto error;
    }
    output_params.channelCount = 2;
    output_params.sampleFormat = paFloat32;
    output_params.suggestedLatency = Pa_GetDeviceInfo(output_params.device)->defaultLowOutputLatency;
    output_params.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
            &stream,
            NULL,
            &output_params,
            SAMPLE_RATE,
            paFramesPerBufferUnspecified,
            paNoFlag,
            stream_callback,
            &data);
    if (err != paNoError) goto error;

    err = Pa_SetStreamFinishedCallback(stream, &stream_finished);
    if (err != paNoError) goto error;

    if ((err = Pa_StartStream(stream)) != paNoError) goto error;

    printf("playing for %d seconds\n", seconds);
    Pa_Sleep(seconds * 1000 );

    if ((err = Pa_StopStream(stream)) != paNoError) goto error;
    if ((err = Pa_CloseStream(stream)) != paNoError) goto error;

    Pa_Terminate();
    printf("pa closed\n");

    free(data.table);
    return err;

error:
    Pa_Terminate();
    fprintf(stderr, "an error occurred while using the portaudio stream\n");
    fprintf(stderr, "error number: %d\n", err);
    fprintf(stderr, "error message: %s\n", Pa_GetErrorText(err));
    free(data.table);
    return err;
}
