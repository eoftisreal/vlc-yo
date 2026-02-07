/*****************************************************************************
 * media_list_internal.h : Definition of opaque structures for libapoi exported API
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

#ifndef _LIBAPOI_MEDIA_LIST_INTERNAL_H
#define _LIBAPOI_MEDIA_LIST_INTERNAL_H 1

#include <apoi/apoi.h>
#include <apoi/libapoi_media.h>

#include <vlc_common.h>
#include <vlc_arrays.h>

struct libapoi_media_list_t
{
    libapoi_event_manager_t      event_manager;
    vlc_mutex_t                 object_lock;
    vlc_atomic_rc_t             rc;
    libapoi_media_t * p_md; /* The media from which the
                                       * mlist comes, if any. */
    libapoi_media_t * p_internal_md; /* media set from media.c */
    vlc_array_t                items;

    /* This indicates if this media list is read-only
     * from a user point of view */
    bool                  b_read_only;
};

/* Media List */
void libapoi_media_list_internal_add_media(
        libapoi_media_list_t * p_mlist,
        libapoi_media_t * p_md );

void libapoi_media_list_internal_insert_media(
        libapoi_media_list_t * p_mlist,
        libapoi_media_t * p_md, int index );

int libapoi_media_list_internal_remove_index(
        libapoi_media_list_t * p_mlist, int index );

void libapoi_media_list_internal_end_reached(
        libapoi_media_list_t * p_mlist );

#endif
