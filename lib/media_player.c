// SPDX-License-Identifier: LGPL-2.1-or-later
/*****************************************************************************
 * media_player.c: Libvlc API Media Instance management functions
 *****************************************************************************
 * Copyright (C) 2005-2015 VLC authors and VideoLAN
 * Copyright (C) 2020-2026 Alexandre Janniaux <ajanni@videolabs.io>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdckdint.h>

#include <apoi/libapoi.h>
#include <apoi/libapoi_renderer_discoverer.h>
#include <apoi/libapoi_picture.h>
#include <apoi/libapoi_media.h>
#include <apoi/libapoi_events.h>

#include <vlc_demux.h>
#include <vlc_vout.h>
#include <vlc_aout.h>
#include <vlc_subpicture.h>
#include <vlc_actions.h>
#include <vlc_modules.h>

#include "libapoi_internal.h"
#include "media_player_internal.h"
#include "renderer_discoverer_internal.h"

#define ES_INIT (-2) /* -1 is reserved for ES deselect */

static int
snapshot_was_taken( vlc_object_t *p_this, char const *psz_cmd,
                    vlc_value_t oldval, vlc_value_t newval, void *p_data );

static void libapoi_media_player_destroy( libapoi_media_player_t *p_mi );

// player callbacks

static void
on_current_media_changed(vlc_player_t *player, input_item_t *new_media,
                         void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    libapoi_media_t *libmedia;
    if (new_media != NULL)
    {
        libmedia = new_media->libapoi_owner;
        assert(libmedia != NULL);
    }
    else
        libmedia = NULL;

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerMediaChanged;
    event.u.media_player_media_changed.new_media = libmedia;
    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_stopping_current_media(vlc_player_t *player, input_item_t *media,
                          enum vlc_player_media_stopping_reason stopping_reason,
                          void *data)
{
    assert(media != NULL);
    (void) player;
    (void) stopping_reason;

    libapoi_media_player_t *mp = data;

    libapoi_media_t *libmedia = media->libapoi_owner;
    assert(libmedia != NULL);

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerMediaStopping;
    event.u.media_player_media_stopping.media = libmedia;
    libapoi_event_send(&mp->event_manager, &event);
}


static libapoi_event_type_t
PlayerStateToLibvlcEventType(enum vlc_player_state new_state)
{
    switch (new_state) {
        case VLC_PLAYER_STATE_STOPPED:
            return libapoi_MediaPlayerStopped;
        case VLC_PLAYER_STATE_STOPPING:
            return libapoi_MediaPlayerStopping;
        case VLC_PLAYER_STATE_STARTED:
            return libapoi_MediaPlayerOpening;
        case VLC_PLAYER_STATE_PLAYING:
            return libapoi_MediaPlayerPlaying;
        case VLC_PLAYER_STATE_PAUSED:
            return libapoi_MediaPlayerPaused;
    }
    vlc_assert_unreachable();
}

static void
on_state_changed(vlc_player_t *player, enum vlc_player_state new_state,
                 void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event = {
        .type = PlayerStateToLibvlcEventType(new_state)
    };
    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_recording_changed(vlc_player_t *player, bool recording, void *data)
{
    (void) player;
    libapoi_media_player_t *mp = data;

    /* "record-file" is only valid when recording ends */
    char *file_path = !recording ? var_GetString(mp, "record-file") : NULL;

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerRecordChanged;
    event.u.media_player_record_changed.recorded_file_path = file_path;
    event.u.media_player_record_changed.recording = recording;

    libapoi_event_send(&mp->event_manager, &event);

    free(file_path);
}

static void
on_error_changed(vlc_player_t *player, enum vlc_player_error error, void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    switch (error) {
        case VLC_PLAYER_ERROR_NONE:
            event.type = libapoi_MediaPlayerNothingSpecial;
            break;
        case VLC_PLAYER_ERROR_GENERIC:
            event.type = libapoi_MediaPlayerEncounteredError;
            break;
        default:
            vlc_assert_unreachable();
    }

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_buffering_changed(vlc_player_t *player, float new_buffering, void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerBuffering;
    event.u.media_player_buffering.new_cache = 100 * new_buffering;

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_capabilities_changed(vlc_player_t *player, int old_caps, int new_caps, void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;

    bool old_seekable = old_caps & VLC_PLAYER_CAP_SEEK;
    bool new_seekable = new_caps & VLC_PLAYER_CAP_SEEK;
    if (new_seekable != old_seekable)
    {
        event.type = libapoi_MediaPlayerSeekableChanged;
        event.u.media_player_seekable_changed.new_seekable = new_seekable;
        libapoi_event_send(&mp->event_manager, &event);
    }

    bool old_pauseable = old_caps & VLC_PLAYER_CAP_PAUSE;
    bool new_pauseable = new_caps & VLC_PLAYER_CAP_PAUSE;
    if (new_pauseable != old_pauseable)
    {
        event.type = libapoi_MediaPlayerPausableChanged;
        event.u.media_player_pausable_changed.new_pausable = new_pauseable;
        libapoi_event_send(&mp->event_manager, &event);
    }
}

static void
on_position_changed(vlc_player_t *player, vlc_tick_t new_time, double new_pos,
                    void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;

    event.type = libapoi_MediaPlayerPositionChanged;
    event.u.media_player_position_changed.new_position = new_pos;
    libapoi_event_send(&mp->event_manager, &event);

    event.type = libapoi_MediaPlayerTimeChanged;
    event.u.media_player_time_changed.new_time = MS_FROM_VLC_TICK(new_time);
    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_length_changed(vlc_player_t *player, vlc_tick_t new_length, void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;

    libapoi_media_t *md = mp->p_md;
    if (md != NULL)
    {
        /* Duration event */
        event.type = libapoi_MediaDurationChanged;
        event.u.media_duration_changed.new_duration =
            libapoi_time_from_vlc_tick(input_item_GetDuration( md->p_input_item ));
        libapoi_event_send( &md->event_manager, &event );
    }

    event.type = libapoi_MediaPlayerLengthChanged;
    event.u.media_player_length_changed.new_length =
        MS_FROM_VLC_TICK(new_length);

    libapoi_event_send(&mp->event_manager, &event);
}

static int
track_type_from_cat(enum es_format_category_e cat)
{
    switch (cat)
    {
        case VIDEO_ES:
            return libapoi_track_video;
        case AUDIO_ES:
            return libapoi_track_audio;
        case SPU_ES:
            return libapoi_track_text;
        default:
            return libapoi_track_unknown;
    }
}

static void
on_track_list_changed(vlc_player_t *player, enum vlc_player_list_action action,
                      const struct vlc_player_track *track, void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    switch (action)
    {
        case VLC_PLAYER_LIST_ADDED:
            event.type = libapoi_MediaPlayerESAdded; break;
        case VLC_PLAYER_LIST_REMOVED:
            event.type = libapoi_MediaPlayerESDeleted; break;
        case VLC_PLAYER_LIST_UPDATED:
            event.type = libapoi_MediaPlayerESUpdated; break;
    }

    event.u.media_player_es_changed.i_type =
        track_type_from_cat(track->fmt.i_cat);
    event.u.media_player_es_changed.i_id = vlc_es_id_GetInputId(track->es_id);
    event.u.media_player_es_changed.psz_id = vlc_es_id_GetStrId(track->es_id);

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_track_selection_changed(vlc_player_t *player, vlc_es_id_t *unselected_id,
                           vlc_es_id_t *selected_id, void *data)
{
    (void) player;
    (void) unselected_id;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerESSelected;

    if (unselected_id)
    {
        enum es_format_category_e cat = vlc_es_id_GetCat(unselected_id);
        event.u.media_player_es_selection_changed.i_type = track_type_from_cat(cat);
    }
    if (selected_id)
    {
        enum es_format_category_e cat = vlc_es_id_GetCat(selected_id);
        event.u.media_player_es_selection_changed.i_type = track_type_from_cat(cat);
    }

    event.u.media_player_es_selection_changed.psz_unselected_id =
        unselected_id ? vlc_es_id_GetStrId(unselected_id) : NULL;

    event.u.media_player_es_selection_changed.psz_selected_id =
        selected_id ? vlc_es_id_GetStrId(selected_id) : NULL;

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_program_list_changed(vlc_player_t *player,
                        enum vlc_player_list_action action,
                        const struct vlc_player_program *prgm, void* data)
{
    (void) player;
    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    switch (action)
    {
        case VLC_PLAYER_LIST_ADDED:
            event.type = libapoi_MediaPlayerProgramAdded;
            break;
        case VLC_PLAYER_LIST_REMOVED:
            event.type = libapoi_MediaPlayerProgramDeleted;
            break;
        case VLC_PLAYER_LIST_UPDATED:
            event.type = libapoi_MediaPlayerProgramUpdated;
            break;
    }

    event.u.media_player_program_changed.i_id = prgm->group_id;
    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_program_selection_changed(vlc_player_t *player, int unselected_id,
                             int selected_id, void *data)
{
    (void) player;
    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerProgramSelected;
    event.u.media_player_program_selection_changed.i_unselected_id = unselected_id;
    event.u.media_player_program_selection_changed.i_selected_id = selected_id;

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_titles_changed(vlc_player_t *player,
                  vlc_player_title_list *titles, void *data)
{
    (void) player;
    (void) titles;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerTitleListChanged;

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_title_selection_changed(vlc_player_t *player,
                           const struct vlc_player_title *new_title,
                           size_t new_idx, void *data)
{
    (void) player;
    (void) new_title;

    libapoi_media_player_t *mp = data;

    const libapoi_title_description_t libtitle = {
        .i_duration = MS_FROM_VLC_TICK(new_title->length),
        .psz_name = (char *) new_title->name,
        .i_flags = new_title->flags,
    };

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerTitleSelectionChanged;
    event.u.media_player_title_selection_changed.title = &libtitle;
    event.u.media_player_title_selection_changed.index = new_idx;

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_chapter_selection_changed(vlc_player_t *player,
                             const struct vlc_player_title *title,
                             size_t title_idx,
                             const struct vlc_player_chapter *new_chapter,
                             size_t new_chapter_idx,
                             void *data)
{
    (void) player;
    (void) title;
    (void) title_idx;
    (void) new_chapter;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerChapterChanged;
    event.u.media_player_chapter_changed.new_chapter = new_chapter_idx;

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_media_meta_changed(vlc_player_t *player, input_item_t *media, void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;
    input_item_t *current = mp->p_md ? mp->p_md->p_input_item : NULL;
    if (media != current)
        return;

    /* Meta event */
    libapoi_event_t event;
    event.type = libapoi_MediaMetaChanged;
    event.u.media_meta_changed.meta_type = 0;
    libapoi_event_send( &mp->p_md->event_manager, &event );

    libapoi_media_parsed_status_t status = libapoi_media_parsed_status_done;
    if (atomic_exchange(&mp->p_md->parsed_status, status) == status)
        return;

    /* Parsed event */
    event.type = libapoi_MediaParsedChanged;
    event.u.media_parsed_changed.new_status = status;
    libapoi_event_send( &mp->p_md->event_manager, &event );
}


static void
on_media_subitems_changed(vlc_player_t *player, input_item_t *media,
                          const input_item_node_t *new_subitems, void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    input_item_t *current = mp->p_md ? mp->p_md->p_input_item : NULL;
    if (media == current)
        libapoi_media_add_subtree(mp->p_md, new_subitems);
}

static void
on_cork_changed(vlc_player_t *player, unsigned cork_count, void *data)
{
    (void) player;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    event.type = cork_count ? libapoi_MediaPlayerCorked
                            : libapoi_MediaPlayerUncorked;

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_vout_changed(vlc_player_t *player, enum vlc_player_vout_action action,
                vout_thread_t *vout, enum vlc_vout_order order,
                vlc_es_id_t *es_id, void *data)
{
    (void) action;
    (void) vout;
    (void) order;

    if (vlc_es_id_GetCat(es_id) != VIDEO_ES)
        return;

    libapoi_media_player_t *mp = data;

    size_t count;
    vout_thread_t **vouts = vlc_player_vout_HoldAll(player, &count);
    if (!vouts)
        return;
    for (size_t i = 0; i < count; ++i)
        vout_Release(vouts[i]);
    free(vouts);

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerVout;
    event.u.media_player_vout.new_count = count;

    libapoi_event_send(&mp->event_manager, &event);
}

// player aout callbacks

static void
on_volume_changed(audio_output_t *aout, float new_volume, void *data)
{
    (void) aout;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerAudioVolume;
    event.u.media_player_audio_volume.volume = new_volume;

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_mute_changed(audio_output_t *aout, bool new_muted, void *data)
{
    (void) aout;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    event.type = new_muted ? libapoi_MediaPlayerMuted
                           : libapoi_MediaPlayerUnmuted;

    libapoi_event_send(&mp->event_manager, &event);
}

static void
on_audio_device_changed(audio_output_t *aout, const char *device, void *data)
{
    (void) aout;

    libapoi_media_player_t *mp = data;

    libapoi_event_t event;
    event.type = libapoi_MediaPlayerAudioDevice;
    event.u.media_player_audio_device.device = device;

    libapoi_event_send(&mp->event_manager, &event);
}

static const struct vlc_player_cbs vlc_player_cbs = {
    .on_current_media_changed = on_current_media_changed,
    .on_stopping_current_media = on_stopping_current_media,
    .on_state_changed = on_state_changed,
    .on_error_changed = on_error_changed,
    .on_buffering_changed = on_buffering_changed,
    .on_capabilities_changed = on_capabilities_changed,
    .on_position_changed = on_position_changed,
    .on_length_changed = on_length_changed,
    .on_track_list_changed = on_track_list_changed,
    .on_track_selection_changed = on_track_selection_changed,
    .on_program_list_changed = on_program_list_changed,
    .on_program_selection_changed = on_program_selection_changed,
    .on_titles_changed = on_titles_changed,
    .on_title_selection_changed = on_title_selection_changed,
    .on_chapter_selection_changed = on_chapter_selection_changed,
    .on_media_meta_changed = on_media_meta_changed,
    .on_media_subitems_changed = on_media_subitems_changed,
    .on_cork_changed = on_cork_changed,
    .on_vout_changed = on_vout_changed,
    .on_recording_changed = on_recording_changed,
};

static const struct vlc_player_aout_cbs vlc_player_aout_cbs = {
    .on_volume_changed = on_volume_changed,
    .on_mute_changed = on_mute_changed,
    .on_device_changed = on_audio_device_changed,
};

/**************************************************************************
 * Snapshot Taken Event.
 *
 * FIXME: This snapshot API interface makes no sense in media_player.
 *************************************************************************/
static int snapshot_was_taken(vlc_object_t *p_this, char const *psz_cmd,
                              vlc_value_t oldval, vlc_value_t newval, void *p_data )
{
    VLC_UNUSED(psz_cmd); VLC_UNUSED(oldval); VLC_UNUSED(p_this);

    libapoi_media_player_t *mp = p_data;
    libapoi_event_t event;
    event.type = libapoi_MediaPlayerSnapshotTaken;
    event.u.media_player_snapshot_taken.psz_filename = newval.psz_string;
    libapoi_event_send(&mp->event_manager, &event);

    return VLC_SUCCESS;
}

/**************************************************************************
 * Create a Media Instance object.
 *
 * Refcount strategy:
 * - All items created by _new start with a refcount set to 1.
 * - Accessor _release decrease the refcount by 1, if after that
 *   operation the refcount is 0, the object is destroyed.
 * - Accessor _retain increase the refcount by 1 (XXX: to implement)
 *
 * Object locking strategy:
 * - No lock held while in constructor.
 * - When accessing any member variable this lock is held. (XXX who locks?)
 * - When attempting to destroy the object the lock is also held.
 **************************************************************************/
libapoi_media_player_t *
libapoi_media_player_new( libapoi_instance_t *instance )
{
    libapoi_media_player_t * mp;
    int doinherit;

    assert(instance);

    mp = vlc_object_create (instance->p_libapoi_int, sizeof(*mp));
    if (unlikely(mp == NULL))
    {
        libapoi_printerr("Not enough memory");
        return NULL;
    }

    /* Input */
    var_Create (mp, "rate", VLC_VAR_FLOAT|VLC_VAR_DOINHERIT);
    var_Create (mp, "sout", VLC_VAR_STRING);
    var_Create (mp, "demux-filter", VLC_VAR_STRING);

    /* Video */
    var_Create (mp, "vout", VLC_VAR_STRING|VLC_VAR_DOINHERIT);
    var_Create (mp, "window", VLC_VAR_STRING);
    var_Create (mp, "gl", VLC_VAR_STRING);
    var_Create (mp, "gles2", VLC_VAR_STRING);
    var_Create (mp, "vmem-lock", VLC_VAR_ADDRESS);
    var_Create (mp, "vmem-unlock", VLC_VAR_ADDRESS);
    var_Create (mp, "vmem-display", VLC_VAR_ADDRESS);
    var_Create (mp, "vmem-data", VLC_VAR_ADDRESS);
    var_Create (mp, "vmem-setup", VLC_VAR_ADDRESS);
    var_Create (mp, "vmem-cleanup", VLC_VAR_ADDRESS);
    var_Create (mp, "vmem-chroma", VLC_VAR_STRING);
    var_Create (mp, "vmem-width", VLC_VAR_INTEGER);
    var_Create (mp, "vmem-height", VLC_VAR_INTEGER);
    var_Create (mp, "vmem-pitch", VLC_VAR_INTEGER);

    var_Create (mp, "vout-cb-type", VLC_VAR_INTEGER );
    var_Create( mp, "vout-cb-opaque", VLC_VAR_ADDRESS );
    var_Create( mp, "vout-cb-setup", VLC_VAR_ADDRESS );
    var_Create( mp, "vout-cb-cleanup", VLC_VAR_ADDRESS );
    var_Create( mp, "vout-cb-window-cb", VLC_VAR_ADDRESS );
    var_Create( mp, "vout-cb-update-output", VLC_VAR_ADDRESS );
    var_Create( mp, "vout-cb-swap", VLC_VAR_ADDRESS );
    var_Create( mp, "vout-cb-get-proc-address", VLC_VAR_ADDRESS );
    var_Create( mp, "vout-cb-make-current", VLC_VAR_ADDRESS );
    var_Create( mp, "vout-cb-metadata", VLC_VAR_ADDRESS );
    var_Create( mp, "vout-cb-select-plane", VLC_VAR_ADDRESS );

    var_Create (mp, "dec-dev", VLC_VAR_STRING);

    mp->vout.default_gl = var_GetString(mp, "gl");
    mp->vout.default_gles2 = var_GetString(mp, "gles2");
    mp->vout.default_vout = var_GetString(mp, "vout");
    mp->vout.default_dec_dev = var_GetString(mp, "dec-dev");

    var_Create (mp, "drawable-xid", VLC_VAR_INTEGER);
#if defined (_WIN32) || defined (__OS2__)
    var_Create (mp, "drawable-hwnd", VLC_VAR_INTEGER);
#endif
#ifdef __APPLE__
    var_Create (mp, "drawable-nsobject", VLC_VAR_ADDRESS);
#endif
#ifdef __ANDROID__
    var_Create (mp, "drawable-androidwindow", VLC_VAR_ADDRESS);
#endif

    var_Create (mp, "keyboard-events", VLC_VAR_BOOL);
    var_SetBool (mp, "keyboard-events", true);
    var_Create (mp, "mouse-events", VLC_VAR_BOOL);
    var_SetBool (mp, "mouse-events", true);

    var_Create (mp, "fullscreen", VLC_VAR_BOOL);
    var_Create (mp, "autoscale", VLC_VAR_BOOL | VLC_VAR_DOINHERIT);
    var_Create (mp, "fit", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT);
    var_Create (mp, "zoom", VLC_VAR_FLOAT | VLC_VAR_DOINHERIT);
    var_Create (mp, "aspect-ratio", VLC_VAR_STRING);
    var_Create (mp, "crop", VLC_VAR_STRING);
    var_Create (mp, "deinterlace", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT);
    var_Create (mp, "deinterlace-mode", VLC_VAR_STRING | VLC_VAR_DOINHERIT);
    var_Create (mp, "projection-mode", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT);
    var_Create (mp, "video-stereo-mode", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT);

    var_Create (mp, "vbi-page", VLC_VAR_INTEGER);
    var_SetInteger (mp, "vbi-page", 100);

    var_Create (mp, "video-filter", VLC_VAR_STRING | VLC_VAR_DOINHERIT);
    var_Create (mp, "sub-source", VLC_VAR_STRING | VLC_VAR_DOINHERIT);
    var_Create (mp, "sub-filter", VLC_VAR_STRING | VLC_VAR_DOINHERIT);

    var_Create (mp, "osd", VLC_VAR_BOOL); // off
    var_Create (mp, "spu-fill", VLC_VAR_BOOL | VLC_VAR_DOINHERIT);

    doinherit = module_exists("marq") ? VLC_VAR_DOINHERIT : 0;
    var_Create(mp, "marq-marquee", VLC_VAR_STRING);
    var_Create(mp, "marq-color", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "marq-opacity", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "marq-position", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "marq-refresh", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "marq-size", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "marq-timeout", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "marq-x", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "marq-y", VLC_VAR_INTEGER | doinherit);

    doinherit = module_exists("logo") ? VLC_VAR_DOINHERIT : 0;
    var_Create(mp, "logo-file", VLC_VAR_STRING);
    var_Create(mp, "logo-x", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "logo-y", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "logo-delay", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "logo-repeat", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "logo-opacity", VLC_VAR_INTEGER | doinherit);
    var_Create(mp, "logo-position", VLC_VAR_INTEGER | doinherit);

    var_Create(mp, "contrast", VLC_VAR_FLOAT);
    var_SetFloat(mp, "contrast", 1.f);
    var_Create(mp, "brightness", VLC_VAR_FLOAT);
    var_SetFloat(mp, "brightness", 1.f);
    var_Create(mp, "hue", VLC_VAR_FLOAT);
    var_Create(mp, "saturation", VLC_VAR_FLOAT);
    var_SetFloat(mp, "saturation", 1.f);
    var_Create(mp, "gamma", VLC_VAR_FLOAT);
    var_SetFloat(mp, "gamma", 1.f);

     /* Audio */
    var_Create (mp, "aout", VLC_VAR_STRING | VLC_VAR_DOINHERIT);
    var_Create (mp, "audio-device", VLC_VAR_STRING);
    var_Create (mp, "mute", VLC_VAR_BOOL);
    var_Create (mp, "volume", VLC_VAR_FLOAT);
    var_Create (mp, "corks", VLC_VAR_INTEGER);
    var_Create (mp, "audio-filter", VLC_VAR_STRING);
    var_Create (mp, "role", VLC_VAR_STRING | VLC_VAR_DOINHERIT);
    var_Create (mp, "amem-data", VLC_VAR_ADDRESS);
    var_Create (mp, "amem-setup", VLC_VAR_ADDRESS);
    var_Create (mp, "amem-cleanup", VLC_VAR_ADDRESS);
    var_Create (mp, "amem-play", VLC_VAR_ADDRESS);
    var_Create (mp, "amem-pause", VLC_VAR_ADDRESS);
    var_Create (mp, "amem-resume", VLC_VAR_ADDRESS);
    var_Create (mp, "amem-flush", VLC_VAR_ADDRESS);
    var_Create (mp, "amem-drain", VLC_VAR_ADDRESS);
    var_Create (mp, "amem-set-volume", VLC_VAR_ADDRESS);
    var_Create (mp, "amem-format", VLC_VAR_STRING);
    var_Create (mp, "amem-rate", VLC_VAR_INTEGER);
    var_Create (mp, "amem-channels", VLC_VAR_INTEGER);

    /* Video Title */
    var_Create (mp, "video-title-show", VLC_VAR_BOOL);
    var_Create (mp, "video-title-position", VLC_VAR_INTEGER);
    var_Create (mp, "video-title-timeout", VLC_VAR_INTEGER);

    /* Equalizer */
    var_Create (mp, "equalizer-preamp", VLC_VAR_FLOAT);
    var_Create (mp, "equalizer-vlcfreqs", VLC_VAR_BOOL);
    var_Create (mp, "equalizer-bands", VLC_VAR_STRING);
    var_Create (mp, "equalizer-preset", VLC_VAR_STRING);

    /* variables for signalling creation of new files */
    var_Create(mp, "record-file", VLC_VAR_STRING);

    mp->timer.id = NULL;
    mp->p_md = NULL;
    mp->p_libapoi_instance = instance;
    /* use a reentrant lock to allow calling libapoi functions from callbacks */
    mp->player = vlc_player_New(VLC_OBJECT(mp), VLC_PLAYER_LOCK_REENTRANT);
    if (unlikely(!mp->player))
        goto error1;
    vlc_cond_init(&mp->wait);

    vlc_player_Lock(mp->player);

    mp->listener = vlc_player_AddListener(mp->player, &vlc_player_cbs, mp);
    if (unlikely(!mp->listener))
        goto error2;

    mp->aout_listener =
        vlc_player_aout_AddListener(mp->player, &vlc_player_aout_cbs, mp);
    if (unlikely(!mp->aout_listener))
        goto error3;

    vlc_player_Unlock(mp->player);

    vlc_atomic_rc_init(&mp->rc);
    libapoi_event_manager_init(&mp->event_manager, mp);

    /* Snapshot initialization */
    /* Attach a var callback to the global object to provide the glue between
     * vout_thread that generates the event and media_player that re-emits it
     * with its own event manager
     *
     * FIXME: It's unclear why we want to put this in public API, and why we
     * want to expose it in such a limiting and ugly way.
     */
    var_AddCallback(vlc_object_instance(mp),
                    "snapshot-file", snapshot_was_taken, mp);

    libapoi_retain(instance);
    return mp;

error3:
    vlc_player_RemoveListener(mp->player, mp->listener);
error2:
    vlc_player_Unlock(mp->player);
    vlc_player_Delete(mp->player);
error1:
    vlc_object_delete(mp);
    return NULL;
}

/**************************************************************************
 * Create a Media Instance object with a media descriptor.
 **************************************************************************/
libapoi_media_player_t *
libapoi_media_player_new_from_media( libapoi_instance_t *inst,
                                    libapoi_media_t * p_md )
{
    libapoi_media_player_t * p_mi;

    p_mi = libapoi_media_player_new( inst );
    if( !p_mi )
        return NULL;

    libapoi_media_retain( p_md );
    p_mi->p_md = p_md;

    vlc_player_Lock(p_mi->player);
    int ret = vlc_player_SetCurrentMedia(p_mi->player, p_md->p_input_item);
    vlc_player_Unlock(p_mi->player);

    if (ret != VLC_SUCCESS)
    {
        libapoi_media_release(p_md);
        p_mi->p_md = NULL;
        return NULL;
    }

    return p_mi;
}

/**************************************************************************
 * Destroy a Media Instance object (libapoi internal)
 *
 * Warning: No lock held here, but hey, this is internal. Caller must lock.
 **************************************************************************/
static void libapoi_media_player_destroy( libapoi_media_player_t *p_mi )
{
    assert( p_mi );

    /* Detach Callback from the main libapoi object */
    var_DelCallback( vlc_object_instance(p_mi),
                     "snapshot-file", snapshot_was_taken, p_mi );

    vlc_player_Lock(p_mi->player);
    vlc_player_aout_RemoveListener(p_mi->player, p_mi->aout_listener);
    vlc_player_RemoveListener(p_mi->player, p_mi->listener);
    vlc_player_Unlock(p_mi->player);

    vlc_player_Delete(p_mi->player);

    libapoi_event_manager_destroy(&p_mi->event_manager);
    libapoi_media_release( p_mi->p_md );

    free(p_mi->vout.default_dec_dev);
    free(p_mi->vout.default_vout);
    free(p_mi->vout.default_gl);
    free(p_mi->vout.default_gles2);

    libapoi_instance_t *instance = p_mi->p_libapoi_instance;
    vlc_object_delete(p_mi);
    libapoi_release(instance);
}

/**************************************************************************
 * Release a Media Instance object.
 *
 * Function does the locking.
 **************************************************************************/
void libapoi_media_player_release( libapoi_media_player_t *p_mi )
{
    assert( p_mi );
    if( !vlc_atomic_rc_dec( &p_mi->rc ) )
        return;

    libapoi_media_player_destroy( p_mi );
}

void libapoi_media_player_lock( libapoi_media_player_t *mp )
{
    vlc_player_Lock(mp->player);
}

void libapoi_media_player_unlock( libapoi_media_player_t *mp )
{
    vlc_player_Unlock(mp->player);
}

void libapoi_media_player_wait( libapoi_media_player_t *mp )
{
    vlc_player_CondWait(mp->player, &mp->wait);
}

void libapoi_media_player_signal( libapoi_media_player_t *mp )
{
    vlc_cond_broadcast(&mp->wait);
}

/**************************************************************************
 * Retain a Media Instance object.
 *
 * Caller must hold the lock.
 **************************************************************************/
libapoi_media_player_t *libapoi_media_player_retain( libapoi_media_player_t *p_mi )
{
    assert( p_mi );
    vlc_atomic_rc_inc( &p_mi->rc );
    return p_mi;
}

/**************************************************************************
 * Set the Media descriptor associated with the instance.
 *
 * Enter without lock -- function will lock the object.
 **************************************************************************/
void libapoi_media_player_set_media(
                            libapoi_media_player_t *p_mi,
                            libapoi_media_t *p_md )
{
    vlc_player_Lock(p_mi->player);

    libapoi_media_release( p_mi->p_md );

    if( p_md )
        libapoi_media_retain( p_md );
    p_mi->p_md = p_md;

    vlc_player_SetCurrentMedia(p_mi->player, p_md ? p_md->p_input_item : NULL);

    vlc_player_Unlock(p_mi->player);
}

/**************************************************************************
 * Get the Media descriptor associated with the instance.
 **************************************************************************/
libapoi_media_t *
libapoi_media_player_get_media( libapoi_media_player_t *p_mi )
{
    libapoi_media_t *p_m;

    vlc_player_Lock(p_mi->player);
    p_m = p_mi->p_md;
    if( p_m )
        libapoi_media_retain( p_m );
    vlc_player_Unlock(p_mi->player);

    return p_m;
}

/**************************************************************************
 * Get the event Manager.
 **************************************************************************/
libapoi_event_manager_t *
libapoi_media_player_event_manager( libapoi_media_player_t *p_mi )
{
    return &p_mi->event_manager;
}

/**************************************************************************
 * Tell media player to start playing.
 **************************************************************************/
int libapoi_media_player_play( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    int ret = vlc_player_Start(player);
    if (ret == VLC_SUCCESS)
    {
        if (vlc_player_IsPaused(player))
            vlc_player_Resume(player);
    }

    vlc_player_Unlock(player);
    return ret;
}

void libapoi_media_player_set_pause( libapoi_media_player_t *p_mi, int paused )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    if (paused)
    {
        if (vlc_player_CanPause(player))
            vlc_player_Pause(player);
        else
            vlc_player_Stop(player);
    }
    else
    {
        vlc_player_Resume(player);
    }

    vlc_player_Unlock(player);
}

/**************************************************************************
 * Toggle pause.
 **************************************************************************/
void libapoi_media_player_pause( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_TogglePause(player);

    vlc_player_Unlock(player);
}

/**************************************************************************
 * Tells whether the media player is currently playing.
 **************************************************************************/
bool libapoi_media_player_is_playing(libapoi_media_player_t *p_mi)
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    bool ret = vlc_player_IsStarted(player) && !vlc_player_IsPaused(player);

    vlc_player_Unlock(player);
    return ret;
}

/**************************************************************************
 * Stop playing.
 **************************************************************************/
int libapoi_media_player_stop_async( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    int ret = vlc_player_Stop(player);

    vlc_player_Unlock(player);

    return ret;
}

int libapoi_media_player_set_renderer( libapoi_media_player_t *p_mi,
                                      libapoi_renderer_item_t *p_litem )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_renderer_item_t *renderer = libapoi_renderer_item_to_vlc(p_litem);
    vlc_player_SetRenderer(player, renderer);

    vlc_player_Unlock(player);
    return 0;
}

static void libapoi_media_player_switch_vout(libapoi_media_player_t *player,
                                            libapoi_media_player_vout_detach_cb vout_detach)
{
    if (player->vout.window_detach != NULL)
        player->vout.window_detach(player);
    player->vout.window_detach = vout_detach;
}

static void libapoi_media_player_detach_video_callbacks(libapoi_media_player_t *player)
{
    var_SetAddress(player, "vmem-lock", NULL);
    var_SetAddress(player, "vmem-unlock", NULL);
    var_SetAddress(player, "vmem-display", NULL);
    var_SetAddress(player, "vmem-data", NULL);
    var_SetString(player, "dec-dev", player->vout.default_dec_dev);
    var_SetString(player, "vout", player->vout.default_vout);
    var_SetString(player, "window", "any");
}

void libapoi_video_set_callbacks( libapoi_media_player_t *mp,
    void *(*lock_cb) (void *, void **),
    void (*unlock_cb) (void *, void *, void *const *),
    void (*display_cb) (void *, void *),
    void *opaque )
{
    libapoi_media_player_switch_vout(mp, libapoi_media_player_detach_video_callbacks);
    var_SetAddress( mp, "vmem-lock", lock_cb );
    var_SetAddress( mp, "vmem-unlock", unlock_cb );
    var_SetAddress( mp, "vmem-display", display_cb );
    var_SetAddress( mp, "vmem-data", opaque );
    var_SetString( mp, "dec-dev", "none" );
    var_SetString( mp, "vout", "vmem" );
    var_SetString( mp, "window", "dummy" );
}

void libapoi_video_set_format_callbacks( libapoi_media_player_t *mp,
                                        libapoi_video_format_cb setup,
                                        libapoi_video_cleanup_cb cleanup )
{
    var_SetAddress( mp, "vmem-setup", setup );
    var_SetAddress( mp, "vmem-cleanup", cleanup );
}

void libapoi_video_set_format( libapoi_media_player_t *mp, const char *chroma,
                              unsigned width, unsigned height, unsigned pitch )
{
    var_SetString( mp, "vmem-chroma", chroma );
    var_SetInteger( mp, "vmem-width", width );
    var_SetInteger( mp, "vmem-height", height );
    var_SetInteger( mp, "vmem-pitch", pitch );
}

static void libapoi_media_player_detach_output_callbacks(libapoi_media_player_t *player)
{
    var_SetString(player, "dec-dev", player->vout.default_dec_dev);
    var_SetString(player, "vout", player->vout.default_vout);
    var_SetString(player, "gl", player->vout.default_gl);
    var_SetString(player, "gles", player->vout.default_gles2);
    var_SetString(player, "window", "any");

    var_SetInteger(player, "vout-cb-type", libapoi_video_engine_disable);
    var_SetAddress(player, "vout-cb-opaque", NULL);
    var_SetAddress(player, "vout-cb-setup", NULL);
    var_SetAddress(player, "vout-cb-cleanup", NULL);
    var_SetAddress(player, "vout-cb-window-cb", NULL);
    var_SetAddress(player, "vout-cb-update-output", NULL);
    var_SetAddress(player, "vout-cb-swap", NULL);
    var_SetAddress(player, "vout-cb-get-proc-address", NULL);
    var_SetAddress(player, "vout-cb-make-current", NULL);
    var_SetAddress(player, "vout-cb-metadata", NULL);
    var_SetAddress(player, "vout-cb-select-plane", NULL);
}

bool libapoi_video_set_output_callbacks(libapoi_media_player_t *mp,
                                       libapoi_video_engine_t engine,
                                       libapoi_video_output_setup_cb setup_cb,
                                       libapoi_video_output_cleanup_cb cleanup_cb,
                                       libapoi_video_output_set_window_cb set_window_cb,
                                       libapoi_video_update_output_cb update_output_cb,
                                       libapoi_video_swap_cb swap_cb,
                                       libapoi_video_makeCurrent_cb makeCurrent_cb,
                                       libapoi_video_getProcAddress_cb getProcAddress_cb,
                                       libapoi_video_frameMetadata_cb metadata_cb,
                                       libapoi_video_output_select_plane_cb select_plane_cb,
                                       void *opaque)
{
    static_assert(libapoi_video_engine_disable == 0, "No engine set must default to 0");


    if (engine == libapoi_video_engine_disable)
    {
        libapoi_media_player_switch_vout(mp, NULL);
        return true;
    }

    var_SetString( mp, "window", "wextern");

    if (engine == libapoi_video_engine_gles2)
    {
        var_SetString(mp, "vout", "gles2");
        var_SetString(mp, "gles2", "vgl");
        libapoi_media_player_switch_vout(mp, libapoi_media_player_detach_output_callbacks);
    }
    else if (engine == libapoi_video_engine_opengl)
    {
        var_SetString(mp, "vout", "gl");
        var_SetString(mp, "gl", "vgl");
        libapoi_media_player_switch_vout(mp, libapoi_media_player_detach_output_callbacks);
    }
    else if (engine == libapoi_video_engine_d3d11)
    {
        var_SetString(mp, "vout", "d3d11drawable");
        var_SetString(mp, "dec-dev", "d3d11");
        libapoi_media_player_switch_vout(mp, libapoi_media_player_detach_output_callbacks);
    }
    else if (engine == libapoi_video_engine_d3d9)
    {
        var_SetString(mp, "vout", "direct3d9");
        var_SetString(mp, "dec-dev", "d3d9");
        libapoi_media_player_switch_vout(mp, libapoi_media_player_detach_output_callbacks);
    }
    else if (engine == libapoi_video_engine_anw)
    {
        /* Force android-display is using MediaCodec or fallback to GL (any) */
        var_SetString(mp, "vout", "android-display,any");
        var_SetString(mp, "dec-dev", "android");
        var_SetString(mp, "window", "android");
        libapoi_media_player_switch_vout(mp, libapoi_media_player_detach_output_callbacks);
    }
    else
        return false;

    var_SetInteger( mp, "vout-cb-type", engine );
    var_SetAddress( mp, "vout-cb-opaque", opaque );
    var_SetAddress( mp, "vout-cb-setup", setup_cb );
    var_SetAddress( mp, "vout-cb-cleanup", cleanup_cb );
    var_SetAddress( mp, "vout-cb-window-cb", set_window_cb );
    var_SetAddress( mp, "vout-cb-update-output", update_output_cb );
    var_SetAddress( mp, "vout-cb-swap", swap_cb );
    var_SetAddress( mp, "vout-cb-get-proc-address", getProcAddress_cb );
    var_SetAddress( mp, "vout-cb-make-current", makeCurrent_cb );
    var_SetAddress( mp, "vout-cb-metadata", metadata_cb );
    var_SetAddress( mp, "vout-cb-select-plane", select_plane_cb );
    return true;
}

static void libapoi_media_player_detach_nsobject(libapoi_media_player_t *player)
{
    var_SetAddress(player, "drawable-nsobject", NULL);
}

/**************************************************************************
 * set_nsobject
 **************************************************************************/
void libapoi_media_player_set_nsobject( libapoi_media_player_t *p_mi,
                                        void * drawable )
{
    assert (p_mi != NULL);
    libapoi_media_player_switch_vout(p_mi, libapoi_media_player_detach_nsobject);

#ifdef __APPLE__
    var_SetAddress (p_mi, "drawable-nsobject", drawable);
#else
    (void)drawable;
    libapoi_printerr ("can't set nsobject: APPLE build required");
    assert(false);
    var_SetString (p_mi, "window", "none");
#endif
}

/**************************************************************************
 * get_nsobject
 **************************************************************************/
void * libapoi_media_player_get_nsobject( libapoi_media_player_t *p_mi )
{
    assert (p_mi != NULL);
#ifdef __APPLE__
    return var_GetAddress (p_mi, "drawable-nsobject");
#else
    (void) p_mi;
    return NULL;
#endif
}

static void libapoi_media_player_detach_xwindow(libapoi_media_player_t *player)
{
    /* Window variable is not configurable from libapoi */
    var_SetString(player, "window", "any");
    var_SetInteger(player, "drawable-xid", 0);
}

/**************************************************************************
 * set_xwindow
 **************************************************************************/
void libapoi_media_player_set_xwindow( libapoi_media_player_t *p_mi,
                                      uint32_t drawable )
{
    assert (p_mi != NULL);
    libapoi_media_player_switch_vout(p_mi, libapoi_media_player_detach_xwindow);

    var_SetString (p_mi, "window", drawable ? "embed-xid,any" : "any");
    var_SetInteger (p_mi, "drawable-xid", drawable);
}

/**************************************************************************
 * get_xwindow
 **************************************************************************/
uint32_t libapoi_media_player_get_xwindow( libapoi_media_player_t *p_mi )
{
    return var_GetInteger (p_mi, "drawable-xid");
}

static void libapoi_media_player_detach_hwnd(libapoi_media_player_t *player)
{
    /* Window variable is not configurable from libapoi */
    var_SetString(player, "window", "any");
    var_SetInteger(player, "drawable-hwnd", 0);
}

/**************************************************************************
 * set_hwnd
 **************************************************************************/
void libapoi_media_player_set_hwnd( libapoi_media_player_t *p_mi,
                                   void *drawable )
{
    assert (p_mi != NULL);
    libapoi_media_player_switch_vout(p_mi, libapoi_media_player_detach_hwnd);
#if defined (_WIN32) || defined (__OS2__)
    var_SetString (p_mi, "window",
                   (drawable != NULL) ? "embed-hwnd,any" : "any");
    var_SetInteger (p_mi, "drawable-hwnd", (uintptr_t)drawable);
#else
    (void) drawable;
    libapoi_printerr ("can't set hwnd: WIN32 build required");
    assert(false);
    var_SetString (p_mi, "window", "none");
#endif
}

/**************************************************************************
 * get_hwnd
 **************************************************************************/
void *libapoi_media_player_get_hwnd( libapoi_media_player_t *p_mi )
{
    assert (p_mi != NULL);
#if defined (_WIN32) || defined (__OS2__)
    return (void *)(uintptr_t)var_GetInteger (p_mi, "drawable-hwnd");
#else
    (void) p_mi;
    return NULL;
#endif
}

static void libapoi_media_player_detach_android_context(libapoi_media_player_t *player)
{
    var_SetAddress(player, "drawable-androidwindow", NULL);
}


/**************************************************************************
 * set_android_context
 **************************************************************************/
void libapoi_media_player_set_android_context( libapoi_media_player_t *p_mi,
                                              void *p_awindow_handler )
{
    assert (p_mi != NULL);
    libapoi_media_player_switch_vout(p_mi, libapoi_media_player_detach_android_context);
#ifdef __ANDROID__
    var_SetAddress (p_mi, "drawable-androidwindow", p_awindow_handler);
#else
    (void) p_awindow_handler;
    libapoi_printerr ("can't set android context: ANDROID build required");
    assert(false);
    var_SetString (p_mi, "window", "none");
#endif
}

void libapoi_audio_set_callbacks( libapoi_media_player_t *mp,
                                 libapoi_audio_play_cb play_cb,
                                 libapoi_audio_pause_cb pause_cb,
                                 libapoi_audio_resume_cb resume_cb,
                                 libapoi_audio_flush_cb flush_cb,
                                 libapoi_audio_drain_cb drain_cb,
                                 void *opaque )
{
    var_SetAddress( mp, "amem-play", play_cb );
    var_SetAddress( mp, "amem-pause", pause_cb );
    var_SetAddress( mp, "amem-resume", resume_cb );
    var_SetAddress( mp, "amem-flush", flush_cb );
    var_SetAddress( mp, "amem-drain", drain_cb );
    var_SetAddress( mp, "amem-data", opaque );
    var_SetString( mp, "aout", "amem,none" );

    vlc_player_aout_Reset( mp->player );
}

void libapoi_audio_set_volume_callback( libapoi_media_player_t *mp,
                                       libapoi_audio_set_volume_cb cb )
{
    var_SetAddress( mp, "amem-set-volume", cb );

    vlc_player_aout_Reset( mp->player );
}

void libapoi_audio_set_format_callbacks( libapoi_media_player_t *mp,
                                        libapoi_audio_setup_cb setup,
                                        libapoi_audio_cleanup_cb cleanup )
{
    var_SetAddress( mp, "amem-setup", setup );
    var_SetAddress( mp, "amem-cleanup", cleanup );

    vlc_player_aout_Reset( mp->player );
}

void libapoi_audio_set_format( libapoi_media_player_t *mp, const char *format,
                              unsigned rate, unsigned channels )
{
    var_SetString( mp, "amem-format", format );
    var_SetInteger( mp, "amem-rate", rate );
    var_SetInteger( mp, "amem-channels", channels );

    vlc_player_aout_Reset( mp->player );
}


/**************************************************************************
 * Getters for stream information
 **************************************************************************/
libapoi_time_t libapoi_media_player_get_length(
                             libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_tick_t length = vlc_player_GetLength(player);
    libapoi_time_t i_time = libapoi_time_from_vlc_tick(length);

    vlc_player_Unlock(player);
    return i_time;
}

libapoi_time_t libapoi_media_player_get_time( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_tick_t tick = vlc_player_GetTime(player);
    libapoi_time_t i_time = libapoi_time_from_vlc_tick(tick);

    vlc_player_Unlock(player);
    return i_time;
}

static int
set_time( libapoi_media_player_t *p_mi, libapoi_time_t i_time, bool b_fast,
          enum vlc_player_whence whence )
{
    vlc_tick_t tick = vlc_tick_from_libapoi_time(i_time);

    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    enum vlc_player_seek_speed speed = b_fast ? VLC_PLAYER_SEEK_FAST
                                              : VLC_PLAYER_SEEK_PRECISE;
    vlc_player_SeekByTime(player, tick, speed, whence);

    vlc_player_Unlock(player);

    /* may not fail anymore, keep int not to break the API */
    return 0;
}

int libapoi_media_player_set_time( libapoi_media_player_t *p_mi,
                                  libapoi_time_t i_time, bool b_fast )
{
    return set_time( p_mi, i_time, b_fast, VLC_PLAYER_WHENCE_ABSOLUTE );
}

int libapoi_media_player_jump_time( libapoi_media_player_t *p_mi,
                                   libapoi_time_t i_time )
{
    return set_time( p_mi, i_time, false, VLC_PLAYER_WHENCE_RELATIVE );
}

int libapoi_media_player_set_position( libapoi_media_player_t *p_mi,
                                      double position, bool b_fast )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    enum vlc_player_seek_speed speed = b_fast ? VLC_PLAYER_SEEK_FAST
                                              : VLC_PLAYER_SEEK_PRECISE;
    vlc_player_SeekByPos(player, position, speed, VLC_PLAYER_WHENCE_ABSOLUTE);

    vlc_player_Unlock(player);

    /* may not fail anymore, keep int not to break the API */
    return 0;
}

double libapoi_media_player_get_position( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    double f_position = vlc_player_GetPosition(player);

    vlc_player_Unlock(player);
    return f_position;
}

int
libapoi_media_player_set_abloop_time( libapoi_media_player_t *p_mi,
                                     libapoi_time_t a_time, libapoi_time_t b_time )
{
    vlc_tick_t a_tick = vlc_tick_from_libapoi_time(a_time);
    if (a_tick >= 0)
        a_tick += VLC_TICK_0;

    vlc_tick_t b_tick = vlc_tick_from_libapoi_time(b_time);
    if (b_tick >= 0)
        b_tick += VLC_TICK_0;

    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    int ret = vlc_player_SetAtoBLoopTime(player, a_tick, b_tick);

    vlc_player_Unlock(player);
    return ret == VLC_SUCCESS ? 0 : -1;
}

int
libapoi_media_player_set_abloop_position( libapoi_media_player_t *p_mi,
                                         double a_pos, double b_pos )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    int ret = vlc_player_SetAtoBLoopPosition(player, a_pos, b_pos);

    vlc_player_Unlock(player);
    return ret == VLC_SUCCESS ? 0 : -1;
}

int
libapoi_media_player_reset_abloop( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    int ret = vlc_player_ResetAtoBLoop(player);

    vlc_player_Unlock(player);
    return ret == VLC_SUCCESS ? 0 : -1;
}

libapoi_abloop_t
libapoi_media_player_get_abloop( libapoi_media_player_t *p_mi,
                                libapoi_time_t *a_time, double *a_pos,
                                libapoi_time_t *b_time, double *b_pos )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_tick_t a_ticks, b_ticks;
    int ret = vlc_player_GetAtoBLoop(player, &a_ticks, a_pos, &b_ticks, b_pos);

    vlc_player_Unlock(player);

    if (a_time != NULL)
        *a_time = a_ticks == VLC_TICK_INVALID ? -1 :
                  libapoi_time_from_vlc_tick(a_ticks);
    if (b_time != NULL)
        *b_time = b_ticks == VLC_TICK_INVALID ? -1 :
                  libapoi_time_from_vlc_tick(b_ticks);

    return ret;
}

void libapoi_media_player_set_chapter( libapoi_media_player_t *p_mi,
                                      int chapter )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_SelectChapterIdx(player, chapter);

    vlc_player_Unlock(player);
}

int libapoi_media_player_get_chapter( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    ssize_t i_chapter = vlc_player_GetSelectedChapterIdx(player);

    vlc_player_Unlock(player);
    return i_chapter;
}

int libapoi_media_player_get_chapter_count( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    const struct vlc_player_title *title = vlc_player_GetSelectedTitle(player);
    int ret = title ? (int) title->chapter_count : -1;

    vlc_player_Unlock(player);
    return ret;
}

int libapoi_media_player_get_chapter_count_for_title(
                                 libapoi_media_player_t *p_mi,
                                 int i_title )
{
    assert(i_title >= 0);
    size_t idx = i_title;
    int ret = -1;

    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_title_list *titles = vlc_player_GetTitleList(player);
    if (!titles)
        goto end;

    size_t titles_count = vlc_player_title_list_GetCount(titles);
    if (idx >= titles_count)
       goto end;

    const struct vlc_player_title *title =
        vlc_player_title_list_GetAt(titles, idx);
    assert(title);

    ret = title->chapter_count;

end:
    vlc_player_Unlock(player);
    return ret;
}

void libapoi_media_player_set_title( libapoi_media_player_t *p_mi,
                                    int i_title )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_SelectTitleIdx(player, i_title);

    vlc_player_Unlock(player);
}

int libapoi_media_player_get_title( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    ssize_t i_title = vlc_player_GetSelectedTitleIdx(player);

    vlc_player_Unlock(player);
    return i_title;
}

int libapoi_media_player_get_title_count( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_title_list *titles = vlc_player_GetTitleList(player);
    int ret = titles ? (int) vlc_player_title_list_GetCount(titles) : -1;

    vlc_player_Unlock(player);
    return ret;
}

int libapoi_media_player_get_full_title_descriptions( libapoi_media_player_t *p_mi,
                                                     libapoi_title_description_t *** pp_titles )
{
    assert( p_mi );

    int ret = -1;

    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_title_list *titles = vlc_player_GetTitleList(player);
    if (!titles)
        goto end;

    size_t count = vlc_player_title_list_GetCount(titles);

    libapoi_title_description_t **descs = vlc_alloc(count, sizeof(*descs));
    if (count > 0 && !descs)
        goto end;

    for (size_t i = 0; i < count; i++)
    {
        const struct vlc_player_title *title =
            vlc_player_title_list_GetAt(titles, i);
        libapoi_title_description_t *desc = malloc(sizeof(*desc));
        if (!desc)
        {
            libapoi_title_descriptions_release(descs, i);
            goto end;
        }

        descs[i] = desc;

        /* we want to return milliseconds to match the rest of the API */
        desc->i_duration = MS_FROM_VLC_TICK(title->length);
        desc->i_flags = title->flags;
        desc->psz_name = title->name ? strdup(title->name) : NULL;
    }

    ret = count;
    *pp_titles = descs;

end:
    vlc_player_Unlock(player);
    return ret;
}

void libapoi_title_descriptions_release( libapoi_title_description_t **p_titles,
                                        unsigned i_count )
{
    for (unsigned i = 0; i < i_count; i++ )
    {
        if ( !p_titles[i] )
            continue;

        free( p_titles[i]->psz_name );
        free( p_titles[i] );
    }
    free( p_titles );
}

int libapoi_media_player_get_full_chapter_descriptions( libapoi_media_player_t *p_mi,
                                                      int i_chapters_of_title,
                                                      libapoi_chapter_description_t *** pp_chapters )
{
    assert( p_mi );

    int ret = -1;

    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_title_list *titles = vlc_player_GetTitleList(player);
    if (!titles)
        goto end;

    size_t titles_count = vlc_player_title_list_GetCount(titles);

    ssize_t title_idx;
    if (i_chapters_of_title < 0)
    {
        title_idx = vlc_player_GetSelectedTitleIdx(player);
        if (title_idx == -1)
            goto end;

        assert((size_t)title_idx < titles_count);
    }
    else
    {
        title_idx = i_chapters_of_title;

        if ((size_t) title_idx  >= titles_count)
           goto end;
    }

    const struct vlc_player_title *title =
        vlc_player_title_list_GetAt(titles, title_idx);
    assert(title);

    size_t i_chapter_count = title->chapter_count;

    libapoi_chapter_description_t **descs =
        vlc_alloc(i_chapter_count, sizeof(*descs));
    if (i_chapter_count > 0 && !descs)
        goto end;

    for (size_t i = 0; i < i_chapter_count; i++)
    {
        const struct vlc_player_chapter *chapter = &title->chapters[i];
        libapoi_chapter_description_t *desc = malloc(sizeof(*desc));
        if (!desc)
        {
            libapoi_chapter_descriptions_release(descs, i);
            goto end;
        }

        descs[i] = desc;

        vlc_tick_t chapter_end = i < i_chapter_count - 1
                               ? title->chapters[i + 1].time
                               : title->length;
        desc->i_time_offset = MS_FROM_VLC_TICK(chapter->time);
        desc->psz_name = chapter->name ? strdup(chapter->name) : NULL;
        desc->i_duration = MS_FROM_VLC_TICK(chapter_end) - desc->i_time_offset;
    }

    ret = i_chapter_count;
    *pp_chapters = descs;

end:
    vlc_player_Unlock(player);
    return ret;
}

void libapoi_chapter_descriptions_release( libapoi_chapter_description_t **p_chapters,
                                          unsigned i_count )
{
    for (unsigned i = 0; i < i_count; i++ )
    {
        if ( !p_chapters[i] )
            continue;

        free( p_chapters[i]->psz_name );
        free( p_chapters[i] );
    }
    free( p_chapters );
}

void libapoi_media_player_next_chapter( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_SelectNextChapter(player);

    vlc_player_Unlock(player);
}

void libapoi_media_player_previous_chapter( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_SelectPrevChapter(player);

    vlc_player_Unlock(player);
}

int libapoi_media_player_set_rate( libapoi_media_player_t *p_mi, float rate )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_ChangeRate(player, rate);

    vlc_player_Unlock(player);
    return 0;
}

float libapoi_media_player_get_rate( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    float rate = vlc_player_GetRate(player);

    vlc_player_Unlock(player);
    return rate;
}

libapoi_state_t libapoi_media_player_get_state( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    enum vlc_player_error error = vlc_player_GetError(player);
    enum vlc_player_state state = vlc_player_GetState(player);

    vlc_player_Unlock(player);

    if (error != VLC_PLAYER_ERROR_NONE)
        return libapoi_Error;
    switch (state) {
        case VLC_PLAYER_STATE_STOPPED:
            return libapoi_Stopped;
        case VLC_PLAYER_STATE_STOPPING:
            return libapoi_Stopping;
        case VLC_PLAYER_STATE_STARTED:
            return libapoi_Opening;
        case VLC_PLAYER_STATE_PLAYING:
            return libapoi_Playing;
        case VLC_PLAYER_STATE_PAUSED:
            return libapoi_Paused;
    }
    vlc_assert_unreachable();
}

bool libapoi_media_player_is_seekable(libapoi_media_player_t *p_mi)
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    bool b_seekable = vlc_player_CanSeek(player);

    vlc_player_Unlock(player);
    return b_seekable;
}

void libapoi_media_player_navigate( libapoi_media_player_t* p_mi,
                                   unsigned navigate )
{
    static const enum vlc_player_nav map[] =
    {
        VLC_PLAYER_NAV_ACTIVATE, VLC_PLAYER_NAV_UP, VLC_PLAYER_NAV_DOWN,
        VLC_PLAYER_NAV_LEFT, VLC_PLAYER_NAV_RIGHT, VLC_PLAYER_NAV_POPUP,
    };

    if( navigate >= sizeof(map) / sizeof(map[0]) )
      return;

    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_Navigate(player, map[navigate]);

    vlc_player_Unlock(player);
}

bool libapoi_media_player_can_pause(libapoi_media_player_t *p_mi)
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    bool b_can_pause = vlc_player_CanPause(player);

    vlc_player_Unlock(player);
    return b_can_pause;
}

bool libapoi_media_player_program_scrambled(libapoi_media_player_t *p_mi)
{
    const struct vlc_player_program *program;
    bool b_program_scrambled = false;

    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);
    program = vlc_player_GetSelectedProgram(player);
    if (program != NULL)
        b_program_scrambled = program->scrambled;
    vlc_player_Unlock(player);
    return b_program_scrambled;
}

void libapoi_media_player_next_frame( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    vlc_player_NextVideoFrame(player);

    vlc_player_Unlock(player);
}

/**
 * Private lookup table to get subpicture alignment flag values corresponding
 * to a libapoi_position_t enumerated value.
 */
static const unsigned char position_subpicture_alignment[] = {
    [libapoi_position_center]       = 0,
    [libapoi_position_left]         = SUBPICTURE_ALIGN_LEFT,
    [libapoi_position_right]        = SUBPICTURE_ALIGN_RIGHT,
    [libapoi_position_top]          = SUBPICTURE_ALIGN_TOP,
    [libapoi_position_top_left]     = SUBPICTURE_ALIGN_TOP | SUBPICTURE_ALIGN_LEFT,
    [libapoi_position_top_right]    = SUBPICTURE_ALIGN_TOP | SUBPICTURE_ALIGN_RIGHT,
    [libapoi_position_bottom]       = SUBPICTURE_ALIGN_BOTTOM,
    [libapoi_position_bottom_left]  = SUBPICTURE_ALIGN_BOTTOM | SUBPICTURE_ALIGN_LEFT,
    [libapoi_position_bottom_right] = SUBPICTURE_ALIGN_BOTTOM | SUBPICTURE_ALIGN_RIGHT
};

void libapoi_media_player_set_video_title_display( libapoi_media_player_t *p_mi, libapoi_position_t position, unsigned timeout )
{
    assert( position >= libapoi_position_disable && position <= libapoi_position_bottom_right );

    if ( position != libapoi_position_disable )
    {
        var_SetBool( p_mi, "video-title-show", true );
        var_SetInteger( p_mi, "video-title-position", position_subpicture_alignment[position] );
        var_SetInteger( p_mi, "video-title-timeout", timeout );
    }
    else
    {
        var_SetBool( p_mi, "video-title-show", false );
    }
}

libapoi_media_tracklist_t *
libapoi_media_player_get_tracklist(libapoi_media_player_t *p_mi,
                                  libapoi_track_type_t type, bool selected)
{
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    libapoi_media_tracklist_t *list =
        libapoi_media_tracklist_from_player(player, type, selected);

    vlc_player_Unlock(player);

    return list;
}

libapoi_media_track_t *
libapoi_media_player_get_selected_track(libapoi_media_player_t *p_mi,
                                       libapoi_track_type_t type)
{
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    const enum es_format_category_e cat = libapoi_track_type_to_escat(type);
    const struct vlc_player_track *track =
        vlc_player_GetSelectedTrack(player, cat);

    if (track == NULL)
    {
        vlc_player_Unlock(player);
        return NULL;
    }

    libapoi_media_track_t *libtrack =
        libapoi_media_track_create_from_player_track(track);
    vlc_player_Unlock(player);

    return libtrack;
}

libapoi_media_track_t *
libapoi_media_player_get_track_from_id( libapoi_media_player_t *p_mi,
                                       const char *psz_id )
{
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    enum es_format_category_e cats[] = { VIDEO_ES, AUDIO_ES, SPU_ES };
    for (size_t i = 0; i < ARRAY_SIZE(cats); ++i)
    {
        enum es_format_category_e cat = cats[i];
        size_t count = vlc_player_GetTrackCount(player, cat);

        for (size_t j = 0; j < count; ++j)
        {
            const struct vlc_player_track *track =
                vlc_player_GetTrackAt(player, cat, j);
            if (strcmp(psz_id, vlc_es_id_GetStrId(track->es_id)) == 0)
            {
                libapoi_media_track_t *libtrack =
                    libapoi_media_track_create_from_player_track(track);
                vlc_player_Unlock(player);
                return libtrack;

            }
        }
    }

    vlc_player_Unlock(player);
    return NULL;
}

void
libapoi_media_player_select_track(libapoi_media_player_t *p_mi,
                                 const libapoi_media_track_t *track)
{
    assert( track != NULL );
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    const libapoi_media_trackpriv_t *trackpriv =
        libapoi_media_track_to_priv(track);

    // It must be a player track
    assert(trackpriv->es_id);

    vlc_player_SelectEsId(player, trackpriv->es_id,
                          VLC_PLAYER_SELECT_EXCLUSIVE);

    vlc_player_Unlock(player);
}

void
libapoi_media_player_unselect_track_type( libapoi_media_player_t *p_mi,
                                         libapoi_track_type_t type )
{
    vlc_player_t *player = p_mi->player;
    const enum es_format_category_e cat = libapoi_track_type_to_escat(type);

    vlc_player_Lock(player);
    vlc_player_UnselectTrackCategory(player, cat);
    vlc_player_Unlock(player);
}

void
libapoi_media_player_select_tracks(libapoi_media_player_t *p_mi,
                                  libapoi_track_type_t type,
                                  const libapoi_media_track_t **tracks,
                                  size_t track_count)
{
    vlc_player_t *player = p_mi->player;

    vlc_es_id_t **es_id_list = vlc_alloc(track_count + 1, sizeof(vlc_es_id_t *));
    size_t es_id_idx = 0;

    if (es_id_list == NULL)
        return;

    const enum es_format_category_e cat = libapoi_track_type_to_escat(type);

    vlc_player_Lock(player);

    for (size_t i = 0; i < track_count; ++i)
    {
        const libapoi_media_track_t *track = tracks[i];
        const libapoi_media_trackpriv_t *trackpriv =
            libapoi_media_track_to_priv(track);

        // It must be a player track
        assert(trackpriv->es_id);

        es_id_list[es_id_idx++] = trackpriv->es_id;
    }
    es_id_list[es_id_idx++] = NULL;
    vlc_player_SelectEsIdList(player, cat, es_id_list);

    vlc_player_Unlock(player);

    free(es_id_list);
}

void
libapoi_media_player_select_tracks_by_ids( libapoi_media_player_t *p_mi,
                                          libapoi_track_type_t type,
                                          const char *psz_ids )
{
    const enum es_format_category_e cat = libapoi_track_type_to_escat(type);

    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    vlc_player_SelectTracksByStringIds(player, cat, psz_ids);

    vlc_player_Unlock(player);
}

int libapoi_media_player_add_slave( libapoi_media_player_t *p_mi,
                                   libapoi_media_slave_type_t i_type,
                                   const char *psz_uri, bool b_select )
{
    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    enum es_format_category_e cat = i_type == libapoi_media_slave_type_subtitle
                                  ? SPU_ES
                                  : AUDIO_ES;

    int ret = vlc_player_AddAssociatedMedia(player, cat, psz_uri, b_select,
                                            false, false);

    vlc_player_Unlock(player);
    return ret;
}

/**
 * Maximum size of a formatted equalizer amplification band frequency value.
 *
 * The allowed value range is supposed to be constrained from -20.0 to 20.0.
 *
 * The format string " %.07f" with a minimum value of "-20" gives a maximum
 * string length of e.g. " -19.1234567", i.e. 12 bytes (not including the null
 * terminator).
 */
#define EQZ_BAND_VALUE_SIZE 12

int libapoi_media_player_set_equalizer( libapoi_media_player_t *p_mi, libapoi_equalizer_t *p_equalizer )
{
    char bands[EQZ_BANDS_MAX * EQZ_BAND_VALUE_SIZE + 1];

    if( p_equalizer != NULL )
    {
        for( unsigned i = 0, c = 0; i < EQZ_BANDS_MAX; i++ )
        {
            c += snprintf( bands + c, sizeof(bands) - c, " %.07f",
                          p_equalizer->f_amp[i] );
            if( unlikely(c >= sizeof(bands)) )
                return -1;
        }

        var_SetFloat( p_mi, "equalizer-preamp", p_equalizer->f_preamp );
        var_SetString( p_mi, "equalizer-bands", bands );
    }
    var_SetString( p_mi, "audio-filter", p_equalizer ? "equalizer" : "" );

    audio_output_t *p_aout = vlc_player_aout_Hold( p_mi->player );
    if( p_aout != NULL )
    {
        if( p_equalizer != NULL )
        {
            var_SetFloat( p_aout, "equalizer-preamp", p_equalizer->f_preamp );
            var_SetString( p_aout, "equalizer-bands", bands );
        }

        var_SetString( p_aout, "audio-filter", p_equalizer ? "equalizer" : "" );
        aout_Release(p_aout);
    }

    return 0;
}


static libapoi_player_program_t *
libapoi_player_program_new(const struct vlc_player_program *program)
{
    libapoi_player_program_t *libprogram = malloc(sizeof(*libprogram));
    if (libprogram == NULL)
        return NULL;

    libprogram->i_group_id = program->group_id;
    libprogram->psz_name = strdup(program->name);
    libprogram->b_selected = program->selected;
    libprogram->b_scrambled = program->scrambled;

    return libprogram;
}

void
libapoi_player_program_delete( libapoi_player_program_t *program )
{
    free( program->psz_name );
    free( program );
}

void libapoi_media_player_select_program_id( libapoi_media_player_t *p_mi,
                                            int program_id)
{
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    vlc_player_SelectProgram(player, program_id);

    vlc_player_Unlock(player);
}

libapoi_player_program_t *
libapoi_media_player_get_selected_program( libapoi_media_player_t *p_mi)
{
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    const struct vlc_player_program *program = vlc_player_GetSelectedProgram( player );
    if( program == NULL )
    {
        vlc_player_Unlock(player);
        return NULL;
    }
    libapoi_player_program_t *libprogram = libapoi_player_program_new(program);

    vlc_player_Unlock(player);

    return libprogram;
}

libapoi_player_program_t *
libapoi_media_player_get_program_from_id( libapoi_media_player_t *p_mi, int i_group_id )
{
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    libapoi_player_program_t *libprogram = NULL;

    size_t count = vlc_player_GetProgramCount(player);
    for (size_t i = 0; i < count; ++i)
    {
        const struct vlc_player_program *program =
            vlc_player_GetProgramAt(player, i);
        assert(program);
        if (program->group_id == i_group_id)
        {
            libprogram = libapoi_player_program_new(program);
            break;
        }
    }

    vlc_player_Unlock(player);

    return libprogram;
}

struct libapoi_player_programlist_t
{
    size_t count;
    libapoi_player_program_t *programs[];
};

size_t
libapoi_player_programlist_count( const libapoi_player_programlist_t *list )
{
    return list->count;
}

libapoi_player_program_t *
libapoi_player_programlist_at( libapoi_player_programlist_t *list, size_t index )
{
    assert(index < list->count);
    return list->programs[index];
}

void
libapoi_player_programlist_delete( libapoi_player_programlist_t *list )
{
    for (size_t i = 0; i < list->count; ++i)
        libapoi_player_program_delete(list->programs[i]);
    free(list);
}

libapoi_player_programlist_t *
libapoi_media_player_get_programlist( libapoi_media_player_t *p_mi )
{
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    size_t count = vlc_player_GetProgramCount(player);
    if (count == 0)
        goto error;

    size_t size;
    if (ckd_mul(&size, count, sizeof (libapoi_player_program_t *)) ||
        ckd_add(&size, sizeof (libapoi_player_programlist_t), size))
        goto error;

    libapoi_player_programlist_t *list = malloc( size );
    if( list == NULL )
        goto error;

    list->count = 0;
    for (size_t i = 0; i < count; ++i)
    {
        const struct vlc_player_program *program =
            vlc_player_GetProgramAt(player, i);
        assert(program);
        list->programs[i] = libapoi_player_program_new(program);
        if (list->programs[i] == NULL)
        {
            libapoi_player_programlist_delete(list);
            goto error;
        }

        list->count++;
    }

    vlc_player_Unlock(player);

    return list;

error:
    vlc_player_Unlock(player);
    return NULL;
}

static const char roles[][16] =
{
    [libapoi_role_Music] =         "music",
    [libapoi_role_Video] =         "video",
    [libapoi_role_Communication] = "communication",
    [libapoi_role_Game] =          "game",
    [libapoi_role_Notification] =  "notification",
    [libapoi_role_Animation] =     "animation",
    [libapoi_role_Production] =    "production",
    [libapoi_role_Accessibility] = "accessibility",
    [libapoi_role_Test] =          "test",
};

int libapoi_media_player_set_role(libapoi_media_player_t *mp, unsigned role)
{
    if (role >= ARRAY_SIZE(roles)
     || var_SetString(mp, "role", roles[role]) != VLC_SUCCESS)
        return -1;
    return 0;
}

int libapoi_media_player_get_role(libapoi_media_player_t *mp)
{
    int ret = -1;
    char *str = var_GetString(mp, "role");
    if (str == NULL)
        return 0;

    for (size_t i = 0; i < ARRAY_SIZE(roles); i++)
        if (!strcmp(roles[i], str))
        {
            ret = i;
            break;
        }

    free(str);
    return ret;
}

void libapoi_media_player_record( libapoi_media_player_t *p_mi,
                                 bool enable,
                                 const char *path)
{
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);
    vlc_player_SetRecordingEnabled(player, enable, path);
    vlc_player_Unlock(player);
}

#define PLAYER_TIME_CORE_TO_LIB(point) { \
    .position = point->position, \
    .rate = point->rate, \
    .ts_us = US_FROM_VLC_TICK(point->ts), \
    .length_us = US_FROM_VLC_TICK(point->length), \
    .system_date_us = US_FROM_VLC_TICK(point->system_date), \
}

