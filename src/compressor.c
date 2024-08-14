#include "compressor.h"

#include <stdlib.h>
#include <pthread.h>
#include <threads.h>
#include <assert.h>

#include "util.h"

Compressor* compressor_create() {
    Compressor* compressor = calloc(1, sizeof(*compressor));

    compressor->params = (CompressorParams) {
        .input_gain = {
            .common = {
                .id = PARAM_ID_INPUT_GAIN,
                .name = "Input Gain",
            },
            .value = db_to_gain(-10.0f),
            .default_value = db_to_gain(-10.0f),
            .min = db_to_gain(-30.0f),
            .max = db_to_gain(30.0f),
        },

        .output_gain = {
            .common = {
                .id = PARAM_ID_OUTPUT_GAIN,
                .name = "Output Gain",
            },
            .value = db_to_gain(-10.0f),
            .default_value = db_to_gain(-10.0f),
            .min = db_to_gain(-30.0f),
            .max = db_to_gain(30.0f),
        },

        .mix = { 
            .common = {
                .id = PARAM_ID_MIX,
                .name = "Mix",
            },
            .default_value = 1,
            .value = 1,
        },
    };

    return compressor;
}

void compressor_process(Compressor* compressor, Buffer* buffer) {
    float gain = compressor->params.input_gain.value;

    for (size_t i = 0; i < buffer->frame_count; i++) {
        for (size_t channel = 0; channel < buffer->channel_count; channel++) {
            buffer->data[channel][i] *= gain;
        }
    }
}
