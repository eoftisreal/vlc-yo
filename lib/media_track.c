/*****************************************************************************
 * media_track.c: Libvlc API media track
 *****************************************************************************
 * Copyright (C) 2020 VLC authors and VideoLAN
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
#include <stdckdint.h>

#include <apoi/libapoi.h>
#include <apoi/libapoi_picture.h>
#include <apoi/libapoi_media.h>
#include <apoi/libapoi_events.h>

#include "libapoi_internal.h"
#include "media_internal.h"

struct libapoi_media_tracklist_t
{
    size_t count;
    libapoi_media_trackpriv_t *tracks[];
};

void
libapoi_media_trackpriv_from_es( libapoi_media_trackpriv_t *trackpriv,
                                const es_format_t *es  )
{
    libapoi_media_track_t *track = &trackpriv->t;

    track->i_codec = es->i_codec;
    track->i_original_fourcc = es->i_original_fourcc;
    track->i_id = es->i_id;

    track->i_profile = es->i_profile;
    track->i_level = es->i_level;

    track->i_bitrate = es->i_bitrate;
    track->psz_language = es->psz_language != NULL ? strdup(es->psz_language) : NULL;
    track->psz_description = es->psz_description != NULL ? strdup(es->psz_description) : NULL;
    track->psz_id = NULL;
    track->id_stable = false;
    track->psz_name = NULL;
    track->selected = false;

    switch( es->i_cat )
    {
    case UNKNOWN_ES:
    default:
        track->i_type = libapoi_track_unknown;
        break;
    case VIDEO_ES:
        track->video = &trackpriv->video;
        track->i_type = libapoi_track_video;
        track->video->i_height = es->video.i_visible_height;
        track->video->i_width = es->video.i_visible_width;
        track->video->i_sar_num = es->video.i_sar_num;
        track->video->i_sar_den = es->video.i_sar_den;
        track->video->i_frame_rate_num = es->video.i_frame_rate;
        track->video->i_frame_rate_den = es->video.i_frame_rate_base;

        assert( es->video.orientation >= ORIENT_TOP_LEFT &&
                es->video.orientation <= ORIENT_RIGHT_BOTTOM );
        track->video->i_orientation = (int) es->video.orientation;

        assert( ( es->video.projection_mode >= PROJECTION_MODE_RECTANGULAR &&
                es->video.projection_mode <= PROJECTION_MODE_EQUIRECTANGULAR ) ||
                ( es->video.projection_mode == PROJECTION_MODE_CUBEMAP_LAYOUT_STANDARD ) );
        track->video->i_projection = (int) es->video.projection_mode;

        vlc_viewpoint_to_euler(&es->video.pose,
                               &track->video->pose.f_yaw,
                               &track->video->pose.f_pitch,
                               &track->video->pose.f_roll);
        track->video->pose.f_field_of_view = es->video.pose.fov;

        assert( es->video.multiview_mode >= MULTIVIEW_2D &&
                es->video.multiview_mode <= MULTIVIEW_STEREO_CHECKERBOARD );
        track->video->i_multiview = (int) es->video.multiview_mode;
        break;
    case AUDIO_ES:
        track->audio = &trackpriv->audio;
        track->i_type = libapoi_track_audio;
        track->audio->i_channels = es->audio.i_channels;
        track->audio->i_rate = es->audio.i_rate;
        break;
    case SPU_ES:
        track->subtitle = &trackpriv->subtitle;
        track->i_type = libapoi_track_text;
        track->subtitle->psz_encoding = es->subs.psz_encoding != NULL ?
                                        strdup(es->subs.psz_encoding) : NULL;
        break;
    }
}

static libapoi_media_trackpriv_t *
libapoi_media_trackpriv_new( void )
{
    libapoi_media_trackpriv_t *trackpriv =
        malloc( sizeof (libapoi_media_trackpriv_t ) );

    if( trackpriv == NULL )
        return NULL;

    trackpriv->es_id = NULL;
    trackpriv->item_str_id = NULL;
    vlc_atomic_rc_init( &trackpriv->rc );
    return trackpriv;
}

static void libapoi_media_track_clean( libapoi_media_track_t *track )
{
    free( track->psz_language );
    free( track->psz_description );
    free( track->psz_name );
    switch( track->i_type )
    {
    case libapoi_track_audio:
        break;
    case libapoi_track_video:
        break;
    case libapoi_track_text:
        free( track->subtitle->psz_encoding );
        break;
    case libapoi_track_unknown:
    default:
        break;
    }
}

libapoi_media_track_t *
libapoi_media_track_hold( libapoi_media_track_t *track )
{
    libapoi_media_trackpriv_t *trackpriv =
        container_of( track, libapoi_media_trackpriv_t, t );
    vlc_atomic_rc_inc( &trackpriv->rc );
    return track;
}

void
libapoi_media_track_release( libapoi_media_track_t *track )
{
    libapoi_media_trackpriv_t *trackpriv =
        container_of( track, libapoi_media_trackpriv_t, t );

    if( vlc_atomic_rc_dec( &trackpriv->rc ) )
    {
        libapoi_media_track_clean( track );
        if( trackpriv->es_id )
            vlc_es_id_Release( trackpriv->es_id );
        free( trackpriv->item_str_id );
        free( trackpriv );
    }
}

static libapoi_media_tracklist_t *
libapoi_media_tracklist_alloc( size_t count )
{
    size_t size;
    if (ckd_mul(&size, count, sizeof (libapoi_media_trackpriv_t *)) ||
        ckd_add(&size, size, sizeof (libapoi_media_tracklist_t)))
        return NULL;

    libapoi_media_tracklist_t *list = malloc( size );
    if( list == NULL )
        return NULL;

    list->count = 0;
    return list;
}

libapoi_media_tracklist_t *
libapoi_media_tracklist_from_item( input_item_t *item, libapoi_track_type_t type )
{
    size_t count = 0;
    const enum es_format_category_e cat = libapoi_track_type_to_escat( type );

    for( size_t i = 0; i < item->es_vec.size; ++i )
    {
        const es_format_t *es_fmt = &item->es_vec.data[i].es;
        if( es_fmt->i_cat == cat )
            count++;
    }

    libapoi_media_tracklist_t *list = libapoi_media_tracklist_alloc( count );

    if( count == 0 || list == NULL )
        return list;

    for( size_t i = 0; i < item->es_vec.size; ++i )
    {
        const struct input_item_es *item_es = &item->es_vec.data[i];
        const es_format_t *es_fmt = &item_es->es;
        if( es_fmt->i_cat == cat )
        {
            libapoi_media_trackpriv_t *trackpriv = libapoi_media_trackpriv_new();
            if( trackpriv == NULL )
            {
                libapoi_media_tracklist_delete( list );
                return NULL;
            }
            list->tracks[list->count++] = trackpriv;
            libapoi_media_trackpriv_from_es( trackpriv, es_fmt );

            trackpriv->item_str_id = strdup( item_es->id );
            if( trackpriv->item_str_id == NULL )
            {
                libapoi_media_tracklist_delete( list );
                return NULL;
            }
            trackpriv->t.psz_id = trackpriv->item_str_id;
            trackpriv->t.id_stable = item_es->id_stable;
        }
    }

    return list;
}

static void
libapoi_media_trackpriv_from_player_track( libapoi_media_trackpriv_t *trackpriv,
                                          const struct vlc_player_track *track )
{
    libapoi_media_trackpriv_from_es( trackpriv, &track->fmt );

    trackpriv->es_id = vlc_es_id_Hold( track->es_id );

    trackpriv->t.psz_id = vlc_es_id_GetStrId( track->es_id );
    trackpriv->t.id_stable = vlc_es_id_IsStrIdStable( track->es_id );
    trackpriv->t.psz_name = strdup( track->name );
    trackpriv->t.selected = track->selected;
}

libapoi_media_track_t *
libapoi_media_track_create_from_player_track( const struct vlc_player_track *track )
{
    libapoi_media_trackpriv_t *trackpriv = libapoi_media_trackpriv_new();
    if( trackpriv == NULL )
        return NULL;
    libapoi_media_trackpriv_from_player_track( trackpriv, track );
    return &trackpriv->t;
}

libapoi_media_tracklist_t *
libapoi_media_tracklist_from_player( vlc_player_t *player,
                                    libapoi_track_type_t type, bool selected )
{
    const enum es_format_category_e cat = libapoi_track_type_to_escat( type );

    size_t count = vlc_player_GetTrackCount( player, cat );

    if( selected )
    {
        size_t selected_count = 0;
        for( size_t i = 0; i < count; ++i )
        {
            const struct vlc_player_track *track =
                vlc_player_GetTrackAt( player, cat, i );
            assert( track );
            if( track->selected )
                selected_count++;
        }
        count = selected_count;
    }

    libapoi_media_tracklist_t *list = libapoi_media_tracklist_alloc( count );

    if( count == 0 || list == NULL )
        return list;

    for( size_t i = 0; i < count; ++i )
    {
        const struct vlc_player_track *track =
            vlc_player_GetTrackAt( player, cat, i );
        assert( track );

        if( selected && !track->selected )
            continue;

        libapoi_media_trackpriv_t *trackpriv = libapoi_media_trackpriv_new();
        if( trackpriv == NULL )
        {
            libapoi_media_tracklist_delete( list );
            return NULL;
        }
        list->tracks[list->count++] = trackpriv;
        libapoi_media_trackpriv_from_player_track( trackpriv, track );
    }

    return list;
}

size_t
libapoi_media_tracklist_count( const libapoi_media_tracklist_t *list )
{
    return list->count;
}

libapoi_media_track_t *
libapoi_media_tracklist_at( libapoi_media_tracklist_t *list, size_t idx )
{
    assert( idx < list->count );
    return &list->tracks[idx]->t;
}

void
libapoi_media_tracklist_delete( libapoi_media_tracklist_t *list )
{
    for( size_t i = 0; i < list->count; ++i )
    {
        libapoi_media_trackpriv_t *trackpriv = list->tracks[i];
        libapoi_media_track_release( &trackpriv->t );
    }
    free( list );
}
