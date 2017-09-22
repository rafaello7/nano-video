#ifndef VIDEOCONTROLSWIN_H
#define VIDEOCONTROLSWIN_H


#define VIDEOCONTROLS_WINDOW_TYPE (videocontrols_window_get_type ())
#define VIDEOCONTROLS_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
            VIDEOCONTROLS_WINDOW_TYPE, VideoControlsWindow))


typedef struct VideoControlsWindow         VideoControlsWindow;
typedef struct VideoControlsWindowClass    VideoControlsWindowClass;


GType videocontrols_window_get_type(void);
VideoControlsWindow *videocontrols_windowNew(NanoVideoWindow *owner);

#endif /* VIDEOCONTROLSWIN_H */
