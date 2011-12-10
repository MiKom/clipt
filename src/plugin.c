#include <config.h>
#include <system.h>
#include <image.h>
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
        return CLIT_ERROR;

    symbol.object = dlsym(handle->library, CLIT_DEFAULT_SYMBOL);
    if(!symbol.object) {
        dlclose(handle->library);
        return CLIT_EINVALID;
    }

    handle->plugin = ((plugin_t* (*)(void))(symbol.func))();
    init_result = handle->plugin->plugin_load();
    if(init_result != CLIT_OK)
        dlclose(handle->library);
    
    return init_result;
}

sys_result_t
plugin_unload(plugin_handle_t* handle)
{
    sys_result_t deinit_result;
    deinit_result = handle->plugin->plugin_unload();
    dlclose(handle->library);
    return deinit_result;
}

int filter_f(const struct dirent *entry) {
        char* name = entry->d_name;
        size_t len = strlen(name);
        if(len < 3) {
                return 0;
        } else if(strncmp( &(name[len - 3]), ".so", 3) == 0) {
                return 1;
        } else {
                return 0;
        }
}

sys_result_t load_plugins()
{
        sys_config_t* config = sys_get_config();
        sys_state_t* state = sys_get_state();
#if __unix__
        struct dirent **match_entries;
        int num_entries = scandir(config->dir_plugins, &match_entries,
                                  filter_f, alphasort);
        if(num_entries == -1) {
                return CLIT_ERROR;
        }

        while(num_entries--) {
                plugin_handle_t *handle = g_malloc(sizeof(plugin_handle_t));
                if(handle == NULL) {
                        g_critical("Out of memory");
                        return CLIT_ERESOURCES;
                }

                char *plugin_path = calloc(PATH_MAX, sizeof(char));
                if(plugin_path == NULL) {
                        g_critical("Out of memory");
                        g_free(handle);
                        return CLIT_ERESOURCES;
                }

                strcat(plugin_path, config->dir_plugins);
                strcat(plugin_path, "/");
                strcat(plugin_path, match_entries[num_entries]->d_name);

                sys_result_t err = plugin_load(plugin_path, handle);
                if(err != CLIT_OK) {
                        g_warning("Cannot load plugin from: %s", plugin_path);
                        g_free(handle);
                } else {
                        state->plugin_handles = g_list_append(state->plugin_handles, handle);
                }

                free(plugin_path);
        }
        free(match_entries);

#else
        g_warning("No plugin system available");
#endif
}

sys_result_t unload_plugins()
{
        sys_state_t* state;
        GList *iter = g_list_first(state->plugin_handles);
        while(iter) {
                plugin_unload(iter->data);
        }
        g_list_free_full(state->plugin_handles, g_free);
}
