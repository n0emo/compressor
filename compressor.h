// TODO: init and deinit mutexes
// TODO: free plugin
// TODO: display parameter name
// TODO: logarithmic scale for gain parameter
// TODO: unit support for parameters
// TODO: process audio
#include <clap/clap.h>

#include <threads.h>

#define array_append(array, item) do { \
    if(array.count == array.capacity) { \
        size_t new_capacity = array.capacity == 0 ? 4 : array.capacity * 2; \
        array.items = realloc(array.items, new_capacity); \
    } \
    array.items[array.count] = item; \
    array.count++; \
} while (0);


typedef enum ParamKind {
    PARAM_FLOAT,
} ParamKind;

typedef struct FloatParam {
    float value;
    float min;
    float max;
    float default_value;
} FloatParam;

typedef union ParamData {
    FloatParam as_float;
} ParamData;

typedef struct Param {
    ParamKind kind;
    ParamData data;
    const char* id;
    const char* name;
    mtx_t* mutex;
} Param;

typedef struct CompressorParams {
    Param* items;
    size_t count;
    size_t capacity;
} CompressorParams;

typedef struct Compressor {
    clap_plugin_t plugin;
    CompressorParams params;
    const clap_host_t* host;
    float sample_rate;
} Compressor;

Compressor* compressor_create();

bool param_write_clap_info(Param* param, clap_param_info_t* info, clap_id id);
bool param_get_value(Param* param, double* value);
bool param_display_value(Param* param, double value, char* display, size_t size);
bool param_read_value_from_display(Param* param, const char* display, double* value);
