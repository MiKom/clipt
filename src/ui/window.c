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
static ui_widget_t* ui_drawing_area;

static GLXContext* glctx;

void ui_add_item_to_menu(GtkWidget* menubar, gchar* menu_name, GtkWidget* item);
void ui_drawing_area_init(GtkWidget* widget, gpointer data);

sys_result_t
ui_window_init(ui_widget_t** widget)
{
	ui_widget_t* ui_widget = ui_widget_defaults(*widget, "Application Window", 400, 300);
	GtkWidget*   ui_window = ui_widget->widget;
	
	if(!ui_window)
		ui_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(ui_window), ui_widget->title);
	gtk_window_set_default_size(GTK_WINDOW(ui_window), ui_widget->width, ui_widget->height);

	g_signal_connect(ui_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
        //setting up app vbox
        ui_vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(ui_window), ui_vbox);
        gtk_widget_show(ui_vbox);

        //setting up menbubar
        ui_menu_bar = gtk_menu_bar_new();
        gtk_box_pack_start(GTK_BOX(ui_vbox), ui_menu_bar, FALSE, FALSE, 2);
        gtk_widget_show(ui_menu_bar);

        char* root_menus[] = {"_File", "_Edit", "_Colors", "_Effects", "_Help", NULL};
        int i;
        for(i=0; root_menus[i] != NULL; i++) {
            ui_root_item = gtk_menu_item_new_with_mnemonic(root_menus[i]);
            ui_submenu = gtk_menu_new();
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(ui_root_item), ui_submenu);
            gtk_menu_shell_append(GTK_MENU_SHELL(ui_menu_bar), ui_root_item);
            gtk_widget_show(ui_root_item);
        }

//File menu
        ui_menu_items = gtk_menu_item_new_with_mnemonic("_Open");
        ui_add_item_to_menu(ui_menu_bar, "_File", ui_menu_items);

        ui_menu_items = gtk_menu_item_new_with_mnemonic("_Save binary");
        ui_add_item_to_menu(ui_menu_bar, "_File", ui_menu_items);

        ui_menu_items = gtk_menu_item_new_with_mnemonic("Save _ASCII");
        ui_add_item_to_menu(ui_menu_bar, "_File", ui_menu_items);

        ui_menu_items = gtk_separator_menu_item_new();
        ui_add_item_to_menu(ui_menu_bar,"_File", ui_menu_items);

        ui_menu_items = gtk_menu_item_new_with_mnemonic("_Quit");
        g_signal_connect(ui_menu_items, "activate",
                         G_CALLBACK(gtk_main_quit), NULL);
        ui_add_item_to_menu(ui_menu_bar,"_File", ui_menu_items);

//Edit menu
        ui_menu_items = gtk_menu_item_new_with_mnemonic("Undo");
        ui_add_item_to_menu(ui_menu_bar, "_Edit", ui_menu_items);

//Help menu
        ui_menu_items = gtk_menu_item_new_with_mnemonic("About");
        ui_add_item_to_menu(ui_menu_bar, "_Help", ui_menu_items);

//Drawing area
        ui_drawing_area = ui_widget_init(NULL, "Drawing area", 100, 100);
        ui_drawing_area->widget = gtk_drawing_area_new();

        gtk_box_pack_end(GTK_BOX(ui_vbox), ui_drawing_area->widget, TRUE, TRUE, 0);
        g_signal_connect(ui_drawing_area->widget, "realize",
                         G_CALLBACK(ui_drawing_area_init), ui_drawing_area);
        gtk_widget_show(ui_drawing_area->widget);

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

void ui_add_item_to_menu(GtkWidget* menubar, gchar* menu_name, GtkWidget* menu_item) {
    GList* menus = gtk_container_get_children(GTK_CONTAINER(menubar));
    GList* list_item = g_list_first(menus);

    for(;list_item; list_item = g_list_next(list_item)){
        GtkWidget* root_menu_widget = list_item->data;
        const gchar* label = gtk_menu_item_get_label(GTK_MENU_ITEM(root_menu_widget));
        if(strcmp(label, menu_name) == 0) {
            gtk_menu_shell_append(GTK_MENU_SHELL(gtk_menu_item_get_submenu(GTK_MENU_ITEM(root_menu_widget))),
                                  menu_item);
            break;
        }
    }
    g_list_free(menus);
}


void ui_drawing_area_init(GtkWidget* widget, gpointer data){
        Window xwin;
        ui_widget_t* drawing_area = (ui_widget_t*)data;
        ui_widget_getnative(drawing_area, &xwin);
        render_context_init(xwin, glctx);
}
