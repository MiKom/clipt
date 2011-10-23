#ifndef __CLIT_SYSTEM_H
#define __CLIT_SYSTEM_H

enum sys_result_e
{
    CLIT_OK = 0x00,
    CLIT_ENOTIMPLEMENTED,
    CLIT_EINVALID,
    CLIT_ERESOURCES,
    CLIT_ENOTFOUND,
    CLIT_ERROR,
};
typedef enum sys_result_e sys_result_t;

struct sys_config_s
{
	gchar project[PATH_MAX];
	gchar dir_plugins[PATH_MAX];
	gchar dir_clprogs[PATH_MAX];
	unsigned long flags;
};
typedef struct sys_config_s sys_config_t;

sys_config_t* sys_get_config(void);

#endif
