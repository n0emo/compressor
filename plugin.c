#include <clap/all.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool entry_init(const char* path);
void entry_deinit();
const void* entry_get_factory(const char* factory_id);

const clap_plugin_entry_t clap_entry = {
    .init = entry_init,
    .deinit = entry_deinit,
    .get_factory = entry_get_factory,
    .clap_version = CLAP_VERSION_INIT,
};

uint32_t get_plugin_count(const clap_plugin_factory_t* factory);
const clap_plugin_descriptor_t* get_plugin_descriptor(const clap_plugin_factory_t* factory, uint32_t index);
const clap_plugin_t * create_plugin(const clap_plugin_factory_t* factory,
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

bool plugin_init(const clap_plugin_t* plugin);
void plugin_destroy(const clap_plugin_t* plugin);
bool plugin_activate(const clap_plugin_t* plugin, double sample_rate, uint32_t min_frames, uint32_t max_frames);
void plugin_deactivate(const clap_plugin_t* plugin);
bool plugin_start_processing(const clap_plugin_t* plugin);
void plugin_stop_processing(const clap_plugin_t* plugin);
void plugin_reset (const clap_plugin_t* plugin);
clap_process_status plugin_process(const clap_plugin_t* plugin, const clap_process_t* process);
const void* plugin_get_extension(const clap_plugin_t* plugin, const char* id);
void plugin_on_main_thread(const clap_plugin_t* plugin);

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

typedef struct Compressor {
    clap_plugin_t plugin;
    const clap_host_t* host;
    float sample_rate;
} Compressor;

uint32_t audio_ports_count(const clap_plugin_t* plugin, bool is_input);
bool audio_ports_get(const clap_plugin_t* plugin, uint32_t index, bool is_input, clap_audio_port_info_t* port_info);

static const clap_plugin_audio_ports_t plugin_audio_ports = {
    .count = audio_ports_count,
    .get = audio_ports_get,
};

bool entry_init(const char* path) {
    (void) path;
    return true;
}

void entry_deinit() {
}

const void* entry_get_factory(const char* factory_id) {
    (void) factory_id;

    return &plugin_factory;
}

uint32_t get_plugin_count(const clap_plugin_factory_t* factory) {
    (void) factory;
    return 1;
}

const clap_plugin_descriptor_t* get_plugin_descriptor(const clap_plugin_factory_t* factory, uint32_t index) {
    (void) factory;
    (void) index;

    if (index != 0) {
        return NULL;
    }

    return &plugin_descriptor;
}

const clap_plugin_t* create_plugin(const clap_plugin_factory_t* factory,
                                    const clap_host_t* host, const char* plugin_id) {
    (void) factory;

    if (!clap_version_is_compatible(host->clap_version) || strcmp(plugin_id, plugin_descriptor.id)) {
        return NULL;
    }

    Compressor* compressor = malloc(sizeof(*compressor));
    compressor-> host = host;
    compressor->plugin = clap_plugin;
    compressor->plugin.plugin_data = compressor;

    return &compressor->plugin;
}

bool plugin_init(const clap_plugin_t* plugin) {
    (void) plugin;

    return true;
}

void plugin_destroy(const clap_plugin_t* plugin) {
    (void) plugin;
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
    (void) plugin;
    (void) process;

    return CLAP_PROCESS_CONTINUE;
}

const void* plugin_get_extension(const clap_plugin_t* plugin, const char* id) {
    (void) plugin;

    if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) {
        return &plugin_audio_ports;
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

bool audio_ports_get(const clap_plugin_t* plugin, uint32_t index, bool is_input, clap_audio_port_info_t* port_info) {
    (void) plugin;
    (void) index;
    (void) is_input;

    port_info->id = 0;
    port_info->channel_count = 2;
    snprintf(port_info->name, sizeof(port_info->name), "%s", "Compressor port");

    return true;
}
