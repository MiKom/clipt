#include<gtk/gtk.h>
#include<glib/gi18n.h>

GtkWidget* make_window() {
	GtkWidget* window;
	GtkWidget* vbox;
	GtkWidget* menu_bar;
	GtkWidget* menu;
	GtkWidget* root_item;
	GtkWidget* menu_items;
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Obrazator");
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

	g_signal_connect(window, "destroy",
	                G_CALLBACK (gtk_main_quit), NULL);

	menu = gtk_menu_new();

	/* Open menu item */
	menu_items = gtk_menu_item_new_with_mnemonic("_Open");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_items);
	gtk_widget_show(menu_items);

	/* Quit menu item */
	menu_items = gtk_menu_item_new_with_mnemonic("_Quit");
	g_signal_connect(menu_items, "activate", 
	                G_CALLBACK(gtk_main_quit), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_items);
	gtk_widget_show(menu_items);

	root_item = gtk_menu_item_new_with_mnemonic("_File");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_item), menu);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(vbox);

	menu_bar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 2);
	gtk_widget_show(menu_bar);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), root_item);

	return window;
}

int main(int argc, char **argv) {
	GtkWidget* window;

	gtk_init(&argc, &argv);
	window = make_window();
	
	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
