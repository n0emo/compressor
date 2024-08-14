#define CLAP_HAS_THREAD
#include <clap/all.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compressor.h"

static bool entry_init(const char* path);
static void entry_deinit();
static const void* entry_get_factory(const char* factory_id);

CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    .init = entry_init,
    .deinit = entry_deinit,
    .get_factory = entry_get_factory,
    .clap_version = CLAP_VERSION_INIT,
};

static uint32_t get_plugin_count(const clap_plugin_factory_t* factory);
static const clap_plugin_descriptor_t* get_plugin_descriptor(const clap_plugin_factory_t* factory, uint32_t index);
static const clap_plugin_t * create_plugin(const clap_plugin_factory_t* factory,
                                    const clap_host_t* host, const char* plugin_id);

static const clap_plugin_factory_t plugin_factory = {
    .get_plugin_count = get_plugin_count,
    .get_plugin_descriptor = get_plugin_descriptor,
    .create_plugin = create_plugin,
};

static const clap_plugin_descriptor_t plugin_descriptor = {
    .clap_version = CLAP_VERSION_INIT,
    .id = "org.n0emo.compressor",
    .name = "_n0emo's Compressor",
    .description = "Simple compressor written in C",
    .vendor = "_n0emo",
    .version = "0.1.0",
    .features = (const char*[]) {
        CLAP_PLUGIN_FEATURE_AUDIO_EFFECT,
        NULL,
    },
    .url = "https://github.com/n0emo/compressor",
};

static bool plugin_init(const clap_plugin_t* plugin);
static void plugin_destroy(const clap_plugin_t* plugin);
static bool plugin_activate(const clap_plugin_t* plugin, double sample_rate, uint32_t min_frames, uint32_t max_frames);
static void plugin_deactivate(const clap_plugin_t* plugin);
static bool plugin_start_processing(const clap_plugin_t* plugin);
static void plugin_stop_processing(const clap_plugin_t* plugin);
static void plugin_reset (const clap_plugin_t* plugin);
static clap_process_status plugin_process(const clap_plugin_t* plugin, const clap_process_t* process);
static const void* plugin_get_extension(const clap_plugin_t* plugin, const char* id);
static void plugin_on_main_thread(const clap_plugin_t* plugin);

static const clap_plugin_t clap_plugin = {
    .desc = &plugin_descriptor,
    .init = plugin_init,
    .destroy = plugin_destroy,
    .activate = plugin_activate,
    .deactivate = plugin_deactivate,
    .start_processing = plugin_start_processing,
    .stop_processing = plugin_stop_processing,
    .reset = plugin_reset,
    .process = plugin_process,
    .get_extension = plugin_get_extension,
    .on_main_thread = plugin_on_main_thread,
};

static uint32_t audio_ports_count(const clap_plugin_t* plugin, bool is_input);
static bool audio_ports_get(const clap_plugin_t* plugin, uint32_t index, bool is_input, clap_audio_port_info_t* port_info);

static const clap_plugin_audio_ports_t plugin_audio_ports = {
    .count = audio_ports_count,
    .get = audio_ports_get,
};

static uint32_t params_count(const clap_plugin_t* plugin);
static void params_flush(const clap_plugin_t* plugin, const clap_input_events_t* input_events,
                         const clap_output_events_t* output_events);
static bool params_get_info(const clap_plugin_t* plugin, uint32_t index, clap_param_info_t* info);
static bool plugin_params_get_value(const clap_plugin_t* plugin, clap_id id, double* value);
static bool params_value_to_text(const clap_plugin_t *, clap_id, double, char *, uint32_t);
static bool params_text_to_value(const clap_plugin_t *, clap_id, const char *, double *);

static const clap_plugin_params_t plugin_params = {
    .count = params_count,
    .flush = params_flush,
    .get_info = params_get_info,
    .get_value = plugin_params_get_value,
    .value_to_text = params_value_to_text,
    .text_to_value = params_text_to_value,
};

bool entry_init(const char* path) {
    (void) path;
    return true;
}

void entry_deinit() {
}

const void* entry_get_factory(const char* factory_id) {
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0) {
        return &plugin_factory;
    }

    return NULL;
}

uint32_t get_plugin_count(const clap_plugin_factory_t* factory) {
    (void) factory;
    return 1;
}

const clap_plugin_descriptor_t* get_plugin_descriptor(const clap_plugin_factory_t* factory, uint32_t index) {
    (void) factory;
    (void) index;

    return &plugin_descriptor;
}

const clap_plugin_t* create_plugin(const clap_plugin_factory_t* factory,
                                    const clap_host_t* host, const char* plugin_id) {
    (void) factory;

    if (!clap_version_is_compatible(host->clap_version) || strcmp(plugin_id, plugin_descriptor.id)) {
        return NULL;
    }

    Compressor* compressor = compressor_create();
    compressor->host = host;
    compressor->plugin = clap_plugin;
    compressor->plugin.plugin_data = compressor;

    return &compressor->plugin;
}

