/*****************************************************************************
 * media_list.c: libapoi new API media list functions
 *****************************************************************************
 * Copyright (C) 2007 VLC authors and VideoLAN
 *
 * Authors: Pierre d'Herbemont <pdherbemont # videolan.org>
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

#include <apoi/libapoi.h>
#include <apoi/libapoi_picture.h>
#include <apoi/libapoi_media.h>
#include <apoi/libapoi_media_list.h>
#include <apoi/libapoi_events.h>

#include <vlc_common.h>
#include <vlc_atomic.h>

#include "libapoi_internal.h"
#include "media_internal.h" // libapoi_media_new_from_input_item()
#include "media_list_internal.h"

typedef enum EventPlaceInTime {
    EventWillHappen,
    EventDidHappen
} EventPlaceInTime;

/*
 * Private functions
 */



/**************************************************************************
 *       notify_item_addition (private)
 *
 * Do the appropriate action when an item is deleted.
 **************************************************************************/
static void
notify_item_addition( libapoi_media_list_t * p_mlist,
                      libapoi_media_t * p_md,
                      int index,
                      EventPlaceInTime event_status )
{
    libapoi_event_t event;

    /* Construct the event */
    if( event_status == EventDidHappen )
    {
        event.type = libapoi_MediaListItemAdded;
        event.u.media_list_item_added.item = p_md;
        event.u.media_list_item_added.index = index;
    }
    else /* if( event_status == EventWillHappen ) */
    {
        event.type = libapoi_MediaListWillAddItem;
        event.u.media_list_will_add_item.item = p_md;
        event.u.media_list_will_add_item.index = index;
    }

    /* Send the event */
    libapoi_event_send( &p_mlist->event_manager, &event );
}

/**************************************************************************
 *       notify_item_deletion (private)
 *
 * Do the appropriate action when an item is added.
 **************************************************************************/
static void
notify_item_deletion( libapoi_media_list_t * p_mlist,
                      libapoi_media_t * p_md,
                      int index,
                      EventPlaceInTime event_status )
{
    libapoi_event_t event;

    /* Construct the event */
    if( event_status == EventDidHappen )
    {
        event.type = libapoi_MediaListItemDeleted;
        event.u.media_list_item_deleted.item = p_md;
        event.u.media_list_item_deleted.index = index;
    }
    else /* if( event_status == EventWillHappen ) */
    {
        event.type = libapoi_MediaListWillDeleteItem;
        event.u.media_list_will_delete_item.item = p_md;
        event.u.media_list_will_delete_item.index = index;
    }

    /* Send the event */
    libapoi_event_send( &p_mlist->event_manager, &event );
}

/* LibAPOI internal */
void libapoi_media_list_internal_end_reached( libapoi_media_list_t * p_mlist )
{
    libapoi_event_t event;

    event.type = libapoi_MediaListEndReached;

    /* Send the event */
    libapoi_event_send( &p_mlist->event_manager, &event );
}

/**************************************************************************
 *       static mlist_is_writable (private)
 **************************************************************************/
static inline
bool mlist_is_writable( libapoi_media_list_t *p_mlist )
{
    if( !p_mlist||p_mlist->b_read_only )
    {
        /* We are read-only from user side */
        libapoi_printerr( "Attempt to write a read-only media list" );
        return false;
    }
    return true;
}

/*
 * Public libapoi functions
 */

/**************************************************************************
 *       libapoi_media_list_new (Public)
 *
 * Init an object.
 **************************************************************************/
libapoi_media_list_t *libapoi_media_list_new(void)
{
    libapoi_media_list_t * p_mlist;

    p_mlist = malloc(sizeof(libapoi_media_list_t));
    if( unlikely(p_mlist == NULL) )
    {
        libapoi_printerr( "Not enough memory" );
        return NULL;
    }

    libapoi_event_manager_init( &p_mlist->event_manager, p_mlist );
    p_mlist->b_read_only = false;

    vlc_mutex_init( &p_mlist->object_lock );
    vlc_atomic_rc_init( &p_mlist->rc );

    vlc_array_init( &p_mlist->items );
    assert( p_mlist->items.i_count == 0 );
    p_mlist->p_md = NULL;
    p_mlist->p_internal_md = NULL;

    return p_mlist;
}

