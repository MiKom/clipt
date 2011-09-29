#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <ui/ui.h>

ui_widget_t* ui_widget_init(ui_widget_t* widget, const char* title, int w, int h)
{
	ui_widget_t* result = widget;
	if(!result)
		result = malloc(sizeof(ui_widget_t));
	
	strncpy(result->title, title, 256);
	result->width  = w;
	result->height = h;
	result->widget = NULL;
	return result;
}

ui_widget_t* ui_widget_defaults(ui_widget_t* widget, const char* title, int w, int h)
{
	ui_widget_t* result = widget;
	if(!result)
		result = malloc(sizeof(ui_widget_t));
	
	if(!*result->title)
		strncpy(result->title, title, 256);
	
	if(!result->width)  result->width  = w;
	if(!result->height) result->height = h;
	return result;
}

void ui_widget_free(ui_widget_t* widget)
{
	if(widget->widget)
		gtk_widget_destroy(widget->widget);
	free(widget);
}
