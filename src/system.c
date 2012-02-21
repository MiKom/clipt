#include <config.h>
#include <image.h>
#include <system.h>
#include <device.h>

// sys_state

static sys_state_t sys_state;

 __attribute__((constructor(101)))
 static void sys_state_ctor(void)
{
        static device_buffer_t _buffer[2];
        static device_context_t _context;
        sys_state.buffer  = _buffer;
        sys_state.context = &_context;
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
