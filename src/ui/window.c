#include <stdlib.h>

#include "config.h"
#include "image.h"
#include "system.h"
#include "core.h"
#include "render.h"
#include "plugin.h"
#include "device.h"
#include "io.h"
#include "ui/ui.h"
#include "ui/window.h"
#include "ui/ui_histogram.h"
#include "ui/ui_curves.h"
#include "ui/ui_convolutions.h"
#include "ui/ui_binarization.h"
#include "ui/ui_colorspaces.h"

static GtkWidget* ui_window;
static GtkWidget* ui_vbox;
static GtkWidget* ui_menu_bar;
static GtkActionGroup* ui_action_group;
static GtkUIManager* ui_manager;
static GtkWidget* ui_toolbar;

static ui_widget_t* ui_drawing_area;

static guint ui_new_image_signal;

static gboolean gl_initialized = FALSE;
static GLXContext glctx;

static void ui_quit(void);

static void ui_open_file_cb(GtkWidget* widget, gpointer data);
static void ui_save_file_cb(GtkWidget* widget, gpointer data);
static void ui_menu_quit_cb(GtkWidget* widget, gpointer data);

static void ui_undo_cb(GtkWidget* widget, gpointer data);
static void ui_reset_cb(GtkWidget* widget, gpointer data);

static void ui_about_cb(GtkWidget* widget, gpointer data);

static gboolean ui_window_delete_event_cb(GtkWidget *widget, gpointer data);
static void ui_window_image_changed_cb(GtkWidget *widget, gpointer data);

static void ui_drawing_area_init(GtkWidget* widget, gpointer data);
static void ui_drawing_area_after_realize_cb(GtkWidget* widget, gpointer data);
static void ui_drawing_area_draw_cb(GtkWidget* widget, cairo_t*, gpointer data);

char *ui_definition =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='FileMenu' action='FileMenuAction'>"
"      <menuitem action='FileOpenAction'/>"
"      <menuitem action='FileSaveAction'/>"
"      <separator/>"
"      <menuitem action='FileQuitAction' />"
"    </menu>"
"    <menu name='EditMenu' action='EditMenuAction'>"
"      <menuitem action='UndoAction'/>"
"      <menuitem action='ResetAction' />"
"    </menu>"
"    <menu name='ColourMenu' action='ColourMenuAction'>"
"      <placeholder />"
"    </menu>"
"    <menu name='ToolsMenu' action='ToolsMenuAction'>"
"      <placeholder/>"
"    </menu>"
"    <menu name='FiltersMenu' action='FiltersMenuAction'>"
"    "
"    </menu>"
"    <menu name='HelpMenu' action='HelpMenuAction'>"
"      <menuitem action='AboutAction' />"
"    </menu>"
"  </menubar>"
"  <toolbar name='MainToolbar'>"
"    <toolitem action='FileOpenAction'/>"
"    <toolitem action='FileSaveAction'/>"
"  </toolbar>"
"</ui>";

static GtkActionEntry entries[] = {
	//File Menu
	{ "FileMenuAction", NULL, "_File",
	  NULL, NULL, NULL},

	{ "FileOpenAction", GTK_STOCK_OPEN,
	  "_Open", "<control>O",
	  "Open image",
	  G_CALLBACK(ui_open_file_cb)},

	{ "FileSaveAction", GTK_STOCK_SAVE,
	  "_Save", "<control>S",
	  "Save image",
	  G_CALLBACK(ui_save_file_cb)},

	{ "FileQuitAction", GTK_STOCK_QUIT,
	  "_Quit", "<control>Q",
	  "Quit program",
	  G_CALLBACK(ui_menu_quit_cb)},

	//Edit Menu
	{ "EditMenuAction", NULL, "_Edit",
	  NULL, NULL, NULL},
	{ "UndoAction", GTK_STOCK_UNDO,
	  "Undo", "<control>Z",
	  "Undo last action",
	  G_CALLBACK(ui_undo_cb)},
	{ "ResetAction", GTK_STOCK_REVERT_TO_SAVED,
	  "Reset image", NULL,
	  "Reset all changes made to image",
	  G_CALLBACK(ui_reset_cb)},

	{ "ColourMenuAction", NULL, "_Colour",
	  NULL, NULL, NULL},

	{ "FiltersMenuAction", NULL, "_Filters",
	  NULL, NULL, NULL},
	//Tools Menu
	{ "ToolsMenuAction", NULL, "_Tools",
	  NULL, NULL, NULL},

	//Help Menu
	{ "HelpMenuAction", NULL, "_Help",
	  NULL, NULL, NULL},
	{ "AboutAction", GTK_STOCK_ABOUT,
	  "About", NULL,
	  "Information about program",
	  G_CALLBACK(ui_about_cb)}
};
static guint n_entries = G_N_ELEMENTS(entries);

