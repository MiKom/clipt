#include <stdlib.h>

#include <config.h>
#include <system.h>
#include <ui/ui.h>
#include <ui/window.h>

static GtkWidget* ui_vbox;
static GtkWidget* ui_menu_bar;
static GtkWidget* ui_menu;
static GtkWidget* ui_root_item;
static GtkWidget* ui_menu_items;

int ui_window_init(ui_widget_t** widget)
{
	ui_widget_t* ui_widget = ui_widget_defaults(*widget, "Application Window", 400, 300);
	GtkWidget*   ui_window = ui_widget->widget;
	
	if(!ui_window)
		ui_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(ui_window), ui_widget->title);
	gtk_window_set_default_size(GTK_WINDOW(ui_window), ui_widget->width, ui_widget->height);

	g_signal_connect(ui_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	ui_menu = gtk_menu_new();
	
		/* Open menu item */
	ui_menu_items = gtk_menu_item_new_with_mnemonic("_Open");
	gtk_menu_shell_append(GTK_MENU_SHELL(ui_menu), ui_menu_items);
	gtk_widget_show(ui_menu_items);

	/* Quit menu item */
	ui_menu_items = gtk_menu_item_new_with_mnemonic("_Quit");
	g_signal_connect(ui_menu_items, "activate", 
	                G_CALLBACK(gtk_main_quit), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(ui_menu), ui_menu_items);
	gtk_widget_show(ui_menu_items);

	ui_root_item = gtk_menu_item_new_with_mnemonic("_File");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(ui_root_item), ui_menu);

	ui_vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(ui_window), ui_vbox);
	gtk_widget_show(ui_vbox);

	ui_menu_bar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(ui_vbox), ui_menu_bar, FALSE, FALSE, 2);
	gtk_widget_show(ui_menu_bar);

	gtk_menu_shell_append(GTK_MENU_SHELL(ui_menu_bar), ui_root_item);

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
