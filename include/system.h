#ifndef __CLIT_SYSTEM_H
#define __CLIT_SYSTEM_H

#define CLIT_OK              0x00
#define CLIT_ENOTIMPLEMENTED 0x01
#define CLIT_EINVALID        0x02
#define CLIT_ERESOURCES      0x04
#define CLIT_ENOTFOUND       0x08
#define CLIT_ERROR           0xFF

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
