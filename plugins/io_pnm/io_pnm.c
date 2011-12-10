#include <config.h>
#include <system.h>
#include <image.h>
#include <plugin.h>

sys_result_t pnm_plugin_load();
sys_result_t pnm_plugin_unload();

plugin_t* clit_plugin_info()
{
       plugin_fileio_t *ret = malloc(sizeof(plugin_fileio_t));


       ret->base.name = "PNM extension plugin";
       ret->base.author = "Milosz Kosobucki";
       ret->base.version = "0.1";

       ret->base.type = PLUGIN_FILEIO;
       ret->base.plugin_load = pnm_plugin_load;
       ret->base.plugin_unload = pnm_plugin_unload;

       return (plugin_t*) ret;
}

sys_result_t pnm_plugin_load()
{
        return CLIT_OK;
}

sys_result_t pnm_plugin_unload()
{
        return CLIT_OK;
}