#define PLAYER_TIME_LIB_TO_CORE(point) { \
    .position = point->position, \
    .rate = point->rate, \
    .ts = VLC_TICK_FROM_US(point->ts_us), \
    .length = VLC_TICK_FROM_US(point->length_us), \
    .system_date = VLC_TICK_FROM_US(point->system_date_us), \
}

static void player_timer_on_update(const struct vlc_player_timer_point *point,
                                   void *data)
{
    libapoi_media_player_t *p_mi = data;

    if (p_mi->timer.seeking)
        return;

    const libapoi_media_player_time_point_t libpoint = PLAYER_TIME_CORE_TO_LIB(point);

    p_mi->timer.on_update(&libpoint, p_mi->timer.cbs_data);
}

static void player_timer_on_paused(vlc_tick_t system_date, void *data)
{
    libapoi_media_player_t *p_mi = data;

    if (p_mi->timer.on_paused == NULL)
        return;

    p_mi->timer.on_paused(US_FROM_VLC_TICK(system_date), p_mi->timer.cbs_data);
}

static void player_timer_on_seek(const struct vlc_player_timer_point *point,
                                 void *data)
{
    libapoi_media_player_t *p_mi = data;

    if (p_mi->timer.on_seek == NULL)
        return;

    if (point != NULL)
    {
        const libapoi_media_player_time_point_t libpoint = PLAYER_TIME_CORE_TO_LIB(point);
        p_mi->timer.on_seek(&libpoint, p_mi->timer.cbs_data);
        p_mi->timer.seeking = true;
    }
    else
    {
        p_mi->timer.on_seek(NULL, p_mi->timer.cbs_data);
        p_mi->timer.seeking = false;
    }
}

