#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gdk/gdkx.h>
#include <gst/video/videooverlay.h>
#include <gst/base/gstbasesink.h>
#include "nanovideowin.h"
#include <string.h>


struct NanoVideoWindow {
    GtkApplicationWindow parent;
};

struct NanoVideoWindowClass {
    GtkApplicationWindowClass parent_class;
};

typedef struct {
	GstElement *playbin;
	GstElement *videosink;
	GtkDrawingArea *drawingArea;
	guint videoWidth, videoHeight;
	GstState desiredState;
	gboolean isFullScreen;
} NanoVideoWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(NanoVideoWindow, nanovideo_window,
        GTK_TYPE_APPLICATION_WINDOW);

static void state_changed_cb(GstBus *bus, GstMessage *msg, NanoVideoWindow *win)
{
	NanoVideoWindowPrivate *priv;
	GstState old_state, new_state, pending_state;
	GstPad *sinkPad;
	GstCaps *sinkCaps;
	GstStructure *sinkStruct;
	gint width = 0, height = 0;

	priv = nanovideo_window_get_instance_private(win);
	if( priv->videoWidth == 0 || priv->videoHeight == 0 ) {
		/* resize window after video startup to the video size */
		gst_message_parse_state_changed(msg, &old_state, &new_state,
				&pending_state);
		if(GST_MESSAGE_SRC (msg) == GST_OBJECT (priv->playbin)) {
			if( new_state == GST_STATE_PLAYING && priv->videoWidth == 0 ) {
				sinkPad = gst_element_get_static_pad(priv->videosink, "sink");
				sinkCaps = gst_pad_get_current_caps(sinkPad);
				if( sinkCaps != NULL ) {
					sinkStruct = gst_caps_get_structure(sinkCaps, 0);
					gst_structure_get_int(sinkStruct, "width", &width);
					gst_structure_get_int(sinkStruct, "height", &height);
					if( width && height ) {
						priv->videoWidth = width;
						priv->videoHeight = height;
						gtk_widget_translate_coordinates(
								GTK_WIDGET(priv->drawingArea),
								GTK_WIDGET(win), width, height,
								&width, &height);
						gtk_window_resize(GTK_WINDOW(win), width, height);
					}
				}
			}
		}
	}
}

static void error_cb(GstBus *bus, GstMessage *msg, NanoVideoWindow *win)
{
	NanoVideoWindowPrivate *priv;
	GError *err;
	gchar *debug_info;
	GtkWidget *dialog;

	priv = nanovideo_window_get_instance_private(win);
	gst_message_parse_error(msg, &err, &debug_info);
	dialog = gtk_message_dialog_new(GTK_WINDOW(win),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
			"Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src),
			err->message);
	if( debug_info )
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
				"Debug information: %s", debug_info);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_clear_error (&err);
	g_free (debug_info);
	gst_element_set_state (priv->playbin, GST_STATE_READY);
}

static void on_drawingArea_realize(GtkWidget *drawingArea, gpointer user_data)
{
	NanoVideoWindow *win = user_data;
	NanoVideoWindowPrivate *priv;
	GdkWindow *window;
	guintptr window_handle;

	priv = nanovideo_window_get_instance_private(win);
	window = gtk_widget_get_window(GTK_WIDGET(priv->drawingArea));
	window_handle = GDK_WINDOW_XID (window);
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(priv->playbin),
			window_handle);
}

static void clearVideoSinkUnlockFuncs(GstElement *videosink)
{
	GstBaseSinkClass *videoSinkClass;

	/* v4l2sink bug workaround: the sink unlock function may cause inability
	 * to resume video play after pause
	 */
	videoSinkClass = GST_BASE_SINK_CLASS(G_OBJECT_GET_CLASS(
				G_OBJECT(videosink)));
	videoSinkClass->unlock = NULL;
	videoSinkClass->unlock_stop = NULL;
}

static void adjustVideoLayerPriority(NanoVideoWindowPrivate *priv)
{
	GstStructure *extraControls;

	if( priv->videosink == NULL )
		return;
	extraControls = gst_structure_new("extra-controls", "video_layer_z_order",
			G_TYPE_INT, priv->isFullScreen ? 0 : 3, NULL);
	g_object_set(priv->videosink, "extra-controls", extraControls, NULL);
	gst_structure_free(extraControls);
}

