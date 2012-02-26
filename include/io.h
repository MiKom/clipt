#ifndef __CLIT_IO_H
#define __CLIT_IO_H

#include <system.h>

struct io_load_handler_desc_s
{
	char *desc;
	GList *filters;
};
typedef struct io_load_handler_desc_s io_load_handler_desc_t;

/**
  Returns list of load handler descriptions from all io plugins
  \returns List of load handlers description. All elements of this list must be
  freed
*/
GList *
io_get_load_handler_descriptions(void);

sys_result_t
io_load_image(const char *path);

#endif
