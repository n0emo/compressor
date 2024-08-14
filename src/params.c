#include "params.h"

#include <stdio.h>
#include <string.h>

#include "util.h"

void params_init_mutexes(CompressorParams* params) {
    mtx_init(&params->input_gain.common.mutex, mtx_plain);
    mtx_init(&params->output_gain.common.mutex, mtx_plain);
    mtx_init(&params->mix.common.mutex, mtx_plain);
}

void params_destroy_mutexes(CompressorParams* params) {
    mtx_destroy(&params->input_gain.common.mutex);
    mtx_destroy(&params->output_gain.common.mutex);
    mtx_destroy(&params->mix.common.mutex);
}

bool params_is_valid_id(clap_id id) {
    return 0 <= id && id < PARAM_ID_COUNT;
}

bool params_write_clap_info(CompressorParams* params, ParamId id, clap_param_info_t* info) {
    switch(id) {
        case PARAM_ID_INPUT_GAIN: 
            return gain_write_clap_info(params->input_gain, info);
        case PARAM_ID_OUTPUT_GAIN:
            return gain_write_clap_info(params->output_gain, info);
        case PARAM_ID_MIX:
            return percent_write_clap_info(params->mix, info);
    }
    params_log_unknown_param(id);
    return false;
}

bool params_get_value(CompressorParams* params, ParamId id, double* value) {
    switch(id) {
        case PARAM_ID_INPUT_GAIN:
            return gain_get_value(params->input_gain, value);
        case PARAM_ID_OUTPUT_GAIN:
            return gain_get_value(params->output_gain, value);
        case PARAM_ID_MIX:
            return percent_get_value(params->mix, value);
    }
    params_log_unknown_param(id);
    return false;
}

bool params_display_value(CompressorParams* params, ParamId id, double value, char* display, size_t size) {
    (void) params;

    switch(id) {
        case PARAM_ID_INPUT_GAIN:
        case PARAM_ID_OUTPUT_GAIN:
            return gain_display_value(value, display, size);
        case PARAM_ID_MIX:
            return percent_display_value(value, display, size);
    }
    params_log_unknown_param(id);
    return false;
}

bool params_read_value_from_display(CompressorParams* params, clap_id id, const char* display, double* value) {
    (void) params;

    switch(id) {
        case PARAM_ID_INPUT_GAIN:
        case PARAM_ID_OUTPUT_GAIN:
            return gain_read_value_from_display(display, value);
        case PARAM_ID_MIX:
            return percent_read_value_from_display(display, value);
    }
    params_log_unknown_param(id);
    return false;
}

void params_log_unknown_param(ParamId id) {
    if(0 <= id && id < PARAM_ID_COUNT) {
        eprintf("Param ID %d is not handled\n", id);
    } else {
        eprintf("Unknown Param ID\n");
    }
}

bool gain_write_clap_info(GainParam param, clap_param_info_t* info) {
    memset(info, 0, sizeof(*info));

    common_write_clap_info(param.common, info);
    info->default_value = param.default_value;
    info->min_value = gain_to_db(param.min);
    info->max_value = gain_to_db(param.max);

    return true;
}

bool percent_write_clap_info(PercentParam param, clap_param_info_t* info) {
    memset(info, 0, sizeof(*info));

    common_write_clap_info(param.common, info);
    info->default_value = param.default_value;
    info->min_value = 0;
    info->max_value = 100;

    return true;
}

void common_write_clap_info(ParamCommon common, clap_param_info_t* info) {
    info->id = common.id;
    snprintf(info->name, sizeof(info->name), "%s", common.name);
}

bool gain_get_value(GainParam param, double* value) {
    *value = gain_to_db(param.value);
    return true;
}

bool percent_get_value(PercentParam param, double* value) {
    *value = param.value * 100;
    return true;
}

bool gain_display_value(double value, char* display, size_t size) {
    snprintf(display, size, "%0.2f dB", value);
    return true;
}

bool percent_display_value(double value, char* display, size_t size) {
    snprintf(display, size, "%0.0f%%", value);
    return true;
}

bool gain_read_value_from_display(const char* display, double* value) {
    sscanf(display, "%lf dB", value);
    return true;
}

bool percent_read_value_from_display(const char* display, double* value) {
    sscanf(display, "%lf%%", value);
    return true;
}