sys_result_t
ui_window_init(ui_widget_t** widget)
{
	ui_widget_t *ui_widget = ui_widget_defaults(*widget,
						    "Application Window",
						    400, 300);
	ui_window = ui_widget->widget;
	GError *error = NULL;
	
	if(!ui_window)
		ui_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(ui_window), ui_widget->title);
	gtk_window_set_default_size(GTK_WINDOW(ui_window),
				    ui_widget->width,
				    ui_widget->height);

	g_signal_connect(ui_window, "delete-event", G_CALLBACK(ui_window_delete_event_cb), NULL);
	
	//setting up app vbox
	ui_vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(ui_window), ui_vbox);
	//gtk_widget_show(ui_vbox);
	ui_manager = gtk_ui_manager_new();
	ui_action_group = gtk_action_group_new("Actions");
	gtk_action_group_add_actions(ui_action_group, entries, n_entries, NULL);

	gtk_ui_manager_add_ui_from_string(ui_manager, ui_definition,-1, &error);

        //Populating menus with operations
        ui_histogram_add_action_entries(ui_action_group, ui_window);
	if( ui_histogram_add_ui_string(ui_manager) != CLIT_OK ) {
		g_warning("Adding histogram menus failed");
	}

	ui_curves_add_action_entries(ui_action_group, ui_window);
	if( ui_curves_add_ui_string(ui_manager) != CLIT_OK ) {
		g_warning("Adding curves menus failed");
	}

	ui_convolutions_add_action_entries(ui_action_group, ui_window);
	if( ui_convolutions_add_ui_string(ui_manager) != CLIT_OK ) {
		g_warning("Adding convolutions menu failed");
	}

	ui_binarization_add_action_entries(ui_action_group, ui_window);
	if( ui_binarization_add_ui_string(ui_manager) != CLIT_OK ) {
		g_warning("Adding binarization menu failed");
	}

	ui_colorspaces_add_action_entries(ui_action_group, ui_window);
	if( ui_colorspaces_add_ui_string(ui_manager) != CLIT_OK ) {
		g_warning("Adding colorspaces menu entries failed");
	}

	gtk_ui_manager_insert_action_group(ui_manager, ui_action_group, 0);
	if(error) {
		g_error("Building menus failed: %s", error->message);
		g_error_free(error);
		return CLIT_ERROR;
	}
	ui_menu_bar = gtk_ui_manager_get_widget(ui_manager,"/MainMenu");
	gtk_box_pack_start(GTK_BOX(ui_vbox), ui_menu_bar, FALSE, FALSE, 2);

	ui_toolbar = gtk_ui_manager_get_widget(ui_manager, "/MainToolbar");
	GtkStyleContext* style_ctx = gtk_widget_get_style_context(ui_toolbar);
	gtk_style_context_add_class(style_ctx, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
	gtk_box_pack_start(GTK_BOX(ui_vbox), ui_toolbar, FALSE, FALSE, 2);

	gtk_window_add_accel_group(GTK_WINDOW(ui_window),
				   gtk_ui_manager_get_accel_group(ui_manager));
	//Drawing area
	ui_drawing_area = ui_widget_init(NULL, "Drawing area", 100, 100);
	ui_drawing_area->widget = gtk_drawing_area_new();
	g_signal_connect_after(ui_drawing_area->widget, "realize",
			       G_CALLBACK(ui_drawing_area_after_realize_cb), ui_drawing_area);
	g_signal_connect_after(ui_drawing_area->widget, "draw",
			       G_CALLBACK(ui_drawing_area_draw_cb), ui_drawing_area);
	gtk_box_pack_end(GTK_BOX(ui_vbox), ui_drawing_area->widget, TRUE, TRUE, 0);
	gtk_widget_set_double_buffered(ui_drawing_area->widget, FALSE);
	//gtk_widget_show(ui_drawing_area->widget);

	//signal emitted when image gets changed somehow
	ui_new_image_signal = g_signal_new("image-changed",
					   G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST,
					   0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
					   G_TYPE_NONE, 0);

	g_signal_connect(G_OBJECT(ui_window), "image-changed",
			 G_CALLBACK(ui_window_image_changed_cb), NULL);

	gtk_widget_show_all(ui_window);
	ui_widget->widget = ui_window;
	return CLIT_OK;
}

