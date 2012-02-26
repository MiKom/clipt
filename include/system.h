#ifndef __CLIT_SYSTEM_H
#define __CLIT_SYSTEM_H

typedef struct device_context_s device_context_t;
typedef struct device_buffer_s device_buffer_t;
typedef struct image_data_s image_data_t;
typedef struct image_s image_t;

enum sys_result_e
{
	CLIT_OK = 0x00,
	CLIT_ENOTIMPLEMENTED,
	CLIT_EINVALID,
	CLIT_ERESOURCES,
	CLIT_ENOTFOUND,
	CLIT_EACCESS,
	CLIT_ERROR
};
typedef enum sys_result_e sys_result_t;

enum sys_access_e
{
	CLIT_READ_ONLY = 0,
	CLIT_WRITE_ONLY,
	CLIT_READ_WRITE
};
typedef enum sys_access_e sys_access_t;

struct sys_config_s
{
	gchar project[PATH_MAX];
	gchar dir_plugins[PATH_MAX];
	gchar dir_clprogs[PATH_MAX];
	unsigned long flags;
};
typedef struct sys_config_s sys_config_t;

sys_config_t* sys_get_config(void);

struct sys_state_s
{
	device_buffer_t* source;
	device_buffer_t* buffer;
	device_context_t* context;
	GList* plugin_handles;

	int buffer_index;
};
typedef struct sys_state_s sys_state_t;

sys_state_t* sys_get_state(void);

device_buffer_t* sys_get_active_buffer(void);
device_buffer_t* sys_swap_buffers(void);

#endif
