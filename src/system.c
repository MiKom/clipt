#include <config.h>
#include <system.h>

sys_config_t* sys_get_config(void)
{
	static sys_config_t config;
	return &config;
}
