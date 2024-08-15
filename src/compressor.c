#include "compressor.h"

#include <clap/events.h>
#include <stdlib.h>
#include <pthread.h>
#include <threads.h>
#include <assert.h>

#include "util.h"

Compressor* compressor_create() {
    Compressor* compressor = calloc(1, sizeof(*compressor));

    compressor->params = (CompressorParams) {
        .threshold = gain_param(PARAM_ID_THRESHOLD, "Threshold", 0.0f, -30.0f, 10.0f),
        .attack = float_param(PARAM_ID_ATTACK, "Attack", 10, 1, 100, " ms"),
        .release = float_param(PARAM_ID_RELEASE, "Release", 10, 1, 100, " ms"),
        .ratio = float_param(PARAM_ID_RATIO, "ratio", 1, 0, 4, ""),
        .output_gain = gain_param(PARAM_ID_OUTPUT_GAIN, "Output", 0.0f, -30.0f, 30.0f),
        .mix = percent_param(PARAM_ID_MIX, "Mix", 1),
    };

    compressor->params.param_map[PARAM_ID_THRESHOLD] = (Param*) &compressor->params.threshold;
    compressor->params.param_map[PARAM_ID_ATTACK] = (Param*) &compressor->params.attack;
    compressor->params.param_map[PARAM_ID_RELEASE] = (Param*) &compressor->params.release;
    compressor->params.param_map[PARAM_ID_RATIO] = (Param*) &compressor->params.ratio;
    compressor->params.param_map[PARAM_ID_OUTPUT_GAIN] = (Param*) &compressor->params.output_gain;
    compressor->params.param_map[PARAM_ID_MIX] = (Param*) &compressor->params.mix;

    return compressor;
}

void compressor_process(Compressor* compressor, Buffer* buffer) {
    float output_gain = compressor->params.output_gain.value;
    float mix = compressor->params.mix.value;

    for (size_t i = 0; i < buffer->frame_count; i++) {
        for (size_t channel = 0; channel < buffer->channel_count; channel++) {
            const float input = buffer->data[channel][i];

            float s = input;
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
            if(!params_is_valid_id(event->param_id)) {
                break;
            }

            Param* p = compressor->params.param_map[event->param_id];
            p->methods.set_value(p, event->value);
        } break;
    }
}
