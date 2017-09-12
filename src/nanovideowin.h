#ifndef NANOVIDEOWIN_H
#define NANOVIDEOWIN_H


#define NANOVIDEO_WINDOW_TYPE (nanovideo_window_get_type ())
#define NANOVIDEO_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
            NANOVIDEO_WINDOW_TYPE, NanoVideoWindow))


typedef struct NanoVideoWindow         NanoVideoWindow;
typedef struct NanoVideoWindowClass    NanoVideoWindowClass;


GType nanovideo_window_get_type(void);
NanoVideoWindow *nanovideo_windowNew(GtkApplication *app);
void nanovideo_windowPlayUri(NanoVideoWindow*, const gchar *uri);
void nanovideo_windowPauseResume(NanoVideoWindow*);

#endif /* NANOVIDEOWIN_H */