int
libapoi_media_player_watch_time(libapoi_media_player_t *p_mi,
                               int64_t min_period_us,
                               libapoi_media_player_watch_time_on_update on_update,
                               libapoi_media_player_watch_time_on_paused on_paused,
                               libapoi_media_player_watch_time_on_seek on_seek,
                               void *cbs_data)
{
    assert(on_update != NULL);

    static const struct vlc_player_timer_cbs player_timer_cbs = {
        .on_update = player_timer_on_update,
        .on_paused = player_timer_on_paused,
        .on_seek = player_timer_on_seek,
    };

    vlc_player_t *player = p_mi->player;
    vlc_player_Lock(player);

    if (p_mi->timer.id != NULL)
    {
        libapoi_printerr("libapoi_media_player_watch_time error:"
                        "already watching for events");
        vlc_player_Unlock(player);
        return -1;
    }

    p_mi->timer.on_update = on_update;
    p_mi->timer.on_paused = on_paused;
    p_mi->timer.on_seek = on_seek;
    p_mi->timer.cbs_data = cbs_data;
    p_mi->timer.seeking = false;

    p_mi->timer.id = vlc_player_AddTimer(player, VLC_TICK_FROM_US(min_period_us),
                                         &player_timer_cbs, p_mi);
    vlc_player_Unlock(player);

    if (unlikely(p_mi->timer.id == NULL))
        return -1;

    return 0;
}

