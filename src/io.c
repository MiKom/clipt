#include <config.h>
#include <io.h>
#include <plugin.h>
#include <device.h>
#include <image.h>

GList *
io_get_load_handler_descriptions(void)
{
	int i,j;
	GList *ret = NULL;

	GList *io_plugins = plugin_get_by_type(PLUGIN_FILEIO);
	GList *iter;
	for(iter = g_list_first(io_plugins); iter; iter = g_list_next(iter)) {
		plugin_fileio_t *plugin = (plugin_fileio_t*) iter->data;
		for(i=0; i < (int) plugin->n_load_handlers; i++) {
			plugin_load_handler_t *load_handler = plugin->load_handlers[i];
			io_load_handler_desc_t *desc =
					malloc(sizeof(io_load_handler_desc_t));
			desc->filters = NULL;
			desc->desc = load_handler->desc;

			for(j=0; j < (int) load_handler->nfilters; j++) {
				desc->filters = g_list_append(desc->filters,
							      load_handler->filters[j]);
			}
			ret = g_list_append(ret, desc);
		}
	}
	g_list_free(io_plugins);
	return ret;
}

sys_result_t
io_load_image(const char *path)
{
//TODO: Add proper buffer cleaning
//	device_buffer_destroy(sys_get_state()->context, &sys_get_state()->buffer[0]);
//	device_buffer_destroy(sys_get_state()->context, &sys_get_state()->buffer[1]);
//	device_buffer_destroy(sys_get_state()->context, &sys_get_state()->source);
	GList *io_plugins = plugin_get_by_type(PLUGIN_FILEIO);
	GList *iter;

	for(iter = g_list_first(io_plugins); iter; iter = g_list_next(iter)) {
		plugin_fileio_t* plugin = (plugin_fileio_t*) iter->data;
		size_t i;
		for(i=0; i<plugin->n_load_handlers; i++) {
			if(plugin->load_handlers[i]->can_open(path)) {
				image_data_t *data;
				plugin_io_load_func_t load_fun =
						plugin->load_handlers[i]->function;
				if( load_fun(path, &data) != CLIT_OK ) {
					continue;
				}
				device_buffer_create_from_data(sys_get_state()->context,
							       DEVICE_BUFFER_SOFTWARE, data,
							       sys_get_state()->source);
				free(data->data);
				int j;
				for(j=0; j<2; j++) {
					device_buffer_create(sys_get_state()->context,
							     DEVICE_BUFFER_HARDWARE,
							     data->width,
							     data->height,
							     data->channels,
							     &sys_get_state()->buffer[j]);
				}
				device_buffer_copy(sys_get_state()->source,
						   sys_get_active_buffer());
				//TODO: Find out why this causes SIGSEGV
				//g_list_free(io_plugins);
				//g_list_free(iter);
				return CLIT_OK;
			}
		}
	}
	g_list_free(io_plugins);
	g_list_free(iter);
	return CLIT_EINVALID;
}
