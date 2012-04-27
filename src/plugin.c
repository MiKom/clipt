#include <config.h>
#include <image.h>
#include <system.h>
#include <plugin.h>

#include <dlfcn.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>

sys_result_t
plugin_load(const char* path, plugin_handle_t* handle)
{
	sys_result_t init_result;
	union { void* object; void (*func)(void); } symbol;

	handle->library = dlopen(path, RTLD_NOW);
	if(!handle->library)
		return CLIPT_ERROR;

	symbol.object = dlsym(handle->library, CLIPT_DEFAULT_SYMBOL);
	if(!symbol.object) {
		dlclose(handle->library);
		return CLIPT_EINVALID;
	}

	handle->plugin = ((plugin_t* (*)(void))(symbol.func))();
	init_result = handle->plugin->plugin_load();
	if(init_result != CLIPT_OK)
		dlclose(handle->library);

	return init_result;
}

sys_result_t
plugin_unload(plugin_handle_t* handle, gpointer placeholder)
{
	sys_result_t deinit_result;
	g_debug("Unloading plugin %s", handle->plugin->name);
	deinit_result = handle->plugin->plugin_unload();
	if(deinit_result != CLIPT_OK) {
		g_warning("Unloading plugin \"%s\" failed", handle->plugin->name);
	}
	dlclose(handle->library);
	return deinit_result;
}

GList*
plugin_get_by_type(plugin_type_t type)
{
	GList *io_plugins = NULL;
	GList *iter = g_list_first(sys_get_state()->plugin_handles);

	for(;iter; iter = g_list_next(iter)) {
		plugin_handle_t *handle = iter->data;
		plugin_t *plugin = (plugin_t*) (handle->plugin);

		if(plugin->type == type) {
			io_plugins = g_list_append(io_plugins, plugin);
		}

	}
	return io_plugins;
}

int filter_f(const struct dirent *entry)
{
	const char* name = entry->d_name;
	size_t len = strlen(name);
	if(len < 3) {
		return 0;
	} else if(strncmp( &(name[len - 3]), ".so", 3) == 0) {
		return 1;
	} else {
		return 0;
	}
}

sys_result_t plugin_load_all()
{
	sys_config_t* config = sys_get_config();
	sys_state_t* state = sys_get_state();
#if __unix__
	struct dirent **match_entries;
	g_debug("Current plugin path: %s", config->dir_plugins);
	int num_entries = scandir(config->dir_plugins, &match_entries,
				  filter_f, alphasort);
	if(num_entries == -1) {
		return CLIPT_ERROR;
	}

	while(num_entries--) {
		plugin_handle_t *handle = g_malloc(sizeof(plugin_handle_t));
		if(handle == NULL) {
			g_critical("Out of memory");
			return CLIPT_ERESOURCES;
		}

		char *plugin_path = calloc(PATH_MAX, sizeof(char));
		if(plugin_path == NULL) {
			g_critical("Out of memory");
			g_free(handle);
			return CLIPT_ERESOURCES;
		}

		strcat(plugin_path, config->dir_plugins);
		strcat(plugin_path, "/");
		strcat(plugin_path, match_entries[num_entries]->d_name);

		sys_result_t err = plugin_load(plugin_path, handle);
		if(err != CLIPT_OK) {
			g_warning("Cannot load plugin from: %s", plugin_path);
			g_free(handle);
		} else {
			g_debug("Plugin %s from %s loaded",
				handle->plugin->name,
				plugin_path);
			state->plugin_handles = g_list_append(state->plugin_handles, handle);
		}

		free(plugin_path);
	}
	free(match_entries);
	return CLIPT_OK;
#else
	g_warning("No plugin system available");
	return CLIPT_ENOTIMPLEMENTED;
#endif
}

sys_result_t plugin_unload_all()
{
	sys_state_t* state = sys_get_state();
	g_list_foreach(state->plugin_handles, (GFunc) plugin_unload, NULL);
	return CLIPT_OK;
}
