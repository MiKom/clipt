#include <config.h>
#include <image.h>
#include <system.h>
#include <device.h>

// sys_state

static sys_state_t sys_state;

 __attribute__((constructor(101)))
 static void sys_state_ctor(void)
{
        sys_state.buffer  = malloc(2 * sizeof(device_buffer_t));
        sys_state.context = malloc(sizeof(device_context_t));
        
        sys_state.buffer_index = 0;
}

 __attribute__((destructor(101)))
 static void sys_state_dtor(void)
{
        free(sys_state.buffer);
        free(sys_state.context);
}

sys_state_t* sys_get_state(void)
{
	return &sys_state;
}

// sys_config

sys_config_t* sys_get_config(void)
{
	static sys_config_t config;
	return &config;
}


device_buffer_t* sys_get_active_buffer(void)
{
        sys_state_t* state = sys_get_state();
        return &state->buffer[state->buffer_index];
}

device_buffer_t* sys_swap_buffers(void)
{
        sys_state_t* state = sys_get_state();
        state->buffer_index = (state->buffer_index + 1) % 2;
        return &state->buffer[state->buffer_index];
}