static void element_setup(GstElement *object, GstElement *arg0,
		gpointer user_data)
{
	/* Configure v4l2videodec plugin to use buffers from v4l2sink.
	 * This way decoded video data is not copied between private buffers
	 * of each element but the video decoder device stores decoded image
	 * directly in buffer from v4l2sink pool.
	 */
	if( !strncmp(gst_object_get_name(GST_OBJECT(arg0)), "v4l2video", 9) ) {
		g_object_set(G_OBJECT(arg0), "capture-io-mode", 5, NULL);
	}
}

static void nanovideo_window_init(NanoVideoWindow *win)
{
    NanoVideoWindowPrivate *priv;
	GstBus *bus;

    gtk_widget_init_template(GTK_WIDGET(win));
    priv = nanovideo_window_get_instance_private(win);
	priv->playbin = gst_element_factory_make ("playbin", "playbin");
	if( priv->playbin ) {
		bus = gst_element_get_bus (priv->playbin);
		gst_bus_add_signal_watch (bus);
		g_signal_connect(G_OBJECT(bus), "message::error",
				(GCallback)error_cb, win);
		/* Use video layer of s5p6818 soc to render video. The video layer
		 * is available as /dev/video1. Use v4l2sink to control the layer */
		priv->videosink = gst_element_factory_make ("v4l2sink", "v4l2sink");
		if( priv->videosink ) {
			clearVideoSinkUnlockFuncs(priv->videosink);
			g_object_set(priv->videosink, "io-mode", 4, NULL);
			g_object_set(priv->playbin, "video-sink", priv->videosink, NULL);
			g_signal_connect(G_OBJECT (bus), "message::state-changed",
					(GCallback)state_changed_cb, win);
			g_signal_connect(G_OBJECT (priv->playbin), "element-setup",
					(GCallback)element_setup, NULL);
		}else{
			/* fallback */
			g_signal_connect(priv->drawingArea, "realize",
					G_CALLBACK(on_drawingArea_realize), win);
		}
	}
	priv->videoWidth = priv->videoHeight = 0;
	priv->desiredState = GST_STATE_NULL;
	priv->isFullScreen = FALSE;
	adjustVideoLayerPriority(priv);
}

static void nanovideo_window_dispose(GObject *object)
{
    NanoVideoWindow *win;
    NanoVideoWindowPrivate *priv;

    /* note: nanovideo_window_dispose may be invoked more than once
     * for the same object
     */
    win = NANOVIDEO_WINDOW(object);
    priv = nanovideo_window_get_instance_private(win);
	if( priv->isFullScreen ) {
		priv->isFullScreen = FALSE;
		adjustVideoLayerPriority(priv);
	}
	if( priv->playbin ) {
		priv->desiredState = GST_STATE_NULL;
		gst_element_set_state(priv->playbin, priv->desiredState);
		gst_object_unref(priv->playbin);
		priv->playbin = NULL;
	}
    G_OBJECT_CLASS(nanovideo_window_parent_class)->dispose(object);
}

static void nanovideo_window_class_init(NanoVideoWindowClass *nanovideoClass)
{
    GtkWidgetClass *widgetClass = GTK_WIDGET_CLASS(nanovideoClass);
    G_OBJECT_CLASS(nanovideoClass)->dispose = nanovideo_window_dispose;
    gtk_widget_class_set_template_from_resource(widgetClass,
            "/org/rafaello7/nanovideo/nanovideowin.ui");
    gtk_widget_class_bind_template_child_private(widgetClass,
            NanoVideoWindow, drawingArea);
}

/* Set video layer position and size
 */
static void setVideoLayerCoord(NanoVideoWindowPrivate *priv,
		gint x, gint y, gint width, gint height)
{
	gint adj;

	if( priv->videosink == NULL )
		return;
	if( priv->videoWidth && priv->videoHeight ) {
		if( width * priv->videoHeight < height * priv->videoWidth ) {
			adj = width * priv->videoHeight / priv->videoWidth;
			y += (height - adj) / 2;
			height = adj;
		}else{
			adj = height * priv->videoWidth / priv->videoHeight;
			x += (width - adj) / 2;
			width = adj;
		}
	}
	g_object_set(priv->videosink, "overlay-left", x, "overlay-top", y,
			"overlay-width", width, "overlay-height", height, NULL);
}

gboolean on_NanoVideoWindow_configure_event(GtkWidget *widget, GdkEvent *ev,
		gpointer data)
{
    NanoVideoWindowPrivate *priv;
	gint x, y, width, height;

	/* track window moving/resizing to move/resize video layer respectively */
	priv = nanovideo_window_get_instance_private(NANOVIDEO_WINDOW(widget));
	x = ev->configure.x;
	y = ev->configure.y;
	width = ev->configure.width;
	height = ev->configure.height;
	if( ! priv->isFullScreen ) {
		// keep the video layer inside drawing area widget
		gtk_widget_translate_coordinates(GTK_WIDGET(priv->drawingArea),
				widget, x, y, &x, &y);
		width -= x - ev->configure.x;
		height -= y - ev->configure.y;
	}
	setVideoLayerCoord(priv, x, y, width, height);
	return FALSE;
}

