#ifndef NANOVIDEOAPP_H
#define NANOVIDEOAPP_H

#define NANOVIDEO_APP_TYPE (nanovideo_app_get_type())
#define NANOVIDEO_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
            NANOVIDEO_APP_TYPE, NanoVideoApp))

typedef struct NanoVideoApp NanoVideoApp;

GType nanovideo_app_get_type(void);
NanoVideoApp *nanovideo_appNew(void);


#endif /* NANOVIDEOAPP_H */
