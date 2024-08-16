#include "params.h"

#include <stdio.h>
#include <string.h>

#include "util.h"

void params_init_mutexes(CompressorParams* params) {
    for(size_t i = 0; i < PARAM_ID_COUNT; i++) {
        mtx_init(&params->map[i]->mutex, mtx_plain);
    }
}

void params_destroy_mutexes(CompressorParams* params) {
    for(size_t i = 0; i < PARAM_ID_COUNT; i++) {
        mtx_destroy(&params->map[i]->mutex);
    }
}

bool params_is_valid_id(clap_id id) {
    return 0 <= id && id < PARAM_ID_COUNT;
}

FloatParam float_param(ParamId id, const char* name, float default_value, float min, float max, const char* unit) {
    return (FloatParam) {
        .base = {
            .id = id,
            .name = name,
            .write_clap_info = float_write_clap_info,
            .get_value = float_get_value,
            .set_value = float_set_value,
            .display_value = float_display_value,
            .read_value_from_display = float_read_value_from_display,
        },
        .default_value = default_value,
        .value = default_value,
        .min = min,
        .max = max,
        .unit = unit,
    };
}

GainParam gain_param(ParamId id, const char* name, float default_value, float min, float max) {
    default_value = db_to_gain(default_value);
    min = db_to_gain(min);
    max = db_to_gain(max);

    return (GainParam) {
        .base = {
            .id = id,
            .name = name,
            .write_clap_info = gain_write_clap_info,
            .get_value = gain_get_value,
            .set_value = gain_set_value,
            .display_value = gain_display_value,
            .read_value_from_display = gain_read_value_from_display,
        },
        .default_value = default_value,
        .value = default_value,
        .min = min,
        .max = max,
    };
}

PercentParam percent_param(ParamId id, const char* name, float default_value) {
    return (PercentParam) {
        .base = {
            .id = id,
            .name = name,
            .write_clap_info = percent_write_clap_info,
            .get_value = percent_get_value,
            .set_value = percent_set_value,
            .display_value = percent_display_value,
            .read_value_from_display = percent_read_value_from_display,
        },
        .default_value = default_value,
        .value = default_value,
    };
}

bool gain_write_clap_info(const Param* base, clap_param_info_t* info) {
    GainParam* param = (GainParam*) base;

    base_write_clap_info(&param->base, info);
    info->default_value = param->default_value;
    info->min_value = gain_to_db(param->min);
    info->max_value = gain_to_db(param->max);

    return true;
}

bool percent_write_clap_info(const Param* base, clap_param_info_t* info) {
    PercentParam* param = (PercentParam*) base;

    base_write_clap_info(&param->base, info);
    info->default_value = param->default_value;
    info->min_value = 0;
    info->max_value = 100;

    return true;
}

bool float_write_clap_info(const Param* base, clap_param_info_t* info) {
    FloatParam* param = (FloatParam*) base;

    base_write_clap_info(&param->base, info);
    info->default_value = param->default_value;
    info->min_value = param->min;
    info->max_value = param->max;

    return true;
}

void base_write_clap_info(const Param* param, clap_param_info_t* info) {
    memset(info, 0, sizeof(*info));

    info->id = param->id;
    snprintf(info->name, sizeof(info->name), "%s", param->name);
}

bool gain_get_value(const Param* base, double* value) {
    GainParam* param = (GainParam*) base;

    *value = gain_to_db(param->value);
    return true;
}

bool percent_get_value(const Param* base, double* value) {
    PercentParam* param = (PercentParam*) base;

    *value = param->value * 100;
    return true;
}

bool float_get_value(const Param* base, double* value) {
    FloatParam* param = (FloatParam*) base;

    *value = param->value;
    return true;
}

bool gain_display_value(const Param* base, double value, char* display, size_t size) {
    (void) base;

    snprintf(display, size, "%0.2f dB", value);
    return true;
}

bool percent_display_value(const Param* base, double value, char* display, size_t size) {
    (void) base;

    snprintf(display, size, "%0.0f%%", value);
    return true;
}

bool float_display_value(const Param* base, double value, char* display, size_t size) {
    FloatParam* param = (FloatParam*) base;

    snprintf(display, size, "%0.2f%s", value, param->unit);
    return true;
}

bool gain_read_value_from_display(const Param* base, const char* display, double* value) {
    (void) base;

    sscanf(display, "%lf dB", value);
    return true;
}

bool percent_read_value_from_display(const Param* base, const char* display, double* value) {
    (void) base;

    sscanf(display, "%lf%%", value);
    return true;
}

bool float_read_value_from_display(const Param* base, const char* display, double* value) {
    (void) base;

    sscanf(display, "%lf", value); // TODO
    return true;
}

void gain_set_value(Param* base, float value) {
    GainParam* param = (GainParam*) base;

    param->value = db_to_gain(value);
}

void percent_set_value(Param* base, float value) {
    PercentParam* param = (PercentParam*) base;

    param->value = value * 0.01f;
}

void float_set_value(Param* base, float value) {
    FloatParam* param = (FloatParam*) base;

    param->value = value;
}