/**************************************************************************
 *       libapoi_media_list_release (Public)
 *
 * Release an object.
 **************************************************************************/
void libapoi_media_list_release( libapoi_media_list_t * p_mlist )
{
    if( !vlc_atomic_rc_dec( &p_mlist->rc ) )
        return;

    /* Refcount null, time to free */

    libapoi_event_manager_destroy( &p_mlist->event_manager );
    libapoi_media_release( p_mlist->p_md );

    for( size_t i = 0; i < vlc_array_count( &p_mlist->items ); i++ )
    {
        libapoi_media_t* p_md = vlc_array_item_at_index( &p_mlist->items, i );
        libapoi_media_release( p_md );
    }

    vlc_array_clear( &p_mlist->items );

    free( p_mlist );
}

/**************************************************************************
 *       libapoi_media_list_retain (Public)
 *
 * Increase an object refcount.
 **************************************************************************/
libapoi_media_list_t *libapoi_media_list_retain( libapoi_media_list_t * p_mlist )
{
    vlc_atomic_rc_inc( &p_mlist->rc );
    return p_mlist;
}

/**************************************************************************
 *       set_media (Public)
 **************************************************************************/
void libapoi_media_list_set_media( libapoi_media_list_t * p_mlist,
                                  libapoi_media_t * p_md )

{
    vlc_mutex_lock( &p_mlist->object_lock );
    if( p_mlist->p_internal_md || !mlist_is_writable(p_mlist) )
    {
        vlc_mutex_unlock( &p_mlist->object_lock );
        return;
    }
    libapoi_media_release( p_mlist->p_md );
    libapoi_media_retain( p_md );
    p_mlist->p_md = p_md;
    vlc_mutex_unlock( &p_mlist->object_lock );
}

/**************************************************************************
 *       media (Public)
 *
 * If this media_list comes is a media's subitems,
 * This holds the corresponding media.
 * This md is also seen as the information holder for the media_list.
 * Indeed a media_list can have meta information through this
 * media.
 **************************************************************************/
libapoi_media_t *
libapoi_media_list_media( libapoi_media_list_t * p_mlist )
{
    libapoi_media_t *p_md;

    vlc_mutex_lock( &p_mlist->object_lock );
    p_md = p_mlist->p_internal_md ? p_mlist->p_internal_md : p_mlist->p_md;
    if( p_md )
        libapoi_media_retain( p_md );
    vlc_mutex_unlock( &p_mlist->object_lock );

    return p_md;
}

/**************************************************************************
 *       libapoi_media_list_count (Public)
 *
 * Lock should be held when entering.
 **************************************************************************/
int libapoi_media_list_count( libapoi_media_list_t * p_mlist )
{
    return vlc_array_count( &p_mlist->items );
}

/**************************************************************************
 *       libapoi_media_list_add_media (Public)
 *
 * Lock should be held when entering.
 **************************************************************************/
int libapoi_media_list_add_media( libapoi_media_list_t * p_mlist,
                                 libapoi_media_t * p_md )
{
    if( !mlist_is_writable(p_mlist) )
        return -1;
    libapoi_media_list_internal_add_media( p_mlist, p_md );
    return 0;
}

/* LibAPOI internal version */
void libapoi_media_list_internal_add_media( libapoi_media_list_t * p_mlist,
                                           libapoi_media_t * p_md )
{
    libapoi_media_retain( p_md );

    notify_item_addition( p_mlist, p_md, vlc_array_count( &p_mlist->items ),
                          EventWillHappen );
    vlc_array_append_or_abort( &p_mlist->items, p_md );
    notify_item_addition( p_mlist, p_md, vlc_array_count( &p_mlist->items )-1,
                          EventDidHappen );
}

/**************************************************************************
 *       libapoi_media_list_insert_media (Public)
 *
 * Lock should be hold when entering.
 **************************************************************************/
