#include <config.h>
#include <io.h>
#include <plugin.h>

GList *
io_get_load_handler_descriptions(void)
{
	int i,j;
	GList *ret = NULL;

	GList *io_plugins = plugin_get_by_type(PLUGIN_FILEIO);
	GList *iter;
	for(iter = g_list_first(io_plugins); iter; iter = g_list_next(iter)) {
		plugin_fileio_t *plugin = (plugin_fileio_t*) iter->data;
		for(i=0; i < plugin->n_load_handlers; i++) {
			plugin_load_handler_t *load_handler = plugin->load_handlers[i];
			io_load_handler_desc_t *desc =
					malloc(sizeof(io_load_handler_desc_t));
			desc->filters = NULL;
			desc->desc = load_handler->desc;

			for(j=0; j < load_handler->nfilters; j++) {
				desc->filters = g_list_append(desc->filters,
							      load_handler->filters[j]);
			}
			ret = g_list_append(ret, desc);
		}
	}
	g_list_free(io_plugins);
	return ret;
}


