#include "config.h"
#include "system.h"
#include "ui/ui.h"
#include "ui/ui_morphology.h"

//size of the pixel used by drawing areas that draw structural elements,
//effectively it's a magnification factor
#define PIXEL_SIZE 6

//Defining common structural elements
static unsigned int
horizontal_line[] = {
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
};

static unsigned int
vertical_line[] = {
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
};

static unsigned int
cross[] = {
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
};

static void
ui_morphology_show_dialog(GtkWidget *widget, gpointer data);

static ui_morphology_t*
ui_morphology_dialog_new(GtkWidget *parent);

/**
  This function takes pointer to array of STRUCTURAL_ELEMENT_SIZE*STRUCTURAL_ELEMENT_SIZE
  unsigned int elements in data, that contain zeroes or ones, and draws grid
  of black and white pixels, according to this array. This grid is magniefied,
  so single pixel is PIXEL_SIZE x PIXEL_SIZE in size.
  */
static void
ui_morphology_draw_structural_element(
		GtkWidget *widget,
		cairo_t *cr,
		gpointer data);

static gboolean
ui_morphology_button_press(
		GtkWidget *widget,
		GdkEventButton *event,
		gpointer data);

static char* ui_def =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='FiltersMenu' action='FiltersMenuAction'>"
"      <menuitem action='MorphologyAction' />"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry actions[] = {
	{"MorphologyAction", "color-curves",
	 "Morphology", NULL,
	 "Appply mathematical morphology operations",
	 G_CALLBACK(ui_morphology_show_dialog)}
};
static guint n_actions = G_N_ELEMENTS(actions);

sys_result_t
ui_morphology_add_ui_string(GtkUIManager* ui_manager)
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
ui_morphology_add_action_entries(GtkActionGroup* action_group, GtkWindow *parent)
{
	gtk_action_group_add_actions(action_group, actions, n_actions, parent);
}

static void
ui_morphology_show_dialog(GtkWidget *widget, gpointer data)
{
	ui_morphology_t* obj = ui_morphology_dialog_new((GtkWidget*) data);

	gtk_widget_show_all(obj->dialog);
	gint response = gtk_dialog_run(obj->dialog);
	if(response == GTK_RESPONSE_ACCEPT) {

	} else {

	}
	gtk_widget_destroy(obj->dialog);
	free(obj);
}


static ui_morphology_t*
ui_morphology_dialog_new(GtkWidget *parent)
{
	ui_morphology_t *ret = (ui_morphology_t*) malloc(sizeof(ui_morphology_t));

	GtkWindow* dialog = gtk_dialog_new();
	ret->dialog = dialog;
	memset(ret->custom_element, 0, sizeof(ret->custom_element));

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

	GtkWidget *label = gtk_label_new("Operation");
	GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);

	GtkWidget *combobox = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "erode", "Erode");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "dilate", "Dilate");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "open", "Open");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "close", "Close");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(combobox), FALSE, TRUE, 0);

	label = gtk_label_new("Structural element");  //reusing variable
	gtk_widget_set_margin_top(label, 15);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);

	GtkWidget *horizontal_radio = gtk_radio_button_new_with_label(NULL, "Horizontal line");
	GtkWidget *vertical_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(horizontal_radio),
										"Vertical line");
	GtkWidget *cross_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(vertical_radio),
									     "Cross");
	GtkWidget *custom_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(cross_radio),
									      "Custom");

	GtkWidget *grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

	gtk_grid_attach(GTK_GRID(grid), horizontal_radio, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), vertical_radio, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), cross_radio, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), custom_radio, 0, 3, 1, 1);

	gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 0);

	GtkWidget *horizontal_draw = gtk_drawing_area_new();
	gtk_grid_attach_next_to(GTK_GRID(grid), horizontal_draw, horizontal_radio, GTK_POS_RIGHT,1, 1);
	gtk_widget_set_size_request(GTK_WIDGET(horizontal_draw),
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE,
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE);
	g_signal_connect(GTK_WIDGET(horizontal_draw), "draw",
			 G_CALLBACK(ui_morphology_draw_structural_element),
			 (gpointer) horizontal_line);

	GtkWidget *vertical_draw = gtk_drawing_area_new();
	gtk_grid_attach_next_to(GTK_GRID(grid), vertical_draw, vertical_radio, GTK_POS_RIGHT,1, 1);
	gtk_widget_set_size_request(GTK_WIDGET(vertical_draw),
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE,
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE);
	g_signal_connect(GTK_WIDGET(vertical_draw), "draw",
			 G_CALLBACK(ui_morphology_draw_structural_element),
			 (gpointer) vertical_line);

	GtkWidget *cross_draw = gtk_drawing_area_new();
	gtk_grid_attach_next_to(GTK_GRID(grid), cross_draw, cross_radio, GTK_POS_RIGHT,1, 1);
	gtk_widget_set_size_request(GTK_WIDGET(cross_draw),
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE,
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE);
	g_signal_connect(GTK_WIDGET(cross_draw), "draw",
			 G_CALLBACK(ui_morphology_draw_structural_element),
			 (gpointer) cross);

	GtkWidget *custom_draw = gtk_drawing_area_new();
	gtk_grid_attach_next_to(GTK_GRID(grid), custom_draw, custom_radio, GTK_POS_RIGHT,1, 1);
	gtk_widget_set_size_request(GTK_WIDGET(custom_draw),
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE,
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE);
	g_signal_connect(GTK_WIDGET(custom_draw), "draw",
			 G_CALLBACK(ui_morphology_draw_structural_element),
			 (gpointer) ret->custom_element);
	g_signal_connect(custom_draw, "button-press-event",
			 G_CALLBACK(ui_morphology_button_press), ret);
	gtk_widget_set_events(custom_draw, gtk_widget_get_events(custom_draw)
			      | GDK_BUTTON_PRESS_MASK);
	return ret;
}

static void
ui_morphology_draw_structural_element(
		GtkWidget *widget,
		cairo_t *cr,
		gpointer data)
{
	unsigned int *element = (unsigned int*) data;
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_paint(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	int i,j;
	for(i=0; i<STRUCTURAL_ELEMENT_SIZE; i++) {
		for(j=0; j<STRUCTURAL_ELEMENT_SIZE; j++) {
			if(element[i*STRUCTURAL_ELEMENT_SIZE + j] == 0 ) {
				continue;
			} else {
				double x = PIXEL_SIZE * j;
				double y = PIXEL_SIZE * i;
				cairo_rectangle(cr,x, y, PIXEL_SIZE, PIXEL_SIZE);
			}
		}
	}
	cairo_fill(cr);
}

static gboolean
ui_morphology_button_press(
		GtkWidget *widget,
		GdkEventButton *event,
		gpointer data)
{
	ui_morphology_t* obj = (ui_morphology_t*)(data);
	unsigned int *element = obj->custom_element;
	int x = (int) (event->x / PIXEL_SIZE);
	int y = (int) (event->y / PIXEL_SIZE);
	int idx = y*STRUCTURAL_ELEMENT_SIZE + x;
	element[idx] = (element[idx] + 1) % 2;
	gtk_widget_queue_draw(widget);
	return TRUE;
}
