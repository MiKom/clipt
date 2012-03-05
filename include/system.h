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
	//Original image
	device_buffer_t* source;
	//Previous commited image,
	device_buffer_t* previous;
	device_buffer_t* current;
	device_buffer_t* draw;
	device_context_t* context;
	GList* plugin_handles;

	int buffer_index;
};
typedef struct sys_state_s sys_state_t;

sys_state_t*
sys_get_state(void);

device_buffer_t*
sys_get_source_buffer(void);

device_buffer_t*
sys_get_current_buffer(void);

device_buffer_t*
sys_get_previous_buffer(void);

device_buffer_t*
sys_get_draw_buffer(void);

/**
  Saves buffer as commited buffer after copying current
  buffer to prev_buffer. It may be a current draw buffer.

  \param buffer a buffer to be saved as current
*/
sys_result_t
sys_commit_buffer(device_buffer_t *buffer);

/**
  Clears all system state buffers and makes them invalid:
   * source buffer
   * previous buffer
   * draw buffer
*/
sys_result_t
sys_clear_buffers();

/**
  Copies previous buffer to current buffer and draw buffer. Basically
  it's an undo action
*/
sys_result_t
sys_undo();

/**
  Copies current buffer to draw buffer. Use it when you finished working with
  e.g. previews and user canceled action. Will restore last commited state.
*/
sys_result_t
sys_draw_current_buffer();

#endif
