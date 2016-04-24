#ifndef __AndroidState_h__
#define __AndroidState_h__

enum {
    /**
     * Looper data ID of commands coming from the app's main thread, which
     * is returned as an identifier from ALooper_pollOnce().  The data for this
     * identifier is a pointer to an android_poll_source structure.
     * These can be retrieved and processed with android_app_read_cmd()
     * and android_app_exec_cmd().
     */
            LOOPER_ID_MAIN = 1,

    /**
     * Looper data ID of events coming from the AInputQueue of the
     * application's window, which is returned as an identifier from
     * ALooper_pollOnce().  The data for this identifier is a pointer to an
     * android_poll_source structure.  These can be read via the inputQueue
     * object of android_app.
     */
            LOOPER_ID_INPUT = 2,

    /**
     * Start of user-defined ALooper identifiers.
     */
            LOOPER_ID_USER = 3,
};

enum {
    /**
     * Command from main thread: the AInputQueue has changed.  Upon processing
     * this command, android_app->inputQueue will be updated to the new queue
     * (or NULL).
     */
            APP_CMD_INPUT_CHANGED,

    /**
     * Command from main thread: a new ANativeWindow is ready for use.  Upon
     * receiving this command, android_app->window will contain the new window
     * surface.
     */
            APP_CMD_INIT_WINDOW,

    /**
     * Command from main thread: the existing ANativeWindow needs to be
     * terminated.  Upon receiving this command, android_app->window still
     * contains the existing window; after calling android_app_exec_cmd
     * it will be set to NULL.
     */
            APP_CMD_TERM_WINDOW,

    /**
     * Command from main thread: the current ANativeWindow has been resized.
     * Please redraw with its new size.
     */
            APP_CMD_WINDOW_RESIZED,

    /**
     * Command from main thread: the system needs that the current ANativeWindow
     * be redrawn.  You should redraw the window before handing this to
     * android_app_exec_cmd() in order to avoid transient drawing glitches.
     */
            APP_CMD_WINDOW_REDRAW_NEEDED,

    /**
     * Command from main thread: the content area of the window has changed,
     * such as from the soft input window being shown or hidden.  You can
     * find the new content rect in android_app::contentRect.
     */
            APP_CMD_CONTENT_RECT_CHANGED,

    /**
     * Command from main thread: the app's activity window has gained
     * input focus.
     */
            APP_CMD_GAINED_FOCUS,

    /**
     * Command from main thread: the app's activity window has lost
     * input focus.
     */
            APP_CMD_LOST_FOCUS,

    /**
     * Command from main thread: the current device configuration has changed.
     */
            APP_CMD_CONFIG_CHANGED,

    /**
     * Command from main thread: the system is running low on memory.
     * Try to reduce your memory use.
     */
            APP_CMD_LOW_MEMORY,

    /**
     * Command from main thread: the app's activity has been started.
     */
            APP_CMD_START,

    /**
     * Command from main thread: the app's activity has been resumed.
     */
            APP_CMD_RESUME,

    /**
     * Command from main thread: the app should generate a new saved state
     * for itself, to restore from later if needed.  If you have saved state,
     * allocate it with malloc and place it in android_app.savedState with
     * the size in android_app.savedStateSize.  The will be freed for you
     * later.
     */
            APP_CMD_SAVE_STATE,

    /**
     * Command from main thread: the app's activity has been paused.
     */
            APP_CMD_PAUSE,

    /**
     * Command from main thread: the app's activity has been stopped.
     */
            APP_CMD_STOP,

    /**
     * Command from main thread: the app's activity is being destroyed,
     * and waiting for the app thread to clean up and exit before proceeding.
     */
            APP_CMD_DESTROY,
};



#endif