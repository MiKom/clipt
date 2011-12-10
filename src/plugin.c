#include <config.h>
#include <system.h>
#include <image.h>
#include <plugin.h>

#include <dlfcn.h>

sys_result_t
plugin_load(const gchar* path, plugin_handle_t* handle)
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
plugin_unload(const char* path, plugin_handle_t* handle)
{
    sys_result_t deinit_result;
    deinit_result = handle->plugin->plugin_unload();
    dlclose(handle->library);
    return deinit_result;
}