bool plugin_init(const clap_plugin_t* plugin) {
    Compressor* compressor = plugin->plugin_data;
    params_init_mutexes(&compressor->params);

    return true;
}

void plugin_destroy(const clap_plugin_t* plugin) {
    Compressor* compressor = plugin->plugin_data;
    params_destroy_mutexes(&compressor->params);
    free(compressor);
}

bool plugin_activate(const clap_plugin_t* plugin, double sample_rate, uint32_t min_frames, uint32_t max_frames) {
    (void) min_frames;
    (void) max_frames;

    Compressor* compressor = plugin->plugin_data;
    compressor->sample_rate = sample_rate;

    return true;
}

void plugin_deactivate(const clap_plugin_t* plugin) {
    (void) plugin;
}

bool plugin_start_processing(const clap_plugin_t* plugin) {
    (void) plugin;
    return true;
}

void plugin_stop_processing(const clap_plugin_t* plugin) {
    (void) plugin;
}

void plugin_reset (const clap_plugin_t* plugin) {
    (void) plugin;
}

clap_process_status plugin_process(const clap_plugin_t* plugin, const clap_process_t* process) {
    Compressor* compressor = plugin->plugin_data;

    float* data[process->audio_inputs->channel_count];
    for (size_t channel = 0; channel < 2; channel++) {
        memcpy(
            process->audio_outputs->data32[channel],
            process->audio_inputs->data32[channel],
            process->frames_count * sizeof(float)
        );
    }

    Buffer buffer = {
        .channel_count = process->audio_inputs->channel_count,
        .data = data,
    };

    size_t event_count = process->in_events->size(process->in_events);
    size_t event_index = 0;

    for (size_t frame = 0; frame < process->frames_count; ) {
        size_t next_event_frame = process->frames_count;
        if(event_index < event_count) {
            const clap_event_header_t* event_header = process->in_events->get(process->in_events, event_index);
            compressor_handle_clap_event(compressor, event_header);
            next_event_frame = event_header->time;
            event_index++;
        }

        for (size_t channel = 0; channel < 2; channel++) {
            data[channel] = process->audio_outputs->data32[channel] + frame;
        }
        buffer.frame_count = next_event_frame - frame;

        compressor_process(compressor, &buffer);

        frame += next_event_frame;
    }

    return CLAP_PROCESS_CONTINUE;
}

const void* plugin_get_extension(const clap_plugin_t* plugin, const char* id) {
    (void) plugin;

    if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) {
        return &plugin_audio_ports;
    }
    if (strcmp(id, CLAP_EXT_PARAMS) == 0) {
        return &plugin_params;
    }

    return NULL;
}

void plugin_on_main_thread(const clap_plugin_t* plugin) {
    (void) plugin;
}

uint32_t audio_ports_count(const clap_plugin_t* plugin, bool is_input) {
    (void) plugin;
    (void) is_input;

    return 1;
}

bool audio_ports_get(const clap_plugin_t* plugin, uint32_t index, bool is_input, clap_audio_port_info_t* info) {
    (void) plugin;
    (void) is_input;

    if (index > 0) {
        return false;
    }

    info->id = 0;
    snprintf(info->name, sizeof(info->name), "%s", "Compressor Port");
    info->channel_count = 2;
    info->flags = CLAP_AUDIO_PORT_IS_MAIN;
    info->port_type = CLAP_PORT_STEREO;
    info->in_place_pair = CLAP_INVALID_ID;

    return true;
}

uint32_t params_count(const clap_plugin_t* plugin) {
    (void) plugin;
    return PARAM_ID_COUNT;
}

void params_flush(const clap_plugin_t* plugin, const clap_input_events_t* input_events,
           const clap_output_events_t* output_events) {
    (void) plugin;
    (void) input_events;
    (void) output_events;
    // TODO
}

bool params_get_info(const clap_plugin_t* plugin, uint32_t index, clap_param_info_t* info) {
    Compressor* compressor = plugin->plugin_data;
    ParamId id = index;

    if(!params_is_valid_id(id)) {
        return false;
    }

    return params_write_clap_info(&compressor->params, id, info);
}

bool plugin_params_get_value(const clap_plugin_t* plugin, clap_id id, double* value) {
    Compressor* compressor = plugin->plugin_data;
    ParamId param_id = id;

    if(!params_is_valid_id(param_id)) {
        return false;
    }

    return params_get_value(&compressor->params, param_id, value);
}

bool params_value_to_text(const clap_plugin_t* plugin, clap_id id, double value, char * display, uint32_t size) {
    Compressor* compressor = plugin->plugin_data;
    ParamId param_id = id;

    if(!params_is_valid_id(param_id)) {
        return false;
    }

    return params_display_value(&compressor->params, param_id, value, display, size);
}

bool params_text_to_value(const clap_plugin_t* plugin, clap_id id, const char* display, double* value) {
    Compressor* compressor = plugin->plugin_data;
    ParamId param_id = id;

    if(!params_is_valid_id(param_id)) {
        return false;
    }

    return params_read_value_from_display(&compressor->params, param_id, display, value);
}
