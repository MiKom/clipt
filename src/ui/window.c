#include <stdlib.h>

#include <config.h>
#include <system.h>
#include <render.h>
#include <ui/ui.h>
#include <ui/window.h>

static GtkWidget* ui_vbox;
static GtkWidget* ui_menu_bar;
static GtkWidget* ui_submenu;
static GtkWidget* ui_root_item;
static GtkWidget* ui_menu_items;
static GtkActionGroup* ui_action_group;
static GtkUIManager* ui_manager;
static GtkWidget* ui_toolbar;

static ui_widget_t* ui_drawing_area;

static gboolean gl_initialized = FALSE;
static GLXContext glctx;

void ui_open_file_cb(GtkWidget* widget, gpointer data);
void ui_save_file_cb(GtkWidget* widget, gpointer data);

void ui_undo_cb(GtkWidget* widget, gpointer data);

void ui_drawing_area_init(GtkWidget* widget, gpointer data);
void ui_drawing_area_after_realize_cb(GtkWidget* widget, gpointer data);
void ui_drawing_area_draw_cb(GtkWidget* widget, cairo_t*, gpointer data);

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
"    </menu>"
"    <menu name='ToolsMenu' action='ToolsMenuAction'>"
"      <placeholder/>"
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
        { "FileMenuAction", NULL, "_File"},

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
          G_CALLBACK(gtk_main_quit)},

//Edit Menu
        { "EditMenuAction", NULL, "_Edit"},
        { "UndoAction", GTK_STOCK_UNDO,
          "Undo", "<control>Z",
          "Undo last action",
          G_CALLBACK(ui_undo_cb)},

//Tools Menu
        { "ToolsMenuAction", NULL, "_Tools"},

//Help Menu
        { "HelpMenuAction", NULL, "_Help"},
        { "AboutAction", GTK_STOCK_ABOUT,
          "About", NULL,
          "Information about program",
          NULL}
};
static guint n_entries = G_N_ELEMENTS(entries);

sys_result_t
ui_window_init(ui_widget_t** widget)
{
        ui_widget_t	*ui_widget = ui_widget_defaults(*widget, "Application Window", 400, 300);
        GtkWidget	*ui_window = ui_widget->widget;
        GtkActionGroup 	*actions;
        GError 		*error = NULL;
	
	if(!ui_window)
		ui_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(ui_window), ui_widget->title);
	gtk_window_set_default_size(GTK_WINDOW(ui_window), ui_widget->width, ui_widget->height);

	g_signal_connect(ui_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
        //setting up app vbox
        ui_vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(ui_window), ui_vbox);
        //gtk_widget_show(ui_vbox);
        ui_manager = gtk_ui_manager_new();
        actions = gtk_action_group_new("Actions");
        gtk_action_group_add_actions(actions, entries, n_entries, NULL);
        gtk_ui_manager_insert_action_group(ui_manager, actions, 0);

        gtk_ui_manager_add_ui_from_string(ui_manager, ui_definition,-1, &error);
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

//Tools menu
//        ui_menu_items = gtk_menu_item_new_with_mnemonic("_Initialize GL");
//        ui_add_item_to_menu(ui_menu_bar, "_Tools", ui_menu_items);
//        g_signal_connect(ui_menu_items, "activate",
//                         G_CALLBACK(ui_drawing_area_init), ui_drawing_area);
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

void ui_drawing_area_after_realize_cb(GtkWidget* widget, gpointer data) {
    ui_widget_t* drawing_area = (ui_widget_t*) data;
    Window xwin;
    if(ui_widget_getnative(drawing_area, &xwin) != CLIT_OK) {
            fprintf(stderr, "Cannot get native window of drawing area, aborting\n");
            exit(1);
    }
}

void ui_drawing_area_init(GtkWidget* widget, gpointer data){
        Window xwin;
        ui_widget_t* drawing_area = (ui_widget_t*)data;
        ui_widget_getnative(drawing_area, &xwin);
        render_context_init(xwin, &glctx);
        gl_initialized = TRUE;
}

void ui_drawing_area_draw_cb(GtkWidget* widget, cairo_t* cr, gpointer data) {

        Window xwin;
        ui_widget_getnative((ui_widget_t*) data, &xwin);

        if(!gl_initialized) {
                ui_drawing_area_init(widget, data);
        }
        else
        {
                render_context_draw(xwin, glctx);
        }
}

void ui_open_file_cb(GtkWidget* widget, gpointer data)
{
        printf("Open action\n");
        fflush(stdout);
}

void ui_save_file_cb(GtkWidget* widget, gpointer data)
{
        printf("Save action\n");
        fflush(stdout);
}

void ui_undo_cb(GtkWidget* widget, gpointer data)
{
        printf("Undo action\n");
        fflush(stdout);
}