void
libapoi_media_player_unwatch_time(libapoi_media_player_t *p_mi)
{
    vlc_player_t *player = p_mi->player;

    vlc_player_Lock(player);

    assert(p_mi->timer.id != NULL);
    vlc_player_RemoveTimer(player, p_mi->timer.id);
    p_mi->timer.id = NULL;

    vlc_player_Unlock(player);
}

int
libapoi_media_player_time_point_interpolate(const libapoi_media_player_time_point_t *libpoint,
                                           int64_t system_now_us,
                                           int64_t *out_ts_us, double *out_pos)
{
    const struct vlc_player_timer_point point = PLAYER_TIME_LIB_TO_CORE(libpoint);

    vlc_tick_t out_ts;
    int ret = vlc_player_timer_point_Interpolate(&point,
                                                 VLC_TICK_FROM_US(system_now_us),
                                                 &out_ts, out_pos);
    *out_ts_us = US_FROM_VLC_TICK(out_ts);
    return ret;
}

int64_t
libapoi_media_player_time_point_get_next_date(const libapoi_media_player_time_point_t *libpoint,
                                             int64_t system_now_us,
                                             int64_t interpolated_ts_us,
                                             int64_t next_interval_us)
{
    const struct vlc_player_timer_point point = PLAYER_TIME_LIB_TO_CORE(libpoint);

    vlc_tick_t date =
        vlc_player_timer_point_GetNextIntervalDate(&point,
                                                   VLC_TICK_FROM_US(system_now_us),
                                                   VLC_TICK_FROM_US(interpolated_ts_us),
                                                   VLC_TICK_FROM_US(next_interval_us));
    return US_FROM_VLC_TICK(date);
}

