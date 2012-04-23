#include <config.h>
#include <io.h>
#include <plugin.h>
#include <device.h>
#include <image.h>
#include <system.h>

static io_handler_desc_t*
io_get_load_handler_description(plugin_load_handler_t *handler)
{
	io_handler_desc_t *desc = malloc(sizeof(io_handler_desc_t));
	desc->type = IO_LOAD_HANDLER;
	desc->desc = handler->desc;
	desc->filters = NULL;

	int i;
	for(i=0; i < (int) handler->nfilters; i++) {
		desc->filters = g_list_append(desc->filters,
					      handler->filters[i]);
	}
	return desc;
}

static io_handler_desc_t*
io_get_save_handler_description(plugin_save_handler_t *handler)
{
	io_handler_desc_t *desc = malloc(sizeof(io_handler_desc_t));
	desc->type = IO_SAVE_HANDLER;
	desc->desc = handler->desc;
	desc->filters = NULL;

	int i;
	for(i=0; i < (int) handler->nfilters; i++) {
		desc->filters = g_list_append(desc->filters,
					      handler->filters[i]);
	}
	return desc;
}

GList *
io_get_handler_descriptions(io_handler_type_t type)
{
	GList *ret = NULL;

	GList *io_plugins = plugin_get_by_type(PLUGIN_FILEIO);
	GList *iter;
	for(iter = g_list_first(io_plugins); iter; iter = g_list_next(iter)) {
		plugin_fileio_t *plugin = (plugin_fileio_t*) iter->data;
		int i;
		switch(type) {
		case IO_LOAD_HANDLER:
			for(i=0; i < (int) plugin->n_load_handlers; i++) {
				io_handler_desc_t *desc = io_get_load_handler_description(plugin->load_handlers[i]);
				ret = g_list_append(ret, desc);
			}
			break;
		case IO_SAVE_HANDLER:
			for(i=0; i < (int) plugin->n_save_handlers; i++) {
				io_handler_desc_t *desc = io_get_save_handler_description(plugin->save_handlers[i]);
				ret = g_list_append(ret, desc);
			}
			break;
		}
	}
	g_list_free(io_plugins);
	return ret;
}

void
io_fill_buffers(image_data_t *data)
{
	device_buffer_create_from_data(sys_get_state()->context,
				       DEVICE_BUFFER_SOFTWARE, data,
				       sys_get_state()->source);
	free(data->data);
	//copying data to draw buffer
	device_buffer_create(sys_get_state()->context,
			     DEVICE_BUFFER_HARDWARE,
			     data->width,
			     data->height,
			     data->channels,
			     sys_get_draw_buffer());
	device_buffer_copy(sys_get_state()->source,
			   sys_get_draw_buffer());
	//Copying data to current working buffer
	device_buffer_create(sys_get_state()->context,
			     DEVICE_BUFFER_HARDWARE,
			     data->width,
			     data->height,
			     data->channels,
			     sys_get_current_buffer());
	device_buffer_copy(sys_get_state()->source,
			   sys_get_current_buffer());

	//creating new previous buffer
	device_buffer_create(sys_get_state()->context,
			     DEVICE_BUFFER_HARDWARE,
			     data->width,
			     data->height,
			     data->channels,
			     sys_get_previous_buffer());
}

sys_result_t
io_load_image(const char *path)
{
	sys_clear_buffers();
	GList *io_plugins = plugin_get_by_type(PLUGIN_FILEIO);
	GList *iter;
	image_data_t *data;
	gboolean loaded = FALSE;
	for(iter = g_list_first(io_plugins);
	    iter && !loaded;
	    iter = g_list_next(iter)) {
		plugin_fileio_t* plugin = (plugin_fileio_t*) iter->data;
		size_t i;
		for(i=0; i<plugin->n_load_handlers; i++) {
			if(plugin->load_handlers[i]->can_open(path)) {
				plugin_io_load_func_t load_fun =
						plugin->load_handlers[i]->function;
				if( load_fun(path, &data) == CLIT_OK ) {
					loaded = TRUE;
					break;
				}
			}
		}
	}
	g_list_free(io_plugins);
	g_list_free(iter);

	if( loaded ) {
		io_fill_buffers(data);
		return CLIT_OK;
	} else {
		free(data);
		return CLIT_EINVALID;
	}
}
