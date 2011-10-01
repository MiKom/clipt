#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <system.h>
#include <core.h>
#include <ui/ui.h>

static void show_version(void)
{
	fprintf(stderr, "%s, version %s\n", CLIT_NAME_STRING, CLIT_VERSION_STRING);
	fprintf(stderr, "(c) 2011 Michał Siejak, Miłosz Kosobucki\n");
}

static void parse_arguments(int argc, char** argv, sys_config_t* config)
{
	gboolean flag_version = FALSE;
	
	GOptionEntry entries[] = {
		{ "version", 'v', 0, G_OPTION_ARG_NONE,     &flag_version,   "Print version information and exit", NULL },
		{ "input",   'i', 0, G_OPTION_ARG_FILENAME, config->project, "Input project filename", NULL },
	};

	GError* error = NULL;
	GOptionContext* context = g_option_context_new(" - "CLIT_NAME_STRING);
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));

	if(!g_option_context_parse(context, &argc, &argv, &error)) {
		fprintf(stderr, "Option parsing failed: %s\n", error->message);
		exit(1);
	}
	g_option_context_free(context);

	if(flag_version) {
		show_version();
		exit(0);
	}
}

static int parse_config(const gchar* filename, const gchar* group,
												sys_config_t* config)
{
	gchar* rcfile_keys[] = {
		"plugin-dir", "/usr/lib/clit", config->dir_plugins,
		"clprog-dir", "/usr/lib/clit/cl", config->dir_clprogs,
		NULL,
	};
	
	gchar config_file[PATH_MAX];
	GKeyFile* keyfile;
	GError* error = NULL;
	
	gchar* value;
	int i;
	
	if(filename)
		strncpy(config_file, filename, PATH_MAX);
	else {
		gchar* confdir = getenv("XDG_CONFIG_HOME");
		if(confdir)
			sprintf(config_file, "%s/%s", confdir, CLIT_DEFAULT_CONFIG);
		else {
			g_warning("No XDG_CONFIG_HOME environment variable set");
			sprintf(config_file, "./.%s", CLIT_DEFAULT_CONFIG);
		}
	}

	i=0;
	while(rcfile_keys[i]) {
		strcpy(rcfile_keys[i+2], rcfile_keys[i+1]);
		i += 3;
	}

	keyfile = g_key_file_new();
	if(!g_key_file_load_from_file(keyfile, config_file, G_KEY_FILE_NONE, &error)) {
		g_warning("Configuration parse error: %s", error->message);
		g_error_free(error);
		g_key_file_free(keyfile);
		return CLIT_ERROR;
	}
	if(!g_key_file_has_group(keyfile, group)) {
		g_warning("Configuration file skipped. No [%s] group found.", group);
		g_key_file_free(keyfile);
		return CLIT_EINVALID;
	}
	g_message("Configuration file: %s", config_file);

	i=0;
	while(rcfile_keys[i]) {
		value = g_key_file_get_string(keyfile, group, rcfile_keys[i], NULL);
		if(value)
			strncpy(rcfile_keys[i+2], value, PATH_MAX);
		i += 3;
	}
	
	g_key_file_free(keyfile);
	return CLIT_OK;
}

int main(int argc, char** argv)
{
	int retvalue = CLIT_OK;
	sys_config_t* sys_config = sys_get_config();
	
	parse_arguments(argc, argv, sys_config);
	g_message("%s is starting ...", CLIT_NAME_STRING);
	parse_config(NULL, "clit", sys_config);
	
	retvalue = core_main();
	
	g_message("Shutdown completed");
	exit(retvalue);
}
