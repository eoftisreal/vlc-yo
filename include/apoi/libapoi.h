/*****************************************************************************
 * libapoi.h:  libapoi external API
 *****************************************************************************
 * Copyright (C) 1998-2009 VLC authors and VideoLAN
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Jean-Paul Saman <jpsaman@videolan.org>
 *          Pierre d'Herbemont <pdherbemont@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/**
 * \defgroup libapoi LibAPOI
 * LibAPOI is the external programming interface of the VLC media player.
 * It is used to embed VLC into other applications or frameworks.
 * @{
 * \file
 * LibAPOI core external API
 */

#ifndef VLC_LIBAPOI_H
#define VLC_LIBAPOI_H 1

#if (defined (_WIN32) || defined (__OS2__)) && defined (LIBAPOI_DLL_EXPORT)
# define LIBAPOI_API __declspec(dllexport)
#elif defined (__GNUC__) && (__GNUC__ >= 4)
# define LIBAPOI_API __attribute__((visibility("default")))
#else
# define LIBAPOI_API
#endif

#ifdef LIBAPOI_INTERNAL_
/* Avoid unhelpful warnings from libapoi with our deprecated APIs */
#   define LIBAPOI_DEPRECATED
#elif defined(__GNUC__) && \
      (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
# define LIBAPOI_DEPRECATED __attribute__((deprecated))
#else
# define LIBAPOI_DEPRECATED
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

/** \defgroup libapoi_core LibAPOI core
 * \ingroup libapoi
 * Before it can do anything useful, LibAPOI must be initialized.
 * You can create one (or more) instance(s) of LibAPOI in a given process,
 * with libapoi_new() and destroy them with libapoi_release().
 *
 * \version Unless otherwise stated, these functions are available
 * from LibAPOI versions numbered 1.1.0 or more.
 * Earlier versions (0.9.x and 1.0.x) are <b>not</b> compatible.
 * @{
 */

/** This structure is opaque. It represents a libapoi instance */
typedef struct libapoi_instance_t libapoi_instance_t;

typedef int64_t libapoi_time_t;

/** \defgroup libapoi_error LibAPOI error handling
 * @{
 */

/**
 * A human-readable error message for the last LibAPOI error in the calling
 * thread. The resulting string is valid until another error occurs (at least
 * until the next LibAPOI call).
 *
 * @warning
 * This will be NULL if there was no error.
 */
LIBAPOI_API const char *libapoi_errmsg (void);

/**
 * Clears the LibAPOI error status for the current thread. This is optional.
 * By default, the error status is automatically overridden when a new error
 * occurs, and destroyed when the thread exits.
 */
LIBAPOI_API void libapoi_clearerr (void);

/**
 * Sets the LibAPOI error status and message for the current thread.
 * Any previous error is overridden.
 * \param fmt the format string
 * \param ...  the arguments for the format string
 * \return a nul terminated string in any case
 */
const char *libapoi_printerr (const char *fmt, ...);

/**@} */

/**
 * Create and initialize a libapoi instance.
 * This functions accept a list of "command line" arguments similar to the
 * main(). These arguments affect the LibAPOI instance default configuration.
 *
 * \note
 * LibAPOI may create threads. Therefore, any thread-unsafe process
 * initialization must be performed before calling libapoi_new(). In particular
 * and where applicable:
 * - setlocale() and textdomain(),
 * - setenv(), unsetenv() and putenv(),
 * - with the X11 display system, XInitThreads()
 *   (see also libapoi_media_player_set_xwindow()) and
 * - on Microsoft Windows, SetErrorMode().
 * - sigprocmask() shall never be invoked; pthread_sigmask() can be used.
 *
 * On POSIX systems, the SIGCHLD signal <b>must not</b> be ignored, i.e. the
 * signal handler must set to SIG_DFL or a function pointer, not SIG_IGN.
 * Also while LibAPOI is active, the wait() function shall not be called, and
 * any call to waitpid() shall use a strictly positive value for the first
 * parameter (i.e. the PID). Failure to follow those rules may lead to a
 * deadlock or a busy loop.
 * Also on POSIX systems, it is recommended that the SIGPIPE signal be blocked,
 * even if it is not, in principles, necessary, e.g.:
 * @code
   sigset_t set;

   signal(SIGCHLD, SIG_DFL);
   sigemptyset(&set);
   sigaddset(&set, SIGPIPE);
   pthread_sigmask(SIG_BLOCK, &set, NULL);
 * @endcode
 *
 * On Microsoft Windows, setting the default DLL directories to SYSTEM32
 * exclusively is strongly recommended for security reasons:
 * @code
   SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);
 * @endcode
 *
 * \version
 * Arguments are meant to be passed from the command line to LibAPOI, just like
 * VLC media player does. The list of valid arguments depends on the LibAPOI
 * version, the operating system and platform, and set of available LibAPOI
 * plugins. Invalid or unsupported arguments will cause the function to fail
 * (i.e. return NULL). Also, some arguments may alter the behaviour or
 * otherwise interfere with other LibAPOI functions.
 *
 * \warning
 * There is absolutely no warranty or promise of forward, backward and
 * cross-platform compatibility with regards to libapoi_new() arguments.
 * We recommend that you do not use them, other than when debugging.
 *
 * \param argc the number of arguments (should be 0)
 * \param argv list of arguments (should be NULL)
 * \return the libapoi instance or NULL in case of error
 */
LIBAPOI_API libapoi_instance_t *
libapoi_new( int argc , const char *const *argv );

/**
 * Decrement the reference count of a libapoi instance, and destroy it
 * if it reaches zero.
 *
 * \param p_instance the instance to destroy
 */
LIBAPOI_API void libapoi_release( libapoi_instance_t *p_instance );

/**
 * Increments the reference count of a libapoi instance.
 * The initial reference count is 1 after libapoi_new() returns.
 *
 * \param p_instance the instance to reference
 * \return the same object
 */
LIBAPOI_API libapoi_instance_t *libapoi_retain( libapoi_instance_t *p_instance );

/**
 * Get the ABI version of the libapoi library.
 *
 * This is different than the VLC version, which is the version of the whole
 * VLC package. The value is the same as LIBAPOI_ABI_VERSION_INT used when
 * compiling.
 *
 * \return a value with the following mask in hexadecimal
 *  0xFF000000: major VLC version, similar to VLC major version,
 *  0x00FF0000: major ABI version, incremented incompatible changes are added,
 *  0x0000FF00: minor ABI version, incremented when new functions are added
 *  0x000000FF: micro ABI version, incremented with new release/builds
 *
 * \note This the same value as the .so version but cross platform.
 */
LIBAPOI_API int libapoi_abi_version(void);

/**
 * Sets the application name. LibAPOI passes this as the user agent string
 * when a protocol requires it.
 *
 * \param p_instance LibAPOI instance
 * \param name human-readable application name, e.g. "FooBar player 1.2.3"
 * \param http HTTP User Agent, e.g. "FooBar/1.2.3 Python/2.6.0"
 * \version LibAPOI 1.1.1 or later
 */
LIBAPOI_API
void libapoi_set_user_agent( libapoi_instance_t *p_instance,
                            const char *name, const char *http );

/**
 * Sets some meta-information about the application.
 * See also libapoi_set_user_agent().
 *
 * \param p_instance LibAPOI instance
 * \param id Java-style application identifier, e.g. "com.acme.foobar"
 * \param version application version numbers, e.g. "1.2.3"
 * \param icon application icon name, e.g. "foobar"
 * \version LibAPOI 2.1.0 or later.
 */
LIBAPOI_API
void libapoi_set_app_id( libapoi_instance_t *p_instance, const char *id,
                        const char *version, const char *icon );

/**
 * Retrieve libapoi version.
 *
 * Example: "1.1.0-git The Luggage"
 *
 * \return a string containing the libapoi version
 */
LIBAPOI_API const char * libapoi_get_version(void);

/**
 * Retrieve libapoi compiler version.
 *
 * Example: "gcc version 4.2.3 (Ubuntu 4.2.3-2ubuntu6)"
 *
 * \return a string containing the libapoi compiler version
 */
LIBAPOI_API const char * libapoi_get_compiler(void);

/**
 * Retrieve libapoi changeset.
 *
 * Example: "aa9bce0bc4"
 *
 * \return a string containing the libapoi changeset
 */
LIBAPOI_API const char * libapoi_get_changeset(void);

/**
 * Frees an heap allocation returned by a LibAPOI function.
 * If you know you're using the same underlying C run-time as the LibAPOI
 * implementation, then you can call ANSI C free() directly instead.
 *
 * \param ptr the pointer
 */
LIBAPOI_API void libapoi_free( void *ptr );

/** \defgroup libapoi_event LibAPOI asynchronous events
 * LibAPOI emits asynchronous events.
 *
 * Several LibAPOI objects (such @ref libapoi_instance_t as
 * @ref libapoi_media_player_t) generate events asynchronously. Each of them
 * provides @ref libapoi_event_manager_t event manager. You can subscribe to
 * events with libapoi_event_attach() and unsubscribe with
 * libapoi_event_detach().
 * @{
 */

/**
 * Event manager that belongs to a libapoi object, and from whom events can
 * be received.
 */
typedef struct libapoi_event_manager_t libapoi_event_manager_t;

struct libapoi_event_t;

/**
 * Type of a LibAPOI event.
 */
typedef int libapoi_event_type_t;

/**
 * Callback function notification
 * \param p_event the event triggering the callback
 */
typedef void ( *libapoi_callback_t )( const struct libapoi_event_t *p_event, void *p_data );

/**
 * Register for an event notification.
 *
 * \param p_event_manager the event manager to which you want to attach to.
 *        Generally it is obtained by vlc_my_object_event_manager() where
 *        my_object is the object you want to listen to.
 * \param i_event_type the desired event to which we want to listen
 * \param f_callback the function to call when i_event_type occurs
 * \param user_data user provided data to carry with the event
 * \return 0 on success, ENOMEM on error
 */
LIBAPOI_API int libapoi_event_attach( libapoi_event_manager_t *p_event_manager,
                                        libapoi_event_type_t i_event_type,
                                        libapoi_callback_t f_callback,
                                        void *user_data );

/**
 * Unregister an event notification.
 *
 * \param p_event_manager the event manager
 * \param i_event_type the desired event to which we want to unregister
 * \param f_callback the function to call when i_event_type occurs
 * \param p_user_data user provided data to carry with the event
 */
LIBAPOI_API void libapoi_event_detach( libapoi_event_manager_t *p_event_manager,
                                         libapoi_event_type_t i_event_type,
                                         libapoi_callback_t f_callback,
                                         void *p_user_data );

/** @} */

/** \defgroup libapoi_log LibAPOI logging
 * libapoi_log_* functions provide access to the LibAPOI messages log.
 * This is used for logging and debugging.
 * @{
 */

/**
 * Logging messages level.
 * \note Future LibAPOI versions may define new levels.
 */
enum libapoi_log_level
{
    LIBAPOI_DEBUG=0,   /**< Debug message */
    LIBAPOI_NOTICE=2,  /**< Important informational message */
    LIBAPOI_WARNING=3, /**< Warning (potential error) message */
    LIBAPOI_ERROR=4    /**< Error message */
};

typedef struct vlc_log_t libapoi_log_t;

/**
 * Gets log message debug infos.
 *
 * This function retrieves self-debug information about a log message:
 * - the name of the VLC module emitting the message,
 * - the name of the source code module (i.e. file) and
 * - the line number within the source code module.
 *
 * The returned module name and file name will be NULL if unknown.
 * The returned line number will similarly be zero if unknown.
 *
 * \param ctx message context (as passed to the @ref libapoi_log_cb callback)
 * \param module module name storage (or NULL) [OUT]
 * \param file source code file name storage (or NULL) [OUT]
 * \param line source code file line number storage (or NULL) [OUT]
 * \warning The returned module name and source code file name, if non-NULL,
 * are only valid until the logging callback returns.
 *
 * \version LibAPOI 2.1.0 or later
 */
LIBAPOI_API void libapoi_log_get_context(const libapoi_log_t *ctx,
                       const char **module, const char **file, unsigned *line);

/**
 * Gets log message info.
 *
 * This function retrieves meta-information about a log message:
 * - the type name of the VLC object emitting the message,
 * - the object header if any, and
 * - a temporaly-unique object identifier.
 *
 * This information is mainly meant for <b>manual</b> troubleshooting.
 *
 * The returned type name may be "generic" if unknown, but it cannot be NULL.
 * The returned header will be NULL if unset; in current versions, the header
 * is used to distinguish for VLM inputs.
 * The returned object ID will be zero if the message is not associated with
 * any VLC object.
 *
 * \param ctx message context (as passed to the @ref libapoi_log_cb callback)
 * \param name object name storage (or NULL) [OUT]
 * \param header object header (or NULL) [OUT]
 * \param id temporarily-unique object identifier (or 0) [OUT]
 * \warning The returned module name and source code file name, if non-NULL,
 * are only valid until the logging callback returns.
 *
 * \version LibAPOI 2.1.0 or later
 */
LIBAPOI_API void libapoi_log_get_object(const libapoi_log_t *ctx,
                        const char **name, const char **header, uintptr_t *id);

/**
 * Callback prototype for LibAPOI log message handler.
 *
 * \param data data pointer as given to libapoi_log_set()
 * \param level message level (@ref libapoi_log_level)
 * \param ctx message context (meta-information about the message)
 * \param fmt printf() format string (as defined by ISO C11)
 * \param args variable argument list for the format
 * \note Log message handlers <b>must</b> be thread-safe.
 * \warning The message context pointer, the format string parameters and the
 *          variable arguments are only valid until the callback returns.
 */
typedef void (*libapoi_log_cb)(void *data, int level, const libapoi_log_t *ctx,
                              const char *fmt, va_list args);

/**
 * Unsets the logging callback.
 *
 * This function deregisters the logging callback for a LibAPOI instance.
 * This is rarely needed as the callback is implicitly unset when the instance
 * is destroyed.
 *
 * \note This function will wait for any pending callbacks invocation to
 * complete (causing a deadlock if called from within the callback).
 *
 * \param p_instance libapoi instance
 * \version LibAPOI 2.1.0 or later
 */
LIBAPOI_API void libapoi_log_unset( libapoi_instance_t *p_instance );

/**
 * Sets the logging callback for a LibAPOI instance.
 *
 * This function is thread-safe: it will wait for any pending callbacks
 * invocation to complete.
 *
 * \param cb callback function pointer
 * \param data opaque data pointer for the callback function
 *
 * \note Some log messages (especially debug) are emitted by LibAPOI while
 * is being initialized. These messages cannot be captured with this interface.
 *
 * \warning A deadlock may occur if this function is called from the callback.
 *
 * \param p_instance libapoi instance
 * \version LibAPOI 2.1.0 or later
 */
LIBAPOI_API void libapoi_log_set( libapoi_instance_t *p_instance,
                                libapoi_log_cb cb, void *data );


/**
 * Sets up logging to a file.
 * \param p_instance libapoi instance
 * \param stream FILE pointer opened for writing
 *         (the FILE pointer must remain valid until libapoi_log_unset())
 * \version LibAPOI 2.1.0 or later
 */
LIBAPOI_API void libapoi_log_set_file( libapoi_instance_t *p_instance, FILE *stream );

/** @} */

/**
 * Description of a module.
 */
typedef struct libapoi_module_description_t
{
    char *psz_name;
    char *psz_shortname;
    char *psz_longname;
    char *psz_help;
    char *psz_help_html;
    struct libapoi_module_description_t *p_next;
} libapoi_module_description_t;

/**
 * Release a list of module descriptions.
 *
 * \param p_list the list to be released
 */
LIBAPOI_API
void libapoi_module_description_list_release( libapoi_module_description_t *p_list );

/**
 * Returns a list of audio filters that are available.
 *
 * \param p_instance libapoi instance
 *
 * \return a list of module descriptions. It should be freed with libapoi_module_description_list_release().
 *         In case of an error, NULL is returned.
 *
 * \see libapoi_module_description_t
 * \see libapoi_module_description_list_release
 */
LIBAPOI_API
libapoi_module_description_t *libapoi_audio_filter_list_get( libapoi_instance_t *p_instance );

/**
 * Returns a list of video filters that are available.
 *
 * \param p_instance libapoi instance
 *
 * \return a list of module descriptions. It should be freed with libapoi_module_description_list_release().
 *         In case of an error, NULL is returned.
 *
 * \see libapoi_module_description_t
 * \see libapoi_module_description_list_release
 */
LIBAPOI_API
libapoi_module_description_t *libapoi_video_filter_list_get( libapoi_instance_t *p_instance );

/** @} */

/** \defgroup libapoi_clock LibAPOI time
 * These functions provide access to the LibAPOI time/clock.
 * @{
 */

/**
 * Return the current time as defined by LibAPOI. The unit is the microsecond.
 * Time increases monotonically (regardless of time zone changes and RTC
 * adjustments).
 * The origin is arbitrary but consistent across the whole system
 * (e.g. the system uptime, the time since the system was booted).
 * \note On systems that support it, the POSIX monotonic clock is used.
 */
LIBAPOI_API
int64_t libapoi_clock(void);

/**
 * Return the delay (in microseconds) until a certain timestamp.
 * \param pts timestamp
 * \return negative if timestamp is in the past,
 * positive if it is in the future
 */
static inline int64_t libapoi_delay(int64_t pts)
{
    return pts - libapoi_clock();
}

/** @} */

# ifdef __cplusplus
}
# endif

#endif /** @} */
