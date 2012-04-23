#ifndef __CLIT_IO_H
#define __CLIT_IO_H

#include <system.h>

enum io_handler_type_e
{
	IO_LOAD_HANDLER,
	IO_SAVE_HANDLER
};
typedef enum io_handler_type_e io_handler_type_t;

struct io_handler_desc_s
{
	io_handler_type_t type;
	char *desc;
	GList *filters;
};
typedef struct io_handler_desc_s io_handler_desc_t;

/**
  Returns a list of handler descriptions of given type from all io plugins
  \param type type of the handler, as in io_handler_type_t
  \returns List of handler descriptions of type specified by type. All elements of this list must be
  freed
*/
GList *
io_get_handler_descriptions(io_handler_type_t type);

sys_result_t
io_load_image(const char *path);

#endif
