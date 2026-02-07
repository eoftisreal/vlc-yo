/*****************************************************************************
 * libapoi_events.h:  libapoi_events external API structure
 *****************************************************************************
 * Copyright (C) 1998-2010 VLC authors and VideoLAN
 *
 * Authors: Filippo Carone <littlejohn@videolan.org>
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

#ifndef LIBAPOI_EVENTS_H
#define LIBAPOI_EVENTS_H 1

# include <apoi/libapoi.h>
# include <apoi/libapoi_picture.h>
# include <apoi/libapoi_media_track.h>
# include <apoi/libapoi_media.h>

/**
 * \file
 * This file defines libapoi_event external API
 */

# ifdef __cplusplus
extern "C" {
# else
#  include <stdbool.h>
# endif

typedef struct libapoi_renderer_item_t libapoi_renderer_item_t;
typedef struct libapoi_title_description_t libapoi_title_description_t;
typedef struct libapoi_picture_t libapoi_picture_t;
typedef struct libapoi_picture_list_t libapoi_picture_list_t;
typedef struct libapoi_media_t libapoi_media_t;
typedef struct libapoi_media_list_t libapoi_media_list_t;

/**
 * \ingroup libapoi_event
 * @{
 */

/**
 * Event types
 */
enum libapoi_event_e {
    /* Append new event types at the end of a category.
     * Do not remove, insert or re-order any entry.
     */

    /**
     * 1 or several Metadata of a \link #libapoi_media_t media item\endlink changed
     */
    libapoi_MediaMetaChanged=0,
    /**
     * Subitem was added to a \link #libapoi_media_t media item\endlink
     * \see libapoi_media_subitems()
     */
    libapoi_MediaSubItemAdded,
    /**
     * Deprecated, use libapoi_MediaParsedChanged or libapoi_MediaPlayerLengthChanged.
     */
    libapoi_MediaDurationChanged,
    /**
     * Parsing state of a \link #libapoi_media_t media item\endlink changed
     * \see libapoi_media_parse_request(),
     *      libapoi_media_get_parsed_status(),
     *      libapoi_media_parse_stop()
     */
    libapoi_MediaParsedChanged,

    /* Removed: libapoi_MediaFreed, */
    /* Removed: libapoi_MediaStateChanged */

    /**
     * Subitem tree was added to a \link #libapoi_media_t media item\endlink
     */
    libapoi_MediaSubItemTreeAdded = libapoi_MediaParsedChanged + 3,
    /**
     * A thumbnail generation for this \link #libapoi_media_t media \endlink completed.
     * \see libapoi_media_thumbnail_request_by_time()
     * \see libapoi_media_thumbnail_request_by_pos()
     */
    libapoi_MediaThumbnailGenerated,
    /**
     * One or more embedded thumbnails were found during the media preparsing
     * The user can hold these picture(s) using libapoi_picture_retain if they
     * wish to use them
     */
    libapoi_MediaAttachedThumbnailsFound,

    libapoi_MediaPlayerMediaChanged=0x100,
    libapoi_MediaPlayerNothingSpecial,
    libapoi_MediaPlayerOpening,
    libapoi_MediaPlayerBuffering,
    libapoi_MediaPlayerPlaying,
    libapoi_MediaPlayerPaused,
    libapoi_MediaPlayerStopped,
    libapoi_MediaPlayerForward,
    libapoi_MediaPlayerBackward,
    libapoi_MediaPlayerStopping,
    libapoi_MediaPlayerEncounteredError,
    libapoi_MediaPlayerTimeChanged,
    libapoi_MediaPlayerPositionChanged,
    libapoi_MediaPlayerSeekableChanged,
    libapoi_MediaPlayerPausableChanged,
    /* libapoi_MediaPlayerTitleChanged, */
    libapoi_MediaPlayerSnapshotTaken = libapoi_MediaPlayerPausableChanged + 2,
    libapoi_MediaPlayerLengthChanged,
    libapoi_MediaPlayerVout,

    /* libapoi_MediaPlayerScrambledChanged, use libapoi_MediaPlayerProgramUpdated */

    /** A track was added, cf. media_player_es_changed in \ref libapoi_event_t.u
     * to get the id of the new track. */
    libapoi_MediaPlayerESAdded = libapoi_MediaPlayerVout + 2,
    /** A track was removed, cf. media_player_es_changed in \ref
     * libapoi_event_t.u to get the id of the removed track. */
    libapoi_MediaPlayerESDeleted,
    /** Tracks were selected or unselected, cf.
     * media_player_es_selection_changed in \ref libapoi_event_t.u to get the
     * unselected and/or the selected track ids. */
    libapoi_MediaPlayerESSelected,
    libapoi_MediaPlayerCorked,
    libapoi_MediaPlayerUncorked,
    libapoi_MediaPlayerMuted,
    libapoi_MediaPlayerUnmuted,
    libapoi_MediaPlayerAudioVolume,
    libapoi_MediaPlayerAudioDevice,
    /** A track was updated, cf. media_player_es_changed in \ref
     * libapoi_event_t.u to get the id of the updated track. */
    libapoi_MediaPlayerESUpdated,
    libapoi_MediaPlayerProgramAdded,
    libapoi_MediaPlayerProgramDeleted,
    libapoi_MediaPlayerProgramSelected,
    libapoi_MediaPlayerProgramUpdated,
    /**
     * The title list changed, call
     * libapoi_media_player_get_full_title_descriptions() to get the new list.
     */
    libapoi_MediaPlayerTitleListChanged,
    /**
     * The title selection changed, cf media_player_title_selection_changed in
     * \ref libapoi_event_t.u
     */
    libapoi_MediaPlayerTitleSelectionChanged,
    libapoi_MediaPlayerChapterChanged,
    libapoi_MediaPlayerRecordChanged,

    /**
     * A \link #libapoi_media_t media item\endlink was added to a
     * \link #libapoi_media_list_t media list\endlink.
     */
    libapoi_MediaListItemAdded=0x200,
    /**
     * A \link #libapoi_media_t media item\endlink is about to get
     * added to a \link #libapoi_media_list_t media list\endlink.
     */
    libapoi_MediaListWillAddItem,
    /**
     * A \link #libapoi_media_t media item\endlink was deleted from
     * a \link #libapoi_media_list_t media list\endlink.
     */
    libapoi_MediaListItemDeleted,
    /**
     * A \link #libapoi_media_t media item\endlink is about to get
     * deleted from a \link #libapoi_media_list_t media list\endlink.
     */
    libapoi_MediaListWillDeleteItem,
    /**
     * A \link #libapoi_media_list_t media list\endlink has reached the
     * end.
     * All \link #libapoi_media_t items\endlink were either added (in
     * case of a \ref libapoi_media_discoverer_t) or parsed (preparser).
     */
    libapoi_MediaListEndReached,

    /**
     * \deprecated No longer used.
     * This belonged to the removed libapoi_media_list_view_t
     */
    libapoi_MediaListViewItemAdded LIBAPOI_DEPRECATED =0x300,
    /**
     * \deprecated No longer used.
     * This belonged to the removed libapoi_media_list_view_t
     */
    libapoi_MediaListViewWillAddItem LIBAPOI_DEPRECATED,
    /**
     * \deprecated No longer used.
     * This belonged to the removed libapoi_media_list_view_t
     */
    libapoi_MediaListViewItemDeleted LIBAPOI_DEPRECATED,
    /**
     * \deprecated No longer used.
     * This belonged to the removed libapoi_media_list_view_t
     */
    libapoi_MediaListViewWillDeleteItem LIBAPOI_DEPRECATED,

    /**
     * Playback of a \link #libapoi_media_list_player_t media list
     * player\endlink has started.
     */
    libapoi_MediaListPlayerPlayed=0x400,

    /**
     * The current \link #libapoi_media_t item\endlink of a
     * \link #libapoi_media_list_player_t media list player\endlink
     * has changed to a different item.
     */
    libapoi_MediaListPlayerNextItemSet,

    /**
     * Playback of a \link #libapoi_media_list_player_t media list
     * player\endlink has stopped.
     */
    libapoi_MediaListPlayerStopped,

    /**
     * A new \link #libapoi_renderer_item_t renderer item\endlink was found by a
     * \link #libapoi_renderer_discoverer_t renderer discoverer\endlink.
     * The renderer item is valid until deleted.
     */
    libapoi_RendererDiscovererItemAdded=0x502,

    /**
     * A previously discovered \link #libapoi_renderer_item_t renderer item\endlink
     * was deleted by a \link #libapoi_renderer_discoverer_t renderer discoverer\endlink.
     * The renderer item is no longer valid.
     */
    libapoi_RendererDiscovererItemDeleted,

    /**
     * The current media set into the \ref libapoi_media_player_t is stopping.
     *
     * This event can be used to notify when the media callbacks, initialized
     * from \ref libapoi_media_new_callbacks, should be interrupted, and in
     * particular the \ref libapoi_media_read_cb. It can also be used to signal
     * the application state that any input resource (webserver, file mounting,
     * etc) can be discarded. Output resources still need to be active until
     * the player switches to the \ref libapoi_Stopped state.
     */
    libapoi_MediaPlayerMediaStopping,
};

/**
 * A LibAPOI event
 */
typedef struct libapoi_event_t
{
    int   type; /**< Event type (see @ref libapoi_event_e) */
    void *p_obj; /**< Object emitting the event */
    union
    {
        /* media descriptor */
        struct
        {
            libapoi_meta_t meta_type; /**< Deprecated, any meta_type can change */
        } media_meta_changed;
        struct
        {
            libapoi_media_t * new_child;
        } media_subitem_added;
        struct
        {
            int64_t new_duration;
        } media_duration_changed;
        struct
        {
            int new_status; /**< see @ref libapoi_media_parsed_status_t */
        } media_parsed_changed;
        struct
        {
            int new_state; /**< see @ref libapoi_state_t */
        } media_state_changed;
        struct
        {
            libapoi_picture_t* p_thumbnail;
        } media_thumbnail_generated;
        struct
        {
            libapoi_media_t * item;
        } media_subitemtree_added;
        struct
        {
            libapoi_picture_list_t* thumbnails;
        } media_attached_thumbnails_found;

        /* media instance */
        struct
        {
            float new_cache;
        } media_player_buffering;
        struct
        {
            int new_chapter;
        } media_player_chapter_changed;
        struct
        {
            double new_position;
        } media_player_position_changed;
        struct
        {
            libapoi_time_t new_time;
        } media_player_time_changed;
        struct
        {
            const libapoi_title_description_t *title;
            int index;
        } media_player_title_selection_changed;
        struct
        {
            int new_seekable;
        } media_player_seekable_changed;
        struct
        {
            int new_pausable;
        } media_player_pausable_changed;
        struct
        {
            int new_scrambled;
        } media_player_scrambled_changed;
        struct
        {
            int new_count;
        } media_player_vout;

        /* media list */
        struct
        {
            libapoi_media_t * item;
            int index;
        } media_list_item_added;
        struct
        {
            libapoi_media_t * item;
            int index;
        } media_list_will_add_item;
        struct
        {
            libapoi_media_t * item;
            int index;
        } media_list_item_deleted;
        struct
        {
            libapoi_media_t * item;
            int index;
        } media_list_will_delete_item;

        /* media list player */
        struct
        {
            libapoi_media_t * item;
        } media_list_player_next_item_set;

        /* snapshot taken */
        struct
        {
             char* psz_filename ;
        } media_player_snapshot_taken ;

        /* Length changed */
        struct
        {
            libapoi_time_t   new_length;
        } media_player_length_changed;

        /* Extra MediaPlayer */
        struct
        {
            libapoi_media_t * new_media;
        } media_player_media_changed;

        struct
        {
            libapoi_media_t * media;
        } media_player_media_stopping;


        /* ESAdded, ESDeleted, ESUpdated */
        struct
        {
            libapoi_track_type_t i_type;
            int i_id; /**< Deprecated, use psz_id */
            /** Call libapoi_media_player_get_track_from_id() to get the track
             * description. */
            const char *psz_id;
        } media_player_es_changed;

        /* ESSelected */
        struct
        {
            libapoi_track_type_t i_type;
            const char *psz_unselected_id;
            const char *psz_selected_id;
        } media_player_es_selection_changed;

        /* ProgramAdded, ProgramDeleted, ProgramUpdated */
        struct
        {
            int i_id;
        } media_player_program_changed;

        /* ProgramSelected */
        struct
        {
            int i_unselected_id;
            int i_selected_id;
        } media_player_program_selection_changed;

        struct
        {
            float volume;
        } media_player_audio_volume;

        struct
        {
            const char *device;
        } media_player_audio_device;

        struct
        {
            bool recording;
            /** Only valid when recording ends (recording == false) */
            const char *recorded_file_path;
        } media_player_record_changed;

        struct
        {
            libapoi_renderer_item_t *item;
        } renderer_discoverer_item_added;
        struct
        {
            libapoi_renderer_item_t *item;
        } renderer_discoverer_item_deleted;
    } u; /**< Type-dependent event description */
} libapoi_event_t;


/**@} */

# ifdef __cplusplus
}
# endif

#endif /* _LIBAPOI_EVENTS_H */
