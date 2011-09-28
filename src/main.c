#include<gtk/gtk.h>
#include<glib/gi18n.h>

int main(int argc, char **argv) {
	GtkWidget* window;

	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Obrazator");

	g_signal_connect(window, "destroy", 
		G_CALLBACK (gtk_main_quit), NULL);
	
	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
