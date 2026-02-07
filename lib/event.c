/*****************************************************************************
 * event.c: New libapoi event control API
 *****************************************************************************
 * Copyright (C) 2007-2010 VLC authors and VideoLAN
 *
 * Authors: Filippo Carone <filippo@carone.org>
 *          Pierre d'Herbemont <pdherbemont # videolan.org>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <errno.h>

#include <apoi/libapoi.h>
#include "libapoi_internal.h"

#include <vlc_common.h>

/*
 * Event Handling
 */

/* Example usage
 *
 * struct libapoi_cool_object_t
 * {
 *        ...
 *        libapoi_event_manager_t event_manager;
 *        ...
 * }
 *
 * libapoi_my_cool_object_new()
 * {
 *        ...
 *        libapoi_event_manager_init(&p_self->event_manager, p_self)
 *        ...
 * }
 *
 * libapoi_my_cool_object_release()
 * {
 *         ...
 *         libapoi_event_manager_release(&p_self->event_manager);
 *         ...
 * }
 *
 * libapoi_my_cool_object_do_something()
 * {
 *        ...
 *        libapoi_event_t event;
 *        event.type = libapoi_MyCoolObjectDidSomething;
 *        event.u.my_cool_object_did_something.what_it_did = kSomething;
 *        libapoi_event_send(&p_self->event_manager, &event);
 * }
 * */

typedef struct libapoi_event_listener_t
{
    libapoi_event_type_t event_type;
    void *              p_user_data;
    libapoi_callback_t   pf_callback;
} libapoi_event_listener_t;

/*
 * Internal libapoi functions
 */

void libapoi_event_manager_init(libapoi_event_manager_t *em, void *obj)
{
    em->p_obj = obj;
    vlc_array_init(&em->listeners);
    vlc_mutex_init_recursive(&em->lock);
}

void libapoi_event_manager_destroy(libapoi_event_manager_t *em)
{
    for (size_t i = 0; i < vlc_array_count(&em->listeners); i++)
        free(vlc_array_item_at_index(&em->listeners, i));

    vlc_array_clear(&em->listeners);
}

/**************************************************************************
 *       libapoi_event_send (internal) :
 *
 * Send a callback.
 **************************************************************************/
void libapoi_event_send( libapoi_event_manager_t * p_em,
                        libapoi_event_t * p_event )
{
    /* Fill event with the sending object now */
    p_event->p_obj = p_em->p_obj;

    vlc_mutex_lock(&p_em->lock);
    for (size_t i = 0; i < vlc_array_count(&p_em->listeners); i++)
    {
        libapoi_event_listener_t *listener;

        listener = vlc_array_item_at_index(&p_em->listeners, i);
        if (listener->event_type == p_event->type)
            listener->pf_callback(p_event, listener->p_user_data);
    }
    vlc_mutex_unlock(&p_em->lock);
}

/*
 * Public libapoi functions
 */

/**************************************************************************
 *       libapoi_event_attach (public) :
 *
 * Add a callback for an event.
 **************************************************************************/
int libapoi_event_attach(libapoi_event_manager_t *em, libapoi_event_type_t type,
                        libapoi_callback_t callback, void *opaque)
{
    libapoi_event_listener_t *listener = malloc(sizeof (*listener));
    if (unlikely(listener == NULL))
        return ENOMEM;

    listener->event_type = type;
    listener->p_user_data = opaque;
    listener->pf_callback = callback;

    int i_ret;
    vlc_mutex_lock(&em->lock);
    if(vlc_array_append(&em->listeners, listener) != 0)
    {
        i_ret = VLC_EGENERIC;
        free(listener);
    }
    else
        i_ret = VLC_SUCCESS;
    vlc_mutex_unlock(&em->lock);
    return i_ret;
}

/**************************************************************************
 *       libapoi_event_detach (public) :
 *
 * Remove a callback for an event.
 **************************************************************************/
void libapoi_event_detach(libapoi_event_manager_t *em, libapoi_event_type_t type,
                         libapoi_callback_t callback, void *opaque)
{
    vlc_mutex_lock(&em->lock);
    for (size_t i = 0; i < vlc_array_count(&em->listeners); i++)
    {
         libapoi_event_listener_t *listener;

         listener = vlc_array_item_at_index(&em->listeners, i);

         if (listener->event_type == type
          && listener->pf_callback == callback
          && listener->p_user_data == opaque)
         {   /* that's our listener */
             vlc_array_remove(&em->listeners, i);
             vlc_mutex_unlock(&em->lock);
             free(listener);
             return;
         }
    }
    abort();
}
