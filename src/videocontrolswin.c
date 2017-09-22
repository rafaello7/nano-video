#include <gtk/gtk.h>
#include <gst/gst.h>
#include "nanovideowin.h"
#include "videocontrolswin.h"


struct VideoControlsWindow {
    GtkWindow parent;
};

struct VideoControlsWindowClass {
    GtkWindowClass parent_class;
};

typedef struct {
	NanoVideoWindow *owner;
	GtkAdjustment *adjustmentBrightness;
	GtkAdjustment *adjustmentContrast;
	GtkAdjustment *adjustmentSaturation;
	GtkAdjustment *adjustmentHue;
	GtkAdjustment *adjustmentAlpha;
	GtkAdjustment *adjustmentLayerZOrder;
} VideoControlsWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(VideoControlsWindow, videocontrols_window,
        GTK_TYPE_WINDOW);

static void videocontrols_window_init(VideoControlsWindow *win)
{
	VideoControlsWindowPrivate *priv;

    gtk_widget_init_template(GTK_WIDGET(win));
	priv = videocontrols_window_get_instance_private(win);
	priv->owner = NULL;
}

static void videocontrols_window_dispose(GObject *object)
{
	//VideoControlsWindow *win = VIDEOCONTROLS_WINDOW(object);

    G_OBJECT_CLASS(videocontrols_window_parent_class)->dispose(object);
}

static void videocontrols_window_class_init(VideoControlsWindowClass *videocontrolsClass)
{
    GtkWidgetClass *widgetClass = GTK_WIDGET_CLASS(videocontrolsClass);

    G_OBJECT_CLASS(videocontrolsClass)->dispose = videocontrols_window_dispose;
    gtk_widget_class_set_template_from_resource(widgetClass,
            "/org/rafaello7/nanovideo/videocontrolswin.ui");
    gtk_widget_class_bind_template_child_private(widgetClass,
            VideoControlsWindow, adjustmentBrightness);
    gtk_widget_class_bind_template_child_private(widgetClass,
            VideoControlsWindow, adjustmentContrast);
    gtk_widget_class_bind_template_child_private(widgetClass,
            VideoControlsWindow, adjustmentSaturation);
    gtk_widget_class_bind_template_child_private(widgetClass,
            VideoControlsWindow, adjustmentHue);
    gtk_widget_class_bind_template_child_private(widgetClass,
            VideoControlsWindow, adjustmentAlpha);
    gtk_widget_class_bind_template_child_private(widgetClass,
            VideoControlsWindow, adjustmentLayerZOrder);
}

void on_adjustmentBrightness_value_changed(GtkAdjustment *adjustment,
		VideoControlsWindow *win)
{
	VideoControlsWindowPrivate *priv;

	priv = videocontrols_window_get_instance_private(win);
	if( priv->owner ) {
		nanovideo_windowSetControlParam(priv->owner, "brightness",
				gtk_adjustment_get_value(adjustment));
	}
}

void on_adjustmentContrast_value_changed(GtkAdjustment *adjustment,
		VideoControlsWindow *win)
{
	VideoControlsWindowPrivate *priv;

	priv = videocontrols_window_get_instance_private(win);
	if( priv->owner ) {
		nanovideo_windowSetControlParam(priv->owner, "contrast",
				gtk_adjustment_get_value(adjustment));
	}
}

void on_adjustmentSaturation_value_changed(GtkAdjustment *adjustment,
		VideoControlsWindow *win)
{
	VideoControlsWindowPrivate *priv;

	priv = videocontrols_window_get_instance_private(win);
	if( priv->owner ) {
		nanovideo_windowSetControlParam(priv->owner, "saturation",
				gtk_adjustment_get_value(adjustment));
	}
}

void on_adjustmentHue_value_changed(GtkAdjustment *adjustment,
		VideoControlsWindow *win)
{
	VideoControlsWindowPrivate *priv;

	priv = videocontrols_window_get_instance_private(win);
	if( priv->owner ) {
		nanovideo_windowSetControlParam(priv->owner, "hue",
				gtk_adjustment_get_value(adjustment));
	}
}

void on_adjustmentAlpha_value_changed(GtkAdjustment *adjustment,
		VideoControlsWindow *win)
{
	VideoControlsWindowPrivate *priv;

	priv = videocontrols_window_get_instance_private(win);
	if( priv->owner ) {
		nanovideo_windowSetExtraControlParam(priv->owner, "alpha",
				gtk_adjustment_get_value(adjustment));
	}
}

void on_adjustmentLayerZOrder_value_changed(GtkAdjustment *adjustment,
		VideoControlsWindow *win)
{
	VideoControlsWindowPrivate *priv;

	priv = videocontrols_window_get_instance_private(win);
	if( priv->owner ) {
		nanovideo_windowSetExtraControlParam(priv->owner, "layer_z_order",
				gtk_adjustment_get_value(adjustment));
	}
}

void on_buttonReset_clicked(GtkButton *button, VideoControlsWindow *win)
{
	VideoControlsWindowPrivate *priv;

	priv = videocontrols_window_get_instance_private(win);
	gtk_adjustment_set_value(priv->adjustmentBrightness, 0);
	gtk_adjustment_set_value(priv->adjustmentContrast, 0);
	gtk_adjustment_set_value(priv->adjustmentSaturation, 64);
	gtk_adjustment_set_value(priv->adjustmentHue, 0);
	gtk_adjustment_set_value(priv->adjustmentAlpha, 255);
	gtk_adjustment_set_value(priv->adjustmentLayerZOrder, 3);
}

VideoControlsWindow *videocontrols_windowNew(NanoVideoWindow *owner)
{
	VideoControlsWindowPrivate *priv;

    VideoControlsWindow *win = g_object_new(VIDEOCONTROLS_WINDOW_TYPE, NULL);
	gtk_window_set_transient_for(GTK_WINDOW(win), GTK_WINDOW(owner));
	priv = videocontrols_window_get_instance_private(win);
	priv->owner = owner;
	gtk_adjustment_set_value(priv->adjustmentBrightness,
			nanovideo_windowGetControlParam(owner, "brightness"));
	gtk_adjustment_set_value(priv->adjustmentContrast,
			nanovideo_windowGetControlParam(owner, "contrast"));
	gtk_adjustment_set_value(priv->adjustmentSaturation,
			nanovideo_windowGetControlParam(owner, "saturation"));
	gtk_adjustment_set_value(priv->adjustmentHue,
			nanovideo_windowGetControlParam(owner, "hue"));
	gtk_adjustment_set_value(priv->adjustmentAlpha, 255);
    return win;
}