#include <vlc_vout_display.h>

/* make sure surface structures from libapoi can be passed as such to vlc
   otherwise we will need wrappers between what libapoi understands and what vlc uses */
#define cast_  libapoi_video_color_space_t
static_assert(libapoi_video_colorspace_BT601  == (cast_)COLOR_SPACE_BT601 &&
              libapoi_video_colorspace_BT709  == (cast_)COLOR_SPACE_BT709 &&
              libapoi_video_colorspace_BT2020 == (cast_)COLOR_SPACE_BT2020
              , "libapoi video colorspace mismatch");
#undef cast_

#define cast_  libapoi_video_transfer_func_t
static_assert(libapoi_video_transfer_func_LINEAR       == (cast_)TRANSFER_FUNC_LINEAR &&
              libapoi_video_transfer_func_SRGB         == (cast_)TRANSFER_FUNC_SRGB &&
              libapoi_video_transfer_func_BT470_BG     == (cast_)TRANSFER_FUNC_BT470_BG &&
              libapoi_video_transfer_func_BT470_M      == (cast_)TRANSFER_FUNC_BT470_M &&
              libapoi_video_transfer_func_BT709        == (cast_)TRANSFER_FUNC_BT709 &&
              libapoi_video_transfer_func_PQ           == (cast_)TRANSFER_FUNC_SMPTE_ST2084 &&
              libapoi_video_transfer_func_SMPTE_240    == (cast_)TRANSFER_FUNC_SMPTE_240 &&
              libapoi_video_transfer_func_HLG          == (cast_)TRANSFER_FUNC_HLG
              , "libapoi video transfer function mismatch");
#undef cast_

#define cast_  libapoi_video_color_primaries_t
static_assert(libapoi_video_primaries_BT601_525 == (cast_)COLOR_PRIMARIES_BT601_525 &&
              libapoi_video_primaries_BT601_625 == (cast_)COLOR_PRIMARIES_BT601_625 &&
              libapoi_video_primaries_BT709     == (cast_)COLOR_PRIMARIES_BT709 &&
              libapoi_video_primaries_BT2020    == (cast_)COLOR_PRIMARIES_BT2020 &&
              libapoi_video_primaries_DCI_P3    == (cast_)COLOR_PRIMARIES_DCI_P3 &&
              libapoi_video_primaries_BT470_M   == (cast_)COLOR_PRIMARIES_BT470_M
              , "libapoi video color primaries mismatch");
#undef cast_

#define cast_ libapoi_abloop_t
static_assert(libapoi_abloop_none == (cast_) VLC_PLAYER_ABLOOP_NONE &&
              libapoi_abloop_a    == (cast_) VLC_PLAYER_ABLOOP_A &&
              libapoi_abloop_b    == (cast_) VLC_PLAYER_ABLOOP_B
              , "libapoi abloop mismatch");
#undef cast_
