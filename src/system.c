#include <config.h>
#include <image.h>
#include <system.h>
#include <device.h>

// sys_state

static sys_state_t sys_state;

__attribute__((constructor(101)))
static void sys_state_ctor(void)
{
	static device_buffer_t _current;
	_current.storage = DEVICE_BUFFER_INVALID;
	static device_buffer_t _previous;
	_previous.storage = DEVICE_BUFFER_INVALID;
	static device_buffer_t _draw;
	_draw.storage = DEVICE_BUFFER_INVALID;
	static device_buffer_t _source;
	_source.storage = DEVICE_BUFFER_INVALID;
	static device_context_t _context;

	sys_state.current  = &_current;
	sys_state.previous  = &_previous;
	sys_state.draw = &_draw;
	sys_state.source = &_source;
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

device_buffer_t*
sys_get_source_buffer(void)
{
	sys_state_t *state = sys_get_state();
	return state->source;
}

device_buffer_t*
sys_get_current_buffer(void)
{
	sys_state_t *state = sys_get_state();
	return state->current;
}

device_buffer_t*
sys_get_previous_buffer(void)
{
	sys_state_t *state = sys_get_state();
	return state->previous;
}

device_buffer_t*
sys_get_draw_buffer(void)
{
	sys_state_t *state = sys_get_state();
	return state->draw;
}

sys_result_t*
sys_commit_buffer(device_buffer_t *buffer)
{
	device_buffer_copy(sys_get_current_buffer(), sys_get_current_buffer());
	device_buffer_copy(buffer, sys_get_current_buffer());
}
