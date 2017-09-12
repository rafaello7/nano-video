#include <gtk/gtk.h>
#include <gst/gst.h>
#include "nanovideoapp.h"


int main(int argc, char *argv[])
{
    NanoVideoApp *app;
    int status;

	g_set_application_name("Nano Video");
	gst_init (&argc, &argv);
    app = nanovideo_appNew();
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

