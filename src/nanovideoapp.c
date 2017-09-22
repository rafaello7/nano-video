#include <gtk/gtk.h>
#include "nanovideoapp.h"
#include "nanovideowin.h"


struct NanoVideoApp {
    GtkApplication parent;
};

typedef struct {
    GtkApplicationClass parent_class;
} NanoVideoAppClass;

typedef struct {
	NanoVideoWindow *win;
} NanoVideoAppPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(NanoVideoApp, nanovideo_app, GTK_TYPE_APPLICATION);


static void nanovideo_app_init(NanoVideoApp *app)
{
	NanoVideoAppPrivate *priv;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
	priv->win = NULL;
}

static void nanovideo_app_startup(GApplication *app)
{
    GtkBuilder *menuBld;
    GMenuModel *menuBar;

    G_APPLICATION_CLASS(nanovideo_app_parent_class)->startup(app);
    menuBld = gtk_builder_new_from_resource(
            "/org/rafaello7/nanovideo/menubar.ui");
    menuBar = G_MENU_MODEL(gtk_builder_get_object(menuBld, "menuBar"));
    gtk_application_set_menubar(GTK_APPLICATION(app), menuBar);
    g_object_unref(menuBld);
}

static void nanovideo_app_activate(GApplication *app)
{
	NanoVideoAppPrivate *priv;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
	if( priv->win == NULL )
		priv->win = nanovideo_windowNew(GTK_APPLICATION(app));
	else
		gtk_window_present(GTK_WINDOW (priv->win));
}

static void nanovideo_app_open(GApplication  *app,
        GFile **files, gint n_files, const gchar *hint)
{
	NanoVideoAppPrivate *priv;
	char *uri;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
	if( priv->win == NULL )
		priv->win = nanovideo_windowNew(GTK_APPLICATION(app));
	uri = g_file_get_uri(files[0]);
	nanovideo_windowPlayUri(priv->win, uri);
	g_free(uri);
}

static void nanovideo_app_class_init (NanoVideoAppClass *class)
{
	G_APPLICATION_CLASS(class)->startup = nanovideo_app_startup;
    G_APPLICATION_CLASS(class)->activate = nanovideo_app_activate;
    G_APPLICATION_CLASS(class)->open = nanovideo_app_open;
}

static void on_menu_open(GSimpleAction *action, GVariant *parameter,
        gpointer app)
{
    char *uri;
    NanoVideoAppPrivate *priv;
	GtkWidget *dialog;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
	dialog = gtk_file_chooser_dialog_new("Open File",
			GTK_WINDOW(priv->win), GTK_FILE_CHOOSER_ACTION_OPEN,
			"_Cancel", GTK_RESPONSE_CANCEL,
			"_Open", GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), FALSE);
	if( gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		uri = gtk_file_chooser_get_uri(chooser);
        nanovideo_windowPlayUri(priv->win, uri);
        g_free(uri);
	}
	gtk_widget_destroy (dialog);
}

static void paste_received(GtkClipboard *clipboard,
		const gchar *text, gpointer user_data)
{
	gchar *uri = NULL;

	if( text == NULL )
		return;
	if( text[0] == '/' ) {
		uri = g_strdup_printf("file://%s", text);
		text = uri;
	}
	nanovideo_windowPlayUri(user_data, text);
	g_free(uri);
}

static void on_menu_open_selection(GSimpleAction *action, GVariant *parameter,
        gpointer app)
{
    NanoVideoAppPrivate *priv;
	GtkClipboard *clipboard;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
	clipboard = gtk_widget_get_clipboard(GTK_WIDGET(priv->win),
			GDK_SELECTION_PRIMARY);
	gtk_clipboard_request_text(clipboard, paste_received, priv->win);
}

static void on_menu_open_clipboard(GSimpleAction *action, GVariant *parameter,
        gpointer app)
{
    NanoVideoAppPrivate *priv;
	GtkClipboard *clipboard;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
	clipboard = gtk_widget_get_clipboard(GTK_WIDGET(priv->win),
			GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_request_text(clipboard, paste_received, priv->win);
}

static void on_menu_quit(GSimpleAction *action, GVariant *parameter,
        gpointer app)
{
	NanoVideoAppPrivate *priv;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
    gtk_window_close(GTK_WINDOW(priv->win));
}

static void on_menu_pause_resume(GSimpleAction *action, GVariant *parameter,
        gpointer app)
{
	NanoVideoAppPrivate *priv;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
    nanovideo_windowPauseResume(priv->win);
}

static void on_menu_show_controls(GSimpleAction *action, GVariant *parameter,
        gpointer app)
{
	NanoVideoAppPrivate *priv;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
    nanovideo_windowShowControls(priv->win);
}

static void on_menu_fullscreen(GSimpleAction *action, GVariant *parameter,
        gpointer app)
{
	NanoVideoAppPrivate *priv;
	GdkWindowState winState;

	priv = nanovideo_app_get_instance_private(NANOVIDEO_APP(app));
	winState = gdk_window_get_state(
			gtk_widget_get_window(GTK_WIDGET(priv->win)));
	if( winState & GDK_WINDOW_STATE_FULLSCREEN )
		gtk_window_unfullscreen(GTK_WINDOW(priv->win));
	else
		gtk_window_fullscreen(GTK_WINDOW(priv->win));
}

NanoVideoApp *nanovideo_appNew(void)
{
    static GActionEntry app_entries[] = {
        { "open", on_menu_open, NULL, NULL, NULL },
        { "open-selection", on_menu_open_selection, NULL, NULL, NULL },
        { "open-clipboard", on_menu_open_clipboard, NULL, NULL, NULL },
        { "quit", on_menu_quit, NULL, NULL, NULL },
        { "pause-resume", on_menu_pause_resume, NULL, NULL, NULL },
        { "show-controls", on_menu_show_controls, NULL, NULL, NULL },
        { "fullscreen", on_menu_fullscreen, NULL, NULL, NULL },
    };
	NanoVideoApp *app;

    app = g_object_new (NANOVIDEO_APP_TYPE,
            "application-id", "org.rafaello7.nanovideo",
            "flags", G_APPLICATION_HANDLES_OPEN, NULL);
    g_action_map_add_action_entries(G_ACTION_MAP(app),
            app_entries, G_N_ELEMENTS(app_entries), app);
	return app;
}

