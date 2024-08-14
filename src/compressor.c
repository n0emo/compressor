#include "compressor.h"

#include <clap/events.h>
#include <stdio.h>
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
    float input_gain = compressor->params.input_gain.value;
    float output_gain = compressor->params.output_gain.value;
    float mix = compressor->params.mix.value;

    for (size_t i = 0; i < buffer->frame_count; i++) {
        for (size_t channel = 0; channel < buffer->channel_count; channel++) {
            const float input = buffer->data[channel][i];

            float s = input;
            s *= input_gain;
            s *= output_gain;

            const float output = s;

            buffer->data[channel][i] = lerp(output, input, mix); 
        }
    }
}

void compressor_handle_clap_event(Compressor* compressor, const clap_event_header_t* event_header) {
    switch(event_header->type) {
        case CLAP_EVENT_PARAM_VALUE: {
            const clap_event_param_value_t* event = (const clap_event_param_value_t*) event_header;
            params_set_value(&compressor->params, event->param_id, event->value);
        } break;
    }
}
