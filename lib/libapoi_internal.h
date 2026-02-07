/*****************************************************************************
 * libapoi_internal.h : Definition of opaque structures for libapoi exported API
 * Also contains some internal utility functions
 *****************************************************************************
 * Copyright (C) 2005-2009 VLC authors and VideoLAN
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
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

#ifndef _LIBAPOI_INTERNAL_H
#define _LIBAPOI_INTERNAL_H 1

#include <apoi/libapoi.h>
#include <apoi/libapoi_dialog.h>
#include <apoi/libapoi_picture.h>
#include <apoi/libapoi_media.h>
#include <apoi/libapoi_events.h>

#include <vlc_atomic.h>
#include <vlc_common.h>
#include <vlc_arrays.h>
#include <vlc_threads.h>

typedef struct vlc_preparser_t vlc_preparser_t;
typedef struct vlc_preparser_t vlc_preparser_t;

/* Note well: this header is included from LibAPOI core.
 * Therefore, static inline functions MUST NOT call LibAPOI functions here
 * (this can cause linkage failure on some platforms). */

/***************************************************************************
 * Internal creation and destruction functions
 ***************************************************************************/
VLC_API libapoi_int_t *libapoi_InternalCreate( void );
VLC_API int libapoi_InternalInit( libapoi_int_t *, int, const char *ppsz_argv[] );
VLC_API void libapoi_InternalCleanup( libapoi_int_t * );
VLC_API void libapoi_InternalDestroy( libapoi_int_t * );

/**
 * Try to start a user interface for the libapoi instance.
 *
 * \param libapoiint the internal instance
 * \param name interface name, or NULL for default
 * \return 0 on success, -1 on error.
 */
VLC_API int libapoi_InternalAddIntf( libapoi_int_t *libapoiint, const char *name );

/**
 * Start playing the main playlist
 *
 * The main playlist can only be populated via an interface created by the
 * libapoi_InternalAddIntf() function. The control and media flow will only be
 * controlled by the interface previously added.
 *
 * One of these 2 functions (libapoi_InternalAddIntf() or libapoi_InternalPlay())
 * will trigger the creation of an internal playlist and player.
 *
 * \param libapoiint the internal instance
 */
VLC_API void libapoi_InternalPlay( libapoi_int_t *libapoiint );
VLC_API void libapoi_InternalWait( libapoi_int_t * );

/**
 * Registers a callback for the LibAPOI exit event. This is mostly useful if
 * the VLC playlist and/or at least one interface are started with
 * libapoi_InternalPlay() or libapoi_InternalAddIntf () respectively.
 * Typically, this function will wake up your application main loop (from
 * another thread).
 *
 * \note This function should be called before the playlist or interface are
 * started. Otherwise, there is a small race condition: the exit event could
 * be raised before the handler is registered.
 *
 * \param libapoiint the internal instance
 * \param cb callback to invoke when LibAPOI wants to exit,
 *           or NULL to disable the exit handler (as by default)
 * \param opaque data pointer for the callback
 */
VLC_API void libapoi_SetExitHandler( libapoi_int_t *libapoiint, void (*cb) (void *),
                                    void *opaque );

/***************************************************************************
 * Opaque structures for libapoi API
 ***************************************************************************/

struct libapoi_instance_t
{
    libapoi_int_t *p_libapoi_int;
    vlc_atomic_rc_t ref_count;
    struct libapoi_callback_entry_list_t *p_callback_list;

    vlc_mutex_t lazy_init_lock;
    vlc_preparser_t *parser;
    vlc_preparser_t *thumbnailer;

    struct
    {
        void (*cb) (void *, int, const libapoi_log_t *, const char *, va_list);
        void *data;
    } log;
    struct
    {
        libapoi_dialog_cbs cbs;
        void *data;
    } dialog;
};

struct libapoi_event_manager_t
{
    void * p_obj;
    vlc_array_t listeners;
    vlc_mutex_t lock;
};

/***************************************************************************
 * Other internal functions
 ***************************************************************************/

/* Thread context */
void libapoi_threads_init (void);
void libapoi_threads_deinit (void);

/* Events */
void libapoi_event_manager_init(libapoi_event_manager_t *, void *);
void libapoi_event_manager_destroy(libapoi_event_manager_t *);

void libapoi_event_send(
        libapoi_event_manager_t * p_em,
        libapoi_event_t * p_event );

static inline libapoi_time_t libapoi_time_from_vlc_tick(vlc_tick_t time)
{
    return MS_FROM_VLC_TICK(time + VLC_TICK_FROM_US(500));
}

static inline vlc_tick_t vlc_tick_from_libapoi_time(libapoi_time_t time)
{
    return VLC_TICK_FROM_MS(time);
}

vlc_preparser_t *libapoi_get_preparser(libapoi_instance_t *instance);
vlc_preparser_t *libapoi_get_thumbnailer(libapoi_instance_t *instance);

#endif
