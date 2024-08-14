#include <clap/ext/params.h>

#include <stdbool.h>
#include <threads.h>

typedef enum ParamId {
    PARAM_ID_INPUT_GAIN = 0,
    PARAM_ID_OUTPUT_GAIN,
    PARAM_ID_MIX,
} ParamId;
#define PARAM_ID_COUNT 3

typedef struct ParamCommon {
    clap_id id;
    const char* name;
    mtx_t mutex;
} ParamCommon;

typedef struct FloatParam {
    ParamCommon common;
    float value;
    float min;
    float max;
    float default_value;
} FloatParam;

typedef struct GainParam {
    ParamCommon common;
    float value;
    float min;
    float max;
    float default_value;
} GainParam;

typedef struct PercentParam {
    ParamCommon common;
    float value;
    float default_value;
} PercentParam;

typedef struct CompressorParams {
    GainParam input_gain;
    GainParam output_gain;
    PercentParam mix;
} CompressorParams;

void params_init_mutexes(CompressorParams* params);
void params_destroy_mutexes(CompressorParams* params);
bool params_is_valid_id(ParamId id);
bool params_write_clap_info(CompressorParams* params, ParamId id, clap_param_info_t* info);
bool params_get_value(CompressorParams* params, ParamId id, double* value);
bool params_display_value(CompressorParams* params, ParamId id, double value, char* display, size_t size);
bool params_read_value_from_display(CompressorParams* params, ParamId id, const char* display, double* value);
void params_set_value(CompressorParams* params, ParamId id, double value);
void params_log_unknown_param(ParamId id);

bool gain_write_clap_info(GainParam param, clap_param_info_t* info);
bool percent_write_clap_info(PercentParam param, clap_param_info_t* info);
void common_write_clap_info(ParamCommon common, clap_param_info_t* info);

bool gain_get_value(GainParam param, double* value);
bool percent_get_value(PercentParam param, double* value);

bool gain_display_value(double value, char* display, size_t size);
bool percent_display_value(double value, char* display, size_t size);

bool gain_read_value_from_display(const char* display, double* value);
bool percent_read_value_from_display(const char* display, double* value);

void gain_set_value(GainParam* param, float value);
void percent_set_value(PercentParam* param, float value);
