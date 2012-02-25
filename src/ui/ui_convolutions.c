#include <config.h>

#include <ui/ui_convolutions.h>
#include <ui/window.h>

static char *ui_def =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='FiltersMenu' action='FiltersMenuAction'>"
"      <menu name='BlurMenu' action='BlurMenuAction'>"
"        <menuitem action='GaussianBlurAction' />"
"        <menuitem action='UniformBlurAction' />"
"      </menu>"
"      <menuitem action='ConvolutionAction'/>"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry actions[] = {
	{"BlurMenuAction", NULL, "Blur"},

	{"GaussianBlurAction", "draw-convolve",
	"Gaussian", NULL,
	"Apply gaussian blur",
	NULL },

	{"UniformBlurAction", NULL,
	"Uniform", NULL,
	"Apply uniform blur kernel",
	NULL },

	{"ConvolutionAction", "draw-convolve",
	"Custom convolution", NULL,
	"Apply custom convolution kernel",
	NULL }
};
static guint n_actions = G_N_ELEMENTS(actions);

sys_result_t
ui_convolutions_add_ui_string(GtkUIManager *ui_manager)
{
	GError *error = NULL;
	gtk_ui_manager_add_ui_from_string(GTK_UI_MANAGER(ui_manager), ui_def,
					  -1, &error);
	if(error) {
		g_error_free(error);
		return CLIT_ERROR;
	}
	return CLIT_OK;
}

void
ui_convolutions_add_action_entries(GtkActionGroup *action_group, GtkWindow *parent)
{
	gtk_action_group_add_actions(GTK_ACTION_GROUP(action_group), actions,
				     n_actions, parent);
}

