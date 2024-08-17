// TODO: smooth parameter values
// TODO: state
// TODO: GUI
#include <clap/clap.h>

#include <threads.h>

#include "params.h"

#define RMS_WINDOW_SIZE 2048

typedef enum CompressorState {
    COMPRESSOR_IDLE,
    COMPRESSOR_ATTACKING,
    COMPRESSOR_RELEASING,
} CompressorState;

typedef struct Compressor {
    clap_plugin_t plugin;
    const clap_host_t* host;
    float sample_rate;
    CompressorParams params;

    float rms_window[RMS_WINDOW_SIZE];
    uint32_t rms_index;

    float side_input;
    float side_output;
    CompressorState state;
    float interpolator;
    float step;
} Compressor;

Compressor* compressor_create();

typedef struct Buffer {
    uint32_t channel_count;
    uint32_t frame_count;
    float** data;
} Buffer;

void compressor_process(Compressor* compressor, Buffer* buffer);
void compressor_process_sidechain(Compressor* compressor);
void compressor_process_state(Compressor* compressor);
float compressor_process_gain(Compressor* compressor);
void compressor_handle_clap_event(Compressor* compressor, const clap_event_header_t* event_header);
