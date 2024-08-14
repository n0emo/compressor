// TODO: init and deinit mutexes
// TODO: free plugin
// TODO: display parameter name
// TODO: logarithmic scale for gain parameter
// TODO: unit support for parameters
// TODO: process audio
#include <clap/clap.h>

#include <threads.h>

#include "params.h"

typedef struct Compressor {
    clap_plugin_t plugin;
    const clap_host_t* host;
    float sample_rate;
    CompressorParams params;
} Compressor;

Compressor* compressor_create();

typedef struct Buffer {
    uint32_t channel_count;
    uint32_t frame_count;
    float** data;
} Buffer;

void compressor_process(Compressor* compressor, Buffer* buffer);