int libapoi_media_list_insert_media( libapoi_media_list_t * p_mlist,
                                    libapoi_media_t * p_md,
                                    int index )
{
    if( !mlist_is_writable(p_mlist) )
        return -1;
    libapoi_media_list_internal_insert_media( p_mlist, p_md, index );
    return 0;
}

/* LibAPOI internal version */
void libapoi_media_list_internal_insert_media( libapoi_media_list_t * p_mlist,
                                              libapoi_media_t * p_md,
                                              int index )
{
    libapoi_media_retain( p_md );

    notify_item_addition( p_mlist, p_md, index, EventWillHappen );
    vlc_array_insert_or_abort( &p_mlist->items, p_md, index );
    notify_item_addition( p_mlist, p_md, index, EventDidHappen );
}

/**************************************************************************
 *       libapoi_media_list_remove_index (Public)
 *
 * Lock should be held when entering.
 **************************************************************************/
int libapoi_media_list_remove_index( libapoi_media_list_t * p_mlist,
                                     int index )
{
    if( !mlist_is_writable(p_mlist) )
        return -1;
    return libapoi_media_list_internal_remove_index( p_mlist, index );
}

/* LibAPOI internal version */
int libapoi_media_list_internal_remove_index( libapoi_media_list_t * p_mlist,
                                             int index )
{
    libapoi_media_t * p_md;

    if( (size_t) index >= vlc_array_count( &p_mlist->items ))
    {
        libapoi_printerr( "Index out of bounds" );
        return -1;
    }

    p_md = vlc_array_item_at_index( &p_mlist->items, index );

    notify_item_deletion( p_mlist, p_md, index, EventWillHappen );
    vlc_array_remove( &p_mlist->items, index );
    notify_item_deletion( p_mlist, p_md, index, EventDidHappen );

    libapoi_media_release( p_md );
    return 0;
}

/**************************************************************************
 *       libapoi_media_list_item_at_index (Public)
 *
 * Lock should be held when entering.
 **************************************************************************/
libapoi_media_t *
libapoi_media_list_item_at_index( libapoi_media_list_t * p_mlist,
                                 int index )
{
    libapoi_media_t * p_md;

    if( (size_t) index >= vlc_array_count( &p_mlist->items ))
    {
        libapoi_printerr( "Index out of bounds" );
        return NULL;
    }

    p_md = vlc_array_item_at_index( &p_mlist->items, index );
    libapoi_media_retain( p_md );
    return p_md;
}

/**************************************************************************
 *       libapoi_media_list_index_of_item (Public)
 *
 * Lock should be held when entering.
 * Warning: this function returns the first matching item.
 **************************************************************************/
int libapoi_media_list_index_of_item( libapoi_media_list_t * p_mlist,
                                     libapoi_media_t * p_searched_md )
{
    int idx = vlc_array_index_of_item( &p_mlist->items, p_searched_md );
    if( idx == -1 )
        libapoi_printerr( "Media not found" );

    return idx;
}

/**************************************************************************
 *       libapoi_media_list_is_readonly (Public)
 *
 * This indicates if this media list is read-only from a user point of view
 **************************************************************************/
bool libapoi_media_list_is_readonly( libapoi_media_list_t * p_mlist )
{
    return p_mlist->b_read_only;
}

/**************************************************************************
 *       libapoi_media_list_lock (Public)
 *
 * The lock must be held in access operations. It is never used in the
 * Public method.
 **************************************************************************/
void libapoi_media_list_lock( libapoi_media_list_t * p_mlist )
{
    vlc_mutex_lock( &p_mlist->object_lock );
}


/**************************************************************************
 *       libapoi_media_list_unlock (Public)
 *
 * The lock must be held in access operations
 **************************************************************************/
void libapoi_media_list_unlock( libapoi_media_list_t * p_mlist )
{
    vlc_mutex_unlock( &p_mlist->object_lock );
}


/**************************************************************************
 *       libapoi_media_list_event_manager (Public)
 *
 * The p_event_manager is immutable, so you don't have to hold the lock
 **************************************************************************/
libapoi_event_manager_t *
libapoi_media_list_event_manager( libapoi_media_list_t * p_mlist )
{
    return &p_mlist->event_manager;
}
