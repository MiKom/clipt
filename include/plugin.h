#ifndef __CLIT_PLUGIN_H
#define __CLIT_PLUGIN_H

typedef sys_result_t (*plugin_load_func_t)(void);
typedef sys_result_t (*plugin_unload_func_t)(void);

enum plugin_type_e {
    PLUGIN_FILEIO,
    PLUGIN_NODE,
};
typedef enum plugin_type_e plugin_type_t;

struct plugin_s
{
    const char* name;
    const char* author;
    const char* version;

    plugin_type_t        type;
    plugin_load_func_t   plugin_load;
    plugin_unload_func_t plugin_unload;
};
typedef struct plugin_s plugin_t;

struct plugin_handle_s
{
    struct plugin_s* plugin;
    void* library;
};
typedef struct plugin_handle_s plugin_handle_t;

struct plugin_fileio_s
{
    struct plugin_s base;
    
};
typedef struct plugin_fileio_s plugin_fileio_t;

struct plugin_node_s
{
    struct plugin_s base;
    
};
typedef struct plugin_node_s plugin_node_t;

#endif
