#ifndef __CLIT_PLUGIN_H
#define __CLIT_PLUGIN_H


typedef sys_result_t (*plugin_load_func_t)(void);
typedef sys_result_t (*plugin_unload_func_t)(void);

enum plugin_type_e {
	PLUGIN_FILEIO,
	PLUGIN_NODE
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

typedef sys_result_t (*plugin_io_load_func_t)(char *path, image_t **image);
struct plugin_load_handler_s
{
	size_t nfilters;
	char **filters;
	char *desc;
	plugin_io_load_func_t function;
};
typedef struct plugin_load_handler_s plugin_load_handler_t;

typedef sys_result_t (*plugin_io_save_func_t)(char *path, image_t *image);
struct plugin_save_handler_s
{
	size_t nfilters;
	char **filters;
	char *desc;
	plugin_io_save_func_t function;
};
typedef struct plugin_save_handler_s plugin_save_handler_t;

struct plugin_fileio_s
{
	struct plugin_s base;
	size_t n_load_handlers;
	plugin_load_handler_t **load_handlers;

	size_t n_save_handlers;
	plugin_save_handler_t **save_handlers;
};
typedef struct plugin_fileio_s plugin_fileio_t;

struct plugin_node_s
{
	struct plugin_s base;

};
typedef struct plugin_node_s plugin_node_t;

sys_result_t
plugin_load(const char* path, plugin_handle_t* handle);

sys_result_t
plugin_unload(plugin_handle_t* handle, gpointer placeholder);

/**
 * Loads all plugins from directory specified in config file, otherwise from /usr/lib/clit
 **/
sys_result_t
plugin_load_all();

sys_result_t plugin_unload_all();

#endif
