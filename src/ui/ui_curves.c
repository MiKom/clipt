#include <config.h>

#include <ui/ui_curves.h>
#include <ui/window.h>

static void
ui_curves_show_dialog(GtkWidget* widget, gpointer data);

static void
ui_curves_scale_cb(GtkWidget* widget, gpointer data);

static void
ui_curves_combobox_changed_cb(GtkWidget* widget, gpointer data);

static char* ui_def =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='ColorMenu' action='ColorMenuAction'>"
"      <menuitem action='CurvesAction' />"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry actions[] = {
        {"CurvesAction", "color-curves",
         "Curves", NULL,
         "Manipulate curves of colors",
         G_CALLBACK(ui_curves_show_dialog)}
};
static guint n_actions = G_N_ELEMENTS(actions);

sys_result_t
ui_curves_add_ui_string(GtkUIManager* ui_manager)
{
        GError* error = NULL;
        gtk_ui_manager_add_ui_from_string(ui_manager, ui_def, -1, &error);
        if(error) {
                g_error_free(error);
                return CLIT_ERROR;
        }
        return CLIT_OK;
}

void
ui_curves_add_action_entries(GtkActionGroup* action_group, GtkWindow *parent)
{
        gtk_action_group_add_actions(action_group, actions, n_actions, parent);
}

static void
ui_curves_show_dialog(GtkWidget* widget, gpointer data)
{
        ui_curves_dialog_t *obj = malloc(sizeof(ui_curves_dialog_t));

        GtkWidget *dialog;
        GtkWidget *combobox;
        GtkWidget *scale;
        GtkWidget *drawing_area;

        GtkWidget *box;
        GtkWidget *label;

        GtkWindow* parent = (GtkWindow*) data;

        dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "Curves");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
        gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
        gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
        gtk_dialog_add_button(GTK_DIALOG(dialog),
                               "Apply",
                               GTK_RESPONSE_APPLY);
        obj->dialog = dialog;
        box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        //setting up drawing area
        drawing_area = gtk_drawing_area_new();
        gtk_widget_set_size_request(GTK_WIDGET(drawing_area), 256, 256);
        gtk_box_pack_start(GTK_BOX(box), drawing_area, TRUE, FALSE, 5);
        obj->drawing_area = drawing_area;

        //setting up combobox
        label = gtk_label_new("Operation");
        gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(label), FALSE, FALSE, 0);
        combobox = gtk_combo_box_text_new();
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox),
                                  "brightness", "Adjust brightness");
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox),
                                  "gamma", "Adjust gamma");
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox),
                                  "contrast", "Adjust contrast");
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
        g_signal_connect(G_OBJECT(combobox), "changed",
                         G_CALLBACK(ui_curves_combobox_changed_cb), obj);
        gtk_box_pack_start(GTK_BOX(box), combobox, TRUE, TRUE, 0);
        obj->combobox = combobox;

        //setting up the scale
        scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                             -256.0, 256.0, 1.0);
        gtk_range_set_value(GTK_RANGE(scale), 0.0);
        g_signal_connect(G_OBJECT(scale), "value-changed",
                         G_CALLBACK(ui_curves_scale_cb), NULL);
        gtk_box_pack_start(GTK_BOX(box), scale, FALSE, FALSE, 5);
        obj->scale = scale;

        gtk_widget_show_all(GTK_WIDGET(box));
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(GTK_WIDGET(dialog));

        free(obj);
}

static void
ui_curves_scale_cb(GtkWidget* widget, gpointer data)
{
        ui_window_force_redraw();
}

static void
ui_curves_combobox_changed_cb(GtkWidget* widget, gpointer data)
{
        ui_curves_dialog_t *obj = (ui_curves_dialog_t*) data;
        gtk_range_set_value(GTK_RANGE(obj->scale), 0.0);
}
