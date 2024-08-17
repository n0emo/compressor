#include "compressor.h"

#include <clap/events.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <threads.h>
#include <assert.h>
#include <math.h>

#ifndef M_E
#   define M_E 2.7182818284590452354
#endif

#include "util.h"


Compressor* compressor_create() {
    Compressor* compressor = calloc(1, sizeof(*compressor));

    compressor->params = (CompressorParams) {
        .threshold = float_param(PARAM_ID_THRESHOLD, "Threshold", 0.0f, -30.0f, 10.0f, " dB"),
        .attack = float_param(PARAM_ID_ATTACK, "Attack", 10, 1, 1000, " ms"),
        .release = float_param(PARAM_ID_RELEASE, "Release", 10, 1, 1000, " ms"),
        .ratio = float_param(PARAM_ID_RATIO, "ratio", 2, 1, 4, ""),
        .output_gain = gain_param(PARAM_ID_OUTPUT_GAIN, "Output", 0.0f, -30.0f, 30.0f),
        .mix = percent_param(PARAM_ID_MIX, "Mix", 1),
        .interpolator = float_param(PARAM_ID_INTERPOLATOR, "Interpolator", 0.0f, 0.0f, 1.0f, ""),
    };

    compressor->params.map[PARAM_ID_THRESHOLD] = (Param*) &compressor->params.threshold;
    compressor->params.map[PARAM_ID_ATTACK] = (Param*) &compressor->params.attack;
    compressor->params.map[PARAM_ID_RELEASE] = (Param*) &compressor->params.release;
    compressor->params.map[PARAM_ID_RATIO] = (Param*) &compressor->params.ratio;
    compressor->params.map[PARAM_ID_OUTPUT_GAIN] = (Param*) &compressor->params.output_gain;
    compressor->params.map[PARAM_ID_MIX] = (Param*) &compressor->params.mix;
    compressor->params.map[PARAM_ID_INTERPOLATOR] = (Param*) &compressor->params.interpolator;

    return compressor;
}

void compressor_process(Compressor* compressor, Buffer* buffer) {
    const float output_gain = compressor->params.output_gain.value;
    const float mix = compressor->params.mix.value;

    for (size_t i = 0; i < buffer->frame_count; i++) {
        compressor->side_input = 0;
        for (size_t channel = 0; channel < buffer->channel_count; channel++) {
            compressor->side_input += fabs(buffer->data[channel][i]);
        }
        compressor->side_input /= buffer->channel_count;

        compressor_process_sidechain(compressor);
        compressor_process_state(compressor);
        const float gain = compressor_process_gain(compressor);

        for (size_t channel = 0; channel < buffer->channel_count; channel++) {
            const float input = buffer->data[channel][i];
            const float output = input * gain * output_gain;
            buffer->data[channel][i] = lerp(input, output, mix); 

            compressor->params.interpolator.value = compressor->interpolator;
        }
    }
}

void compressor_process_sidechain(Compressor* compressor) {
    compressor->rms_index++;
    if (compressor->rms_index >= RMS_WINDOW_SIZE) {
        compressor->rms_index = 0;
    }

    const float x = compressor->side_input;
    compressor->rms_window[compressor->rms_index] = x * x;
    float sum = 0;
    for (size_t i = 0; i < RMS_WINDOW_SIZE; i++) {
        sum += compressor->rms_window[i];
    }

    compressor->side_output = sqrt(sum / RMS_WINDOW_SIZE);
}

void compressor_start_attack(Compressor* compressor) {
    const float attack = compressor->params.attack.value;
    compressor->state = COMPRESSOR_ATTACKING;
    compressor->step = 1.0f / (attack * 0.001f * compressor->sample_rate);
}

void compressor_start_release(Compressor* compressor) {
    const float release = compressor->params.attack.value;
    compressor->state = COMPRESSOR_RELEASING;
    compressor->step = 1.0f / (release * 0.001f * compressor->sample_rate);
}

void compressor_process_state(Compressor* compressor) {
    const float threshold = compressor->params.threshold.value;
    const float level = gain_to_db(compressor->side_output);

    switch (compressor->state) {
        case COMPRESSOR_IDLE: {
            if (level >= threshold) {
                compressor_start_attack(compressor);
                break;
            }
        } break;

        case COMPRESSOR_ATTACKING: {
            compressor->interpolator += compressor->step;
            if (compressor->interpolator > 1.0f) {
                compressor->interpolator = 1.0f;
            }

            if (level < threshold) {
                compressor_start_release(compressor);
                break;
            }
        } break;

        case COMPRESSOR_RELEASING: {
            compressor->interpolator -= compressor->step;
            if (compressor->interpolator < 0.0f) {
                compressor->interpolator = 0.0f;
                compressor->state = COMPRESSOR_IDLE;
            }

            if (level >= threshold) {
                compressor_start_attack(compressor);
            }
        } break;
    }
}

// TODO: soft knee
float compressor_process_gain(Compressor* compressor) {
    const float ratio = compressor->params.ratio.value;
    const float threshold = compressor->params.threshold.value;
    const float final_ratio = lerp(1.0f, ratio, compressor->interpolator);
    const float level = gain_to_db(compressor->side_output);

    float gain = 0;
    if (level >= threshold) {
        const float sub = level - threshold ;
        gain = sub / final_ratio - sub;
    }

    return db_to_gain(gain);
}

// TODO: handle errors
void compressor_handle_clap_event(Compressor* compressor, const clap_event_header_t* event_header) {
    switch(event_header->type) {
        case CLAP_EVENT_PARAM_VALUE: {
            const clap_event_param_value_t* event = (const clap_event_param_value_t*) event_header;
            if(!params_is_valid_id(event->param_id)) {
                break;
            }

            Param* p = compressor->params.map[event->param_id];
            p->set_value(p, event->value);
        } break;
    }
}
