#include <clap/ext/params.h>

#include <stdbool.h>
#include <threads.h>

typedef enum ParamId {
    PARAM_ID_THRESHOLD,
    PARAM_ID_ATTACK,
    PARAM_ID_RELEASE,
    PARAM_ID_RATIO,
    PARAM_ID_OUTPUT_GAIN,
    PARAM_ID_MIX,
    PARAM_ID_INTERPOLATOR, // TODO: only for testing
} ParamId;
#define PARAM_ID_COUNT 7

typedef struct Param Param;
typedef bool param_write_clap_info_t(const Param* param, clap_param_info_t* info);
typedef bool param_get_value_t(const Param* param, double* value);
typedef bool param_display_value_t(const Param* param, double value, char* display, size_t size);
typedef bool param_read_value_from_display_t(const Param* param, const char* display, double* value);
typedef void param_set_value_t(Param* param, float value);

typedef struct Param {
    clap_id id;
    const char* name;
    mtx_t mutex;
    param_write_clap_info_t* write_clap_info;
    param_get_value_t* get_value;
    param_display_value_t* display_value;
    param_read_value_from_display_t* read_value_from_display;
    param_set_value_t* set_value;
} Param;

typedef struct FloatParam {
    Param base;

    float value;
    float min;
    float max;
    float default_value;
    const char* unit;
} FloatParam;

typedef struct GainParam {
    Param base;

    float value;
    float min;
    float max;
    float default_value;
} GainParam;

typedef struct PercentParam {
    Param base;

    float value;
    float default_value;
} PercentParam;

typedef struct CompressorParams {
    FloatParam threshold;
    FloatParam attack;
    FloatParam release;
    FloatParam ratio;
    GainParam output_gain;
    PercentParam mix;
    FloatParam interpolator;

    Param* map[PARAM_ID_COUNT];
} CompressorParams;

void params_init_mutexes(CompressorParams* params);
void params_destroy_mutexes(CompressorParams* params);
bool params_is_valid_id(ParamId id);

FloatParam float_param(ParamId id, const char* name, float default_value, float min, float max, const char* unit);
GainParam gain_param(ParamId id, const char* name, float default_value, float min, float max);
PercentParam percent_param(ParamId id, const char* name, float default_value);

param_write_clap_info_t gain_write_clap_info;
param_write_clap_info_t percent_write_clap_info;
param_write_clap_info_t float_write_clap_info;

param_get_value_t gain_get_value;
param_get_value_t percent_get_value;
param_get_value_t float_get_value;

param_display_value_t gain_display_value;
param_display_value_t percent_display_value;
param_display_value_t float_display_value;

param_read_value_from_display_t gain_read_value_from_display;
param_read_value_from_display_t percent_read_value_from_display;
param_read_value_from_display_t float_read_value_from_display;

param_set_value_t gain_set_value;
param_set_value_t percent_set_value;
param_set_value_t float_set_value;

void base_write_clap_info(const Param* param, clap_param_info_t* info);
void params_log_unknown_param(ParamId id);
