/*****************************************************************************
 * libapoi_media_list_player.h:  libapoi_media_list API
 *****************************************************************************
 * Copyright (C) 1998-2008 VLC authors and VideoLAN
 *
 * Authors: Pierre d'Herbemont
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

#ifndef LIBAPOI_MEDIA_LIST_PLAYER_H
#define LIBAPOI_MEDIA_LIST_PLAYER_H 1

#include <apoi/libapoi.h>
#include <apoi/libapoi_media.h>

# ifdef __cplusplus
extern "C" {
# endif

typedef struct libapoi_instance_t libapoi_instance_t;
typedef struct libapoi_media_player_t libapoi_media_player_t;
typedef struct libapoi_media_list_t libapoi_media_list_t;
typedef struct libapoi_media_t libapoi_media_t;

/** \defgroup libapoi_media_list_player LibAPOI media list player
 * \ingroup libapoi
 * The LibAPOI media list player plays a @ref libapoi_media_list_t list of media,
 * in a certain order.
 * This is required to especially support playlist files.
 * The normal @ref libapoi_media_player_t LibAPOI media player can only play a
 * single media, and does not handle playlist files properly.
 * @{
 * \file
 * LibAPOI media list player external API
 */

typedef struct libapoi_media_list_player_t libapoi_media_list_player_t;

/**
 *  Defines playback modes for playlist.
 */
typedef enum libapoi_playback_mode_t
{
    libapoi_playback_mode_default,
    libapoi_playback_mode_loop,
    libapoi_playback_mode_repeat
} libapoi_playback_mode_t;

/**
 * Create new media_list_player.
 *
 * \param p_instance libapoi instance
 * \return media list player instance or NULL on error
 *         (it must be released by libapoi_media_list_player_release())
 */
LIBAPOI_API libapoi_media_list_player_t *
    libapoi_media_list_player_new( libapoi_instance_t * p_instance );

/**
 * Release a media_list_player after use
 * Decrement the reference count of a media player object. If the
 * reference count is 0, then libapoi_media_list_player_release() will
 * release the media player object. If the media player object
 * has been released, then it should not be used again.
 *
 * \param p_mlp media list player instance
 */
LIBAPOI_API void
    libapoi_media_list_player_release( libapoi_media_list_player_t * p_mlp );

/**
 * Retain a reference to a media player list object. Use
 * libapoi_media_list_player_release() to decrement reference count.
 *
 * \param p_mlp media player list object
 * \return the same object
 */
LIBAPOI_API libapoi_media_list_player_t *
    libapoi_media_list_player_retain( libapoi_media_list_player_t *p_mlp );

/**
 * Return the event manager of this media_list_player.
 *
 * \param p_mlp media list player instance
 * \return the event manager
 */
LIBAPOI_API libapoi_event_manager_t *
    libapoi_media_list_player_event_manager(libapoi_media_list_player_t * p_mlp);

/**
 * Replace media player in media_list_player with this instance.
 *
 * \param p_mlp media list player instance
 * \param p_mi media player instance
 */
LIBAPOI_API void
    libapoi_media_list_player_set_media_player(
                                     libapoi_media_list_player_t * p_mlp,
                                     libapoi_media_player_t * p_mi );

/**
 * Get media player of the media_list_player instance.
 *
 * \param p_mlp media list player instance
 * \return media player instance
 * \note the caller is responsible for releasing the returned instance
         with libapoi_media_list_player_set_media_player().
 */
LIBAPOI_API libapoi_media_player_t *
    libapoi_media_list_player_get_media_player(libapoi_media_list_player_t * p_mlp);

/**
 * Set the media list associated with the player
 *
 * \param p_mlp media list player instance
 * \param p_mlist list of media
 */
LIBAPOI_API void
    libapoi_media_list_player_set_media_list(
                                     libapoi_media_list_player_t * p_mlp,
                                     libapoi_media_list_t * p_mlist );

/**
 * Play media list
 *
 * \param p_mlp media list player instance
 */
LIBAPOI_API
void libapoi_media_list_player_play(libapoi_media_list_player_t * p_mlp);

/**
 * Toggle pause (or resume) media list
 *
 * \param p_mlp media list player instance
 */
LIBAPOI_API
void libapoi_media_list_player_pause(libapoi_media_list_player_t * p_mlp);

/**
 * Pause or resume media list
 *
 * \param p_mlp media list player instance
 * \param do_pause play/resume if zero, pause if non-zero
 * \version LibAPOI 3.0.0 or later
 */
LIBAPOI_API
void libapoi_media_list_player_set_pause(libapoi_media_list_player_t * p_mlp,
                                        int do_pause);

/**
 * Is media list playing?
 *
 * \param p_mlp media list player instance
 *
 * \retval true playing
 * \retval false not playing
 */
LIBAPOI_API bool
libapoi_media_list_player_is_playing(libapoi_media_list_player_t * p_mlp);

/**
 * Get current libapoi_state of media list player
 *
 * \param p_mlp media list player instance
 * \return libapoi_state_t for media list player
 */
LIBAPOI_API libapoi_state_t
    libapoi_media_list_player_get_state( libapoi_media_list_player_t * p_mlp );

/**
 * Play media list item at position index
 *
 * \param p_mlp media list player instance
 * \param i_index index in media list to play
 * \return 0 upon success -1 if the item wasn't found
 */
LIBAPOI_API
int libapoi_media_list_player_play_item_at_index(libapoi_media_list_player_t * p_mlp,
                                                int i_index);

/**
 * Play the given media item
 *
 * \param p_mlp media list player instance
 * \param p_md the media instance
 * \return 0 upon success, -1 if the media is not part of the media list
 */
LIBAPOI_API
int libapoi_media_list_player_play_item(libapoi_media_list_player_t * p_mlp,
                                       libapoi_media_t * p_md);

/**
 * Stop playing media list
 *
 * \param p_mlp media list player instance
 */
LIBAPOI_API void
    libapoi_media_list_player_stop_async( libapoi_media_list_player_t * p_mlp);

/**
 * Play next item from media list
 *
 * \param p_mlp media list player instance
 * \return 0 upon success -1 if there is no next item
 */
LIBAPOI_API
int libapoi_media_list_player_next(libapoi_media_list_player_t * p_mlp);

/**
 * Play previous item from media list
 *
 * \param p_mlp media list player instance
 * \return 0 upon success -1 if there is no previous item
 */
LIBAPOI_API
int libapoi_media_list_player_previous(libapoi_media_list_player_t * p_mlp);



/**
 * Sets the playback mode for the playlist
 *
 * \param p_mlp media list player instance
 * \param e_mode playback mode specification
 */
LIBAPOI_API
void libapoi_media_list_player_set_playback_mode(libapoi_media_list_player_t * p_mlp,
                                                libapoi_playback_mode_t e_mode );

/** @} media_list_player */

# ifdef __cplusplus
}
# endif

#endif /* LIBAPOI_MEDIA_LIST_PLAYER_H */