int ui_window_destroy(ui_widget_t* widget)
{
	if(!widget->widget)
		return CLIT_EINVALID;
	ui_widget_free(widget);
	return CLIT_OK;
}

static gboolean ui_window_delete_event_cb(GtkWidget *widget, gpointer data)
{
	ui_quit();
	//Do not destroy window, will die on application quit
	return TRUE;
}

static void ui_menu_quit_cb(GtkWidget *widget, gpointer data)
{
	ui_quit();
}

static void ui_quit()
{
	//TODO: put save before quitting dialog here
	core_subsystem_stop();
	gtk_main_quit();
}

int ui_window_event(ui_widget_t* widget, unsigned long event)
{
	switch(event) {
	case UI_EVENT_SHOW:
		gtk_widget_show_all(widget->widget);
		break;
	case UI_EVENT_HIDE:
		gtk_widget_hide(widget->widget);
		break;
	}
	return CLIT_OK;
}

void ui_drawing_area_after_realize_cb(GtkWidget* widget, gpointer data)
{
	ui_widget_t* drawing_area = (ui_widget_t*) data;
	Window xwin;
	if(ui_widget_getnative(drawing_area, &xwin) != CLIT_OK) {
		fprintf(stderr, "Cannot get native window of drawing area, aborting\n");
		exit(1);
	}
}

void ui_drawing_area_init(GtkWidget* widget, gpointer data)
{
	// This is here because gtk cannot return XWindow that is proper
	// GLXRenderable until it is shown on screen so we have to postpone all
	// OpenGL and OpenCL stuff unitl window is drawn for the first time.
	core_subsystem_start();

	gl_initialized = TRUE;
}

void
ui_window_force_redraw(void)
{
	g_signal_emit(ui_window, ui_new_image_signal, 0);
}

void ui_drawing_area_draw_cb(GtkWidget* widget, cairo_t* cr, gpointer data)
{

	Window xwin;
	ui_widget_getnative((ui_widget_t*) data, &xwin);

	if(!gl_initialized) {
		ui_drawing_area_init(widget, data);
	}
	render_context_draw(xwin, &glctx);
}


void ui_window_image_changed_cb(GtkWidget *widget, gpointer data)
{
	gtk_widget_queue_draw(ui_drawing_area->widget);
}

gulong
ui_window_add_image_changed_cb(GCallback cb, gpointer data)
{
	return g_signal_connect(G_OBJECT(ui_window), "image-changed", cb, data);
}

void
ui_window_remove_image_changed_handler(gulong handler_id)
{
	g_signal_handler_disconnect(G_OBJECT(ui_window), handler_id);
}

void ui_open_file_cb(GtkWidget* widget, gpointer data)
{
	GtkWidget* ui_filedialog;
	GtkFileFilter* ui_filefilter;

	ui_filedialog = gtk_file_chooser_dialog_new("Open...",
						    GTK_WINDOW(ui_window),
						    GTK_FILE_CHOOSER_ACTION_OPEN,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_CANCEL,
						    GTK_STOCK_OPEN,
						    GTK_RESPONSE_ACCEPT,
						    NULL);


	GList *formats = io_get_handler_descriptions(IO_LOAD_HANDLER);
	GList *iter;
	for(iter = g_list_first(formats); iter; iter = g_list_next(iter)) {
		io_handler_desc_t *format =
				(io_handler_desc_t *) iter->data;

		ui_filefilter = gtk_file_filter_new();
		gtk_file_filter_set_name(GTK_FILE_FILTER(ui_filefilter),
							 format->desc);
		GList *f_it;
		for(f_it = g_list_first(format->filters); f_it; f_it = g_list_next(f_it)) {
			gtk_file_filter_add_pattern(
						GTK_FILE_FILTER(ui_filefilter),
						(char *) f_it->data);
		}
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(ui_filedialog),
					    ui_filefilter);
	}
	g_list_free_full(formats, free);

	if( gtk_dialog_run(GTK_DIALOG(ui_filedialog)) == GTK_RESPONSE_ACCEPT) {
		gchar* filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui_filedialog));

		if( io_load_image(filename) != CLIT_OK) {
			GtkWidget *err_dialog;
			err_dialog = gtk_message_dialog_new(ui_filedialog,
							    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_OK,
							    "Cannot open file specified");
			gtk_dialog_run(err_dialog);
			gtk_widget_destroy(err_dialog);
		}
		g_free(filename);
	}
	g_signal_emit(ui_window, ui_new_image_signal, 0);
	gtk_widget_destroy(ui_filedialog);
}

