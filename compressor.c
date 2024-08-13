#include "compressor.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <threads.h>

#define DB_MINUS_INFINITY -100.0f

float db_to_gain(float dbs) {
    if(dbs >= DB_MINUS_INFINITY) {
        return powf(10.0f, dbs * 0.05f);
    } else {
        return 0.0f;
    }
}

Param float_param(const char* name, FloatParam data) {
    Param param = { 0 };

    param.name = name;
    param.kind = PARAM_FLOAT;
    param.data = (ParamData) { .as_float = data };
    param.mutex = malloc(sizeof(*param.mutex));
    mtx_init(param.mutex, PTHREAD_MUTEX_DEFAULT);

    return param;
}

bool param_write_clap_info(Param* param, clap_param_info_t* info, clap_id id) {
    mtx_lock(param->mutex);

    info->id = id;
    strncpy(info->name, param->name, sizeof(info->name));
    // TODO: flags

    switch(param->kind) {
        case PARAM_FLOAT: {
            FloatParam p = param->data.as_float;

            memset(info, 0, sizeof(*info));
            info->default_value = p.default_value;
            info->min_value = p.min;
            info->max_value = p.max;
       } break;
    }

    mtx_unlock(param->mutex);
    return true;
}

bool param_get_value(Param *param, double *value) {
    mtx_lock(param->mutex);

    bool result = true;
    switch(param->kind) {
        case PARAM_FLOAT: {
            *value = param->data.as_float.value;
        } break;
        default:
            result = false;
    }

    mtx_unlock(param->mutex);
    return result;
}

bool param_display_value(Param* param, double value, char* display, size_t size) {
    bool result = true;

    switch(param->kind) {
        case PARAM_FLOAT: {
            snprintf(display, size, "%0.2f", value);
        } break;
        default:
            result = false;
    }

    return result;
}

bool param_read_value_from_display(Param* param, const char* display, double* value) {
    bool result = true;

    switch(param->kind) {
        case PARAM_FLOAT: {
            sscanf(display, "%lf", value);
        } break;
        default:
            result = false;
    }

    return result;
}

Compressor* compressor_create() {
    Compressor* compressor = calloc(1, sizeof(*compressor));

    Param input_gain = float_param("Input gain", (FloatParam) {
        .min = db_to_gain(-30.0f),
        .max = db_to_gain(30.0f),
        .default_value = db_to_gain(0.0f),
    });

    array_append(compressor->params, input_gain);

    return compressor;
}