gboolean on_drawingArea_configure_event(GtkWidget *widget, GdkEvent *ev,
		gpointer data)
{
	NanoVideoWindow *mainWin = data;
    NanoVideoWindowPrivate *priv;
	gint xmain, ymain;

	/* track drawing area widget moving/resizing to move/resize video layer
	 * respectively */
	priv = nanovideo_window_get_instance_private(mainWin);
	if( ! priv->isFullScreen ) {
		gdk_window_get_position(gtk_widget_get_window(GTK_WIDGET(mainWin)),
					&xmain, &ymain);
		setVideoLayerCoord(priv,
			ev->configure.x + xmain, ev->configure.y + ymain,
			ev->configure.width, ev->configure.height);
	}
	return FALSE;
}

gboolean on_NanoVideoWindow_window_state_event(GtkWidget *widget,
		GdkEvent *event, gpointer user_data)
{
    NanoVideoWindowPrivate *priv;
	gint winx, winy, x, y, width, height;

	/* handle fullscreen mode; in fullscreen mode the video layer z-order
	 * is changed in MLC of the s5p6818 soc from lowest to highest */
	priv = nanovideo_window_get_instance_private(NANOVIDEO_WINDOW(widget));
	priv->isFullScreen = event->window_state.new_window_state &
		GDK_WINDOW_STATE_FULLSCREEN;
	gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(widget),
			!priv->isFullScreen);
	gdk_window_get_position(event->window_state.window, &winx, &winy);
	width = gdk_window_get_width(event->window_state.window);
	height = gdk_window_get_height(event->window_state.window);
	if( priv->isFullScreen ) {
		x = winx;
		y = winy;
	}else{
		// keep the video layer inside drawing area widget
		gtk_widget_translate_coordinates(GTK_WIDGET(priv->drawingArea),
				widget, winx, winy, &x, &y);
		width -= x - winx;
		height -= y - winy;
	}
	setVideoLayerCoord(priv, x, y, width, height);
	adjustVideoLayerPriority(priv);
	return FALSE;
}

gboolean on_drawingArea_draw(GtkWidget *drawing, cairo_t *cr,
		gpointer user_data)
{
	/* Fill drawing area with transparent color. This way the video layer
	 * is made visible */
	cairo_rectangle(cr, 0, 0, gtk_widget_get_allocated_width(drawing),
			gtk_widget_get_allocated_height(drawing));
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_fill(cr);
	return TRUE;
}

void nanovideo_windowPlayUri(NanoVideoWindow *win, const gchar *uri)
{
    NanoVideoWindowPrivate *priv;

    priv = nanovideo_window_get_instance_private(win);
	if( priv->playbin == NULL )
		return;
	gst_element_set_state(priv->playbin, GST_STATE_NULL);
	priv->videoWidth = priv->videoHeight = 0;
	g_object_set (priv->playbin, "uri", uri, NULL);
	priv->desiredState = GST_STATE_PLAYING;
	gst_element_set_state(priv->playbin, priv->desiredState);
}

void nanovideo_windowPauseResume(NanoVideoWindow *win)
{
    NanoVideoWindowPrivate *priv;

    priv = nanovideo_window_get_instance_private(win);
	if( priv->playbin == NULL )
		return;
	switch( priv->desiredState ) {
	case GST_STATE_PLAYING:
		priv->desiredState = GST_STATE_PAUSED;
		gst_element_set_state(priv->playbin, priv->desiredState);
		break;
	case GST_STATE_PAUSED:
		priv->desiredState = GST_STATE_PLAYING;
		gst_element_set_state(priv->playbin, priv->desiredState);
	default:
		break;
	}
}

NanoVideoWindow *nanovideo_windowNew(GtkApplication *app)
{
    GdkPixbuf *icon;

    NanoVideoWindow *mainWin = g_object_new(NANOVIDEO_WINDOW_TYPE,
            "application", app, NULL);
    icon = gdk_pixbuf_new_from_resource(
            "/org/rafaello7/nanovideo/nano-video.svg", NULL);
    gtk_window_set_icon(GTK_WINDOW(mainWin), icon);
    g_object_unref(icon);
    gtk_widget_show_all(GTK_WIDGET(mainWin));
    return mainWin;
}