void ui_save_file_cb(GtkWidget* widget, gpointer data)
{
	GtkWidget *filedialog;
	GtkFileFilter *filefilter;

	filedialog = gtk_file_chooser_dialog_new("Save...",
						 GTK_WINDOW(ui_window),
						 GTK_FILE_CHOOSER_ACTION_SAVE,
						 GTK_STOCK_CANCEL,
						 GTK_RESPONSE_CANCEL,
						 GTK_STOCK_OPEN,
						 GTK_RESPONSE_ACCEPT,
						 NULL);

	GList *formats = io_get_handler_descriptions(IO_SAVE_HANDLER);
	GList *iter;

	for(iter = g_list_first(formats); iter; iter = g_list_next(iter)) {
		io_handler_desc_t *format = (io_handler_desc_t *) iter->data;

		filefilter = gtk_file_filter_new();
		gtk_file_filter_set_name(GTK_FILE_FILTER(filefilter),
					  format->desc);
		GList *f_it;
		for(f_it = g_list_first(format->filters); f_it; f_it = g_list_next(f_it)){
			gtk_file_filter_add_pattern(GTK_FILE_FILTER(filefilter),
						    (char*) f_it->data);
		}
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filedialog),
					    filefilter);
	}
	g_list_free_full(formats, free);

	if(gtk_dialog_run(GTK_DIALOG(filedialog)) == GTK_RESPONSE_ACCEPT) {

	}
	gtk_widget_destroy(GTK_WIDGET(filedialog));
}

void ui_undo_cb(GtkWidget* widget, gpointer data)
{
	sys_undo();
	g_signal_emit(ui_window, ui_new_image_signal, 0);
}
void ui_reset_cb(GtkWidget* widget, gpointer data)
{
	printf("Reset action\n");

	GtkWidget* dialog;
	dialog = gtk_message_dialog_new(GTK_WINDOW(ui_window),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_NONE,
					"Reset all changes?");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
						 "This operation is irreversible, "
						 "all changes made to current "
						 "file will be lost.");

	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			       "Don't reset",
			       GTK_RESPONSE_NO,
			       "Reset",
			       GTK_RESPONSE_YES,
			       NULL);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	if(response == GTK_RESPONSE_YES) {
		printf("User chose to reset\n");
	} else {
		printf("User chose not to reset\n");
	}

	gtk_widget_destroy(dialog);
	fflush(stdout);
}

ui_widget_t* ui_window_getdrawable(void)
{
	return ui_drawing_area;
}

GLXContext ui_window_getglcontext(void)
{
	return glctx;
}

void ui_window_setglcontext(GLXContext ctx)
{
	glctx = ctx;
}

static void ui_about_cb(GtkWidget* widget, gpointer data)
{

	static const gchar *authors[] = {"Michal Siejak <masterm@wmi.amu.edu.pl>",
					 "Milosz Kosobucki <mikom3@gmail.com>",
					 NULL};
	static const gchar copyrights[] = "Copyright © 2011 Michał Siejak\n"
			"Copyright © 2011 Miłosz Kosobucki\n";

	gtk_show_about_dialog(GTK_WINDOW(ui_window),
			      "program-name", CLIT_NAME_STRING,
			      "authors", authors,
			      "copyright", copyrights,
			      "version", CLIT_VERSION_STRING,
			      "logo-icon-name", CLIT_PROGRAM_NAME,
			      "comments", "Hardware accelerated Image Processing toolkit",
			      "license-type", GTK_LICENSE_GPL_3_0,
			      NULL
			      );
}
