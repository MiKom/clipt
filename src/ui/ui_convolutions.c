#include <config.h>

#include <ui/ui_convolutions.h>
#include <ui/window.h>

static void
ui_convolutions_show_dialog(GtkWidget *widget, gpointer data);

static char *ui_def =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='FiltersMenu' action='FiltersMenuAction'>"
"      <menuitem action='ConvolutionAction' />"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry actions[] = {
        {"ConvolutionAction", "draw-convolve",
	"Convolution", NULL,
	"Apply predefined or custom convolution",
         G_CALLBACK(ui_convolutions_show_dialog)}
};
static guint n_actions = G_N_ELEMENTS(actions);

static void
ui_convolutions_preview_clicked_cb(GtkButton *button, gpointer data)
{
        ui_convolutions_t* conv = (ui_convolutions_t*)data;
        
}

static void
ui_convolutions_convocombo_changed_cb(GtkComboBox *widget, gpointer data)
{
        ui_convolutions_t* conv = (ui_convolutions_t*)data;
        
}

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

static ui_convolutions_t*
ui_convolutions_dialog_new(GtkWidget *parent)
{
        ui_convolutions_t *ret = malloc(sizeof(ui_convolutions_t));
        
        GtkWindow* dialog = gtk_dialog_new();
        ret->dialog = dialog;

        gtk_window_set_title(dialog, "Convolutions");
        gtk_window_set_title(dialog, "Mathematical Morphology");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			       "Cancel",
			       GTK_RESPONSE_CANCEL,
			       "Apply",
			       GTK_RESPONSE_APPLY,
			       NULL);

        GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_box_set_spacing(GTK_BOX(box), 10);
        
        GtkWidget *grid = gtk_grid_new();

        GtkWidget* label_combo = gtk_label_new("Type: ");
        GtkWidget* convocombo = gtk_combo_box_text_new();
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(convocombo), "custom", "Custom");
        gtk_combo_box_set_active(GTK_COMBO_BOX(convocombo), 0);

        GtkWidget* matrix_view = gtk_text_view_new();
        gtk_widget_set_size_request(matrix_view, 200, 150);
        gtk_widget_set_margin_left(matrix_view, 5);
        gtk_widget_set_margin_right(matrix_view, 5);

        gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
        gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

        GtkWidget* label_bias = gtk_label_new("Bias: ");
        GtkWidget* label_divisor = gtk_label_new("Divisor: ");

        GtkSpinButton* bias = gtk_spin_button_new_with_range(0, 255, 1);
        gtk_spin_button_set_value(bias, 0);
        GtkSpinButton* divisor = gtk_spin_button_new_with_range(1, 255, 1);
        gtk_spin_button_set_value(divisor, 1);

        GtkWidget* preview = gtk_button_new_with_label("Preview");

        gtk_grid_attach(GTK_GRID(grid), label_combo, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), convocombo, 1, 0, 3, 1);
        gtk_grid_attach(GTK_GRID(grid), matrix_view, 0, 1, 4, 3);
        gtk_grid_attach(GTK_GRID(grid), label_bias, 0, 5, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), label_divisor, 2, 5, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(bias), 1, 5, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(divisor), 3, 5, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), preview, 0, 6, 4, 1);
        
        gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 0);

        ret->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(matrix_view));
        ret->bias = bias;
        ret->divisor = divisor;

        g_signal_connect(G_OBJECT(preview), "clicked",
                         G_CALLBACK(ui_convolutions_preview_clicked_cb), ret);
        g_signal_connect(G_OBJECT(convocombo), "changed",
                         G_CALLBACK(ui_convolutions_convocombo_changed_cb), ret);
        return ret;
}

static void
ui_convolutions_show_dialog(GtkWidget *widget, gpointer data)
{
        ui_convolutions_t* obj = ui_convolutions_dialog_new((GtkWidget*)data);
        
	gtk_widget_show_all(obj->dialog);
	gint response = gtk_dialog_run(GTK_DIALOG(obj->dialog));
	if(response == GTK_RESPONSE_ACCEPT) {

	} else {

	}
	gtk_widget_destroy(obj->dialog);
	free(obj);
}
