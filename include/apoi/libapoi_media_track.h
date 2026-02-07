/*****************************************************************************
 * libapoi_media_track.h:  libapoi external API
 *****************************************************************************
 * Copyright (C) 1998-2020 VLC authors and VideoLAN
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

#ifndef VLC_LIBAPOI_MEDIA_TRACK_H
#define VLC_LIBAPOI_MEDIA_TRACK_H 1

# include "libapoi_video.h"

# ifdef __cplusplus
extern "C" {
# else
#  include <stdbool.h>
# endif

/** \defgroup libapoi_media_track LibAPOI media track
 * \ingroup libapoi
 * @ref libapoi_media_track_t is an abstract representation of a media track.
 * @{
 * \file
 * LibAPOI media track
 */

typedef enum libapoi_track_type_t
{
    libapoi_track_unknown   = -1,
    libapoi_track_audio     = 0,
    libapoi_track_video     = 1,
    libapoi_track_text      = 2
} libapoi_track_type_t;

typedef struct libapoi_audio_track_t
{
    unsigned    i_channels;
    unsigned    i_rate;
} libapoi_audio_track_t;

/**
 * Viewpoint
 *
 * \warning allocate using libapoi_video_new_viewpoint()
 */
typedef struct libapoi_video_viewpoint_t
{
    float f_yaw;           /**< view point yaw in degrees  ]-180;180] */
    float f_pitch;         /**< view point pitch in degrees  ]-90;90] */
    float f_roll;          /**< view point roll in degrees ]-180;180] */
    float f_field_of_view; /**< field of view in degrees ]0;180[ (default 80.)*/
} libapoi_video_viewpoint_t;

typedef struct libapoi_video_track_t
{
    unsigned    i_height;
    unsigned    i_width;
    unsigned    i_sar_num;
    unsigned    i_sar_den;
    unsigned    i_frame_rate_num;
    unsigned    i_frame_rate_den;

    libapoi_video_orient_t       i_orientation;
    libapoi_video_projection_t   i_projection;
    libapoi_video_viewpoint_t    pose; /**< Initial view point */
    libapoi_video_multiview_t    i_multiview;
} libapoi_video_track_t;

typedef struct libapoi_subtitle_track_t
{
    char *psz_encoding;
} libapoi_subtitle_track_t;

typedef struct libapoi_media_track_t
{
    /* Codec fourcc */
    uint32_t    i_codec;
    uint32_t    i_original_fourcc;
    int         i_id; /* DEPRECATED: use psz_id */
    libapoi_track_type_t i_type;

    /* Codec specific */
    int         i_profile;
    int         i_level;

    union {
        libapoi_audio_track_t *audio;
        libapoi_video_track_t *video;
        libapoi_subtitle_track_t *subtitle;
    };

    unsigned int i_bitrate;
    char *psz_language;
    char *psz_description;

    /** String identifier of track, can be used to save the track preference
     * from an other LibAPOI run */
    const char *psz_id;
    /** A string identifier is stable when it is certified to be the same
     * across different playback instances for the same track. */
    bool id_stable;
    /** Name of the track, only valid when the track is fetch from a
     * media_player */
    char *psz_name;
    /** true if the track is selected, only valid when the track is fetch from
     * a media_player */
    bool selected;

} libapoi_media_track_t;

/**
 * Opaque struct containing a list of tracks
 */
typedef struct libapoi_media_tracklist_t libapoi_media_tracklist_t;

/**
 * Get the number of tracks in a tracklist
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \param list valid tracklist
 *
 * \return number of tracks, or 0 if the list is empty
 */
LIBAPOI_API size_t
libapoi_media_tracklist_count( const libapoi_media_tracklist_t *list );

/**
 * Get a track at a specific index
 *
 * \warning The behaviour is undefined if the index is not valid.
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \param list valid tracklist
 * \param index valid index in the range [0; count[
 *
 * \return a valid track (can't be NULL if libapoi_media_tracklist_count()
 * returned a valid count)
 */
LIBAPOI_API libapoi_media_track_t *
libapoi_media_tracklist_at( libapoi_media_tracklist_t *list, size_t index );

/**
 * Release a tracklist
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \see libapoi_media_get_tracklist
 * \see libapoi_media_player_get_tracklist
 *
 * \param list valid tracklist
 */
LIBAPOI_API void
libapoi_media_tracklist_delete( libapoi_media_tracklist_t *list );


/**
 * Hold a single track reference
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * This function can be used to hold a track from a tracklist. In that case,
 * the track can outlive its tracklist.
 *
 * \param track valid track
 * \return the same track, need to be released with libapoi_media_track_release()
 */
LIBAPOI_API libapoi_media_track_t *
libapoi_media_track_hold( libapoi_media_track_t *track );

/**
 * Release a single track
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \warning Tracks from a tracklist are released alongside the list with
 * libapoi_media_tracklist_delete().
 *
 * \note You only need to release tracks previously held with
 * libapoi_media_track_hold() or returned by
 * libapoi_media_player_get_selected_track() and
 * libapoi_media_player_get_track_from_id()
 *
 * \param track valid track
 */
LIBAPOI_API void
libapoi_media_track_release( libapoi_media_track_t *track );
/** @}*/

# ifdef __cplusplus
}
# endif

#endif /* VLC_LIBAPOI_MEDIA_TRACK_H */
