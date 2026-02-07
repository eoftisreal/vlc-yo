/*****************************************************************************
 * libapoi_media.h:  libapoi external API
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

#ifndef VLC_LIBAPOI_MEDIA_H
#define VLC_LIBAPOI_MEDIA_H 1

#include <apoi/libapoi_picture.h>
#include <apoi/libapoi_media_track.h>

# ifdef __cplusplus
extern "C" {
# else
#  include <stdbool.h>
# endif
#include <stddef.h>

/** \defgroup libapoi_media LibAPOI media
 * \ingroup libapoi
 * @ref libapoi_media_t is an abstract representation of a playable media.
 * It consists of a media location and various optional meta data.
 * @{
 * \file
 * LibAPOI media item/descriptor external API
 */

typedef struct libapoi_media_t libapoi_media_t;

/** Meta data types */
typedef enum libapoi_meta_t {
    libapoi_meta_Title,
    libapoi_meta_Artist,
    libapoi_meta_Genre,
    libapoi_meta_Copyright,
    libapoi_meta_Album,
    libapoi_meta_TrackNumber,
    libapoi_meta_Description,
    libapoi_meta_Rating,
    libapoi_meta_Date,
    libapoi_meta_Setting,
    libapoi_meta_URL,
    libapoi_meta_Language,
    libapoi_meta_NowPlaying,
    libapoi_meta_Publisher,
    libapoi_meta_EncodedBy,
    libapoi_meta_ArtworkURL,
    libapoi_meta_TrackID,
    libapoi_meta_TrackTotal,
    libapoi_meta_Director,
    libapoi_meta_Season,
    libapoi_meta_Episode,
    libapoi_meta_ShowName,
    libapoi_meta_Actors,
    libapoi_meta_AlbumArtist,
    libapoi_meta_DiscNumber,
    libapoi_meta_DiscTotal
    /* Add new meta types HERE */
} libapoi_meta_t;

/**
 * libapoi media or media_player state
 */
typedef enum libapoi_state_t
{
    libapoi_NothingSpecial=0,
    libapoi_Opening,
    libapoi_Buffering, /* XXX: Deprecated value. Check the
                       * libapoi_MediaPlayerBuffering event to know the
                       * buffering state of a libapoi_media_player */
    libapoi_Playing,
    libapoi_Paused,
    libapoi_Stopped,
    libapoi_Stopping,
    libapoi_Error
} libapoi_state_t;

enum
{
    libapoi_media_option_trusted = 0x2,
    libapoi_media_option_unique = 0x100
};

typedef struct libapoi_media_stats_t
{
    /* Input */
    uint64_t     i_read_bytes;
    float       f_input_bitrate;

    /* Demux */
    uint64_t     i_demux_read_bytes;
    float       f_demux_bitrate;
    uint64_t     i_demux_corrupted;
    uint64_t     i_demux_discontinuity;

    /* Decoders */
    uint64_t     i_decoded_video;
    uint64_t     i_decoded_audio;

    /* Video Output */
    uint64_t     i_displayed_pictures;
    uint64_t     i_late_pictures;
    uint64_t     i_lost_pictures;

    /* Audio output */
    uint64_t     i_played_abuffers;
    uint64_t     i_lost_abuffers;
} libapoi_media_stats_t;

/**
 * Media type
 *
 * \see libapoi_media_get_type
 */
typedef enum libapoi_media_type_t {
    libapoi_media_type_unknown,
    libapoi_media_type_file,
    libapoi_media_type_directory,
    libapoi_media_type_disc,
    libapoi_media_type_stream,
    libapoi_media_type_playlist,
} libapoi_media_type_t;

/**
 * Parse flags used by libapoi_media_parse_request()
 */
typedef enum libapoi_media_parse_flag_t
{
    /**
     * Parse media if it's a local file
     */
    libapoi_media_parse_local    = 0x01,
    /**
     * Parse media even if it's a network file
     */
    libapoi_media_parse_network  = 0x02,
    /**
     * Force parsing the media even if it would be skipped.
     */
    libapoi_media_parse_forced   = 0x04,
    /**
     * Fetch meta and cover art using local resources
     */
    libapoi_media_fetch_local    = 0x08,
    /**
     * Fetch meta and cover art using network resources
     */
    libapoi_media_fetch_network  = 0x10,
    /**
     * Interact with the user (via libapoi_dialog_cbs) when preparsing this item
     * (and not its sub items). Set this flag in order to receive a callback
     * when the input is asking for credentials.
     */
    libapoi_media_do_interact    = 0x20,
} libapoi_media_parse_flag_t;

/**
 * Parse status used sent by libapoi_media_parse_request() or returned by
 * libapoi_media_get_parsed_status()
 */
typedef enum libapoi_media_parsed_status_t
{
    libapoi_media_parsed_status_none,
    libapoi_media_parsed_status_pending,
    libapoi_media_parsed_status_skipped,
    libapoi_media_parsed_status_failed,
    libapoi_media_parsed_status_timeout,
    libapoi_media_parsed_status_cancelled,
    libapoi_media_parsed_status_done,
} libapoi_media_parsed_status_t;

/**
 * Type of a media slave: subtitle or audio.
 */
typedef enum libapoi_media_slave_type_t
{
    libapoi_media_slave_type_subtitle,
    libapoi_media_slave_type_generic,
    libapoi_media_slave_type_audio = libapoi_media_slave_type_generic,
} libapoi_media_slave_type_t;

/**
 * A slave of a libapoi_media_t
 * \see libapoi_media_slaves_get
 */
typedef struct libapoi_media_slave_t
{
    char *                          psz_uri;
    libapoi_media_slave_type_t       i_type;
    unsigned int                    i_priority;
} libapoi_media_slave_t;

/**
 * Type of stat that can be requested from libapoi_media_get_filestat()
 */
#define libapoi_media_filestat_mtime 0
#define libapoi_media_filestat_size 1

/**
 * Callback prototype to open a custom bitstream input media.
 *
 * The same media item can be opened multiple times. Each time, this callback
 * is invoked. It should allocate and initialize any instance-specific
 * resources, then store them in *datap. The instance resources can be freed
 * in the @ref libapoi_media_close_cb callback.
 *
 * \param opaque private pointer as passed to libapoi_media_new_callbacks()
 * \param datap storage space for a private data pointer [OUT]
 * \param sizep byte length of the bitstream or UINT64_MAX if unknown [OUT]
 *
 * \note For convenience, *datap is initially NULL and *sizep is initially 0.
 *
 * \return 0 on success, non-zero on error. In case of failure, the other
 * callbacks will not be invoked and any value stored in *datap and *sizep is
 * discarded.
 */
typedef int (*libapoi_media_open_cb)(void *opaque, void **datap,
                                    uint64_t *sizep);

/**
 * Callback prototype to read data from a custom bitstream input media.
 *
 * \param opaque private pointer as set by the @ref libapoi_media_open_cb
 *               callback
 * \param buf start address of the buffer to read data into
 * \param len bytes length of the buffer
 *
 * \return strictly positive number of bytes read, 0 on end-of-stream,
 *         or -1 on non-recoverable error
 *
 * \note If no data is immediately available, then the callback should sleep.
 * \warning The application is responsible for avoiding deadlock situations.
 */
typedef ptrdiff_t (*libapoi_media_read_cb)(void *opaque, unsigned char *buf,
                                         size_t len);

/**
 * Callback prototype to seek a custom bitstream input media.
 *
 * \param opaque private pointer as set by the @ref libapoi_media_open_cb
 *               callback
 * \param offset absolute byte offset to seek to
 * \return 0 on success, -1 on error.
 */
typedef int (*libapoi_media_seek_cb)(void *opaque, uint64_t offset);

/**
 * Callback prototype to close a custom bitstream input media.
 *
 * \param opaque private pointer as set by the @ref libapoi_media_open_cb
 *               callback
 */
typedef void (*libapoi_media_close_cb)(void *opaque);

/**
 * Create a media with a certain given media resource location,
 * for instance a valid URL.
 *
 * \note To refer to a local file with this function,
 * the file://... URI syntax <b>must</b> be used (see IETF RFC3986).
 * We recommend using libapoi_media_new_path() instead when dealing with
 * local files.
 *
 * \see libapoi_media_release
 *
 * \param psz_mrl the media location
 * \return the newly created media or NULL on error
 */
LIBAPOI_API libapoi_media_t *libapoi_media_new_location(const char * psz_mrl);

/**
 * Create a media for a certain file path.
 *
 * \see libapoi_media_release
 *
 * \param path local filesystem path
 * \return the newly created media or NULL on error
 */
LIBAPOI_API libapoi_media_t *libapoi_media_new_path(const char *path);

/**
 * Create a media for an already open file descriptor.
 * The file descriptor shall be open for reading (or reading and writing).
 *
 * Regular file descriptors, pipe read descriptors and character device
 * descriptors (including TTYs) are supported on all platforms.
 * Block device descriptors are supported where available.
 * Directory descriptors are supported on systems that provide fdopendir().
 * Sockets are supported on all platforms where they are file descriptors,
 * i.e. all except Windows.
 *
 * \note This library will <b>not</b> automatically close the file descriptor
 * under any circumstance. Nevertheless, a file descriptor can usually only be
 * rendered once in a media player. To render it a second time, the file
 * descriptor should probably be rewound to the beginning with lseek().
 *
 * \see libapoi_media_release
 *
 * \version LibAPOI 1.1.5 and later.
 *
 * \param fd open file descriptor
 * \return the newly created media or NULL on error
 */
LIBAPOI_API libapoi_media_t *libapoi_media_new_fd(int fd);

/**
 * Create a media with custom callbacks to read the data from.
 *
 * \param open_cb callback to open the custom bitstream input media
 * \param read_cb callback to read data (must not be NULL)
 * \param seek_cb callback to seek, or NULL if seeking is not supported
 * \param close_cb callback to close the media, or NULL if unnecessary
 * \param opaque data pointer for the open callback
 *
 * \return the newly created media or NULL on error
 *
 * \note If open_cb is NULL, the opaque pointer will be passed to read_cb,
 * seek_cb and close_cb, and the stream size will be treated as unknown.
 *
 * \note The callbacks may be called asynchronously (from another thread).
 * A single stream instance need not be reentrant. However the open_cb needs to
 * be reentrant if the media is used by multiple player instances.
 *
 * \warning The callbacks may be used until all or any player instances
 * that were supplied the media item are stopped.
 *
 * \see libapoi_media_release
 *
 * \version LibAPOI 3.0.0 and later.
 */
LIBAPOI_API libapoi_media_t *libapoi_media_new_callbacks(
                                   libapoi_media_open_cb open_cb,
                                   libapoi_media_read_cb read_cb,
                                   libapoi_media_seek_cb seek_cb,
                                   libapoi_media_close_cb close_cb,
                                   void *opaque );

/**
 * Create a media as an empty node with a given name.
 *
 * \see libapoi_media_release
 *
 * \param psz_name the name of the node
 * \return the new empty media or NULL on error
 */
LIBAPOI_API libapoi_media_t *libapoi_media_new_as_node(const char * psz_name);

/**
 * Add an option to the media.
 *
 * This option will be used to determine how the media_player will
 * read the media. This allows to use VLC's advanced
 * reading/streaming options on a per-media basis.
 *
 * \note The options are listed in 'vlc --longhelp' from the command line,
 * e.g. "--sout-all". Keep in mind that available options and their semantics
 * vary across LibAPOI versions and builds.
 * \warning Not all options affects libapoi_media_t objects:
 * Specifically, due to architectural issues most audio and video options,
 * such as text renderer options, have no effects on an individual media.
 * These options must be set through libapoi_new() instead.
 *
 * \param p_md the media descriptor
 * \param psz_options the options (as a string)
 */
LIBAPOI_API void libapoi_media_add_option(
                                   libapoi_media_t *p_md,
                                   const char * psz_options );

/**
 * Add an option to the media with configurable flags.
 *
 * This option will be used to determine how the media_player will
 * read the media. This allows to use VLC's advanced
 * reading/streaming options on a per-media basis.
 *
 * The options are detailed in vlc --longhelp, for instance
 * "--sout-all". Note that all options are not usable on medias:
 * specifically, due to architectural issues, video-related options
 * such as text renderer options cannot be set on a single media. They
 * must be set on the whole libapoi instance instead.
 *
 * \param p_md the media descriptor
 * \param psz_options the options (as a string)
 * \param i_flags the flags for this option
 */
LIBAPOI_API void libapoi_media_add_option_flag(
                                   libapoi_media_t *p_md,
                                   const char * psz_options,
                                   unsigned i_flags );


/**
 * Retain a reference to a media descriptor object (libapoi_media_t). Use
 * libapoi_media_release() to decrement the reference count of a
 * media descriptor object.
 *
 * \param p_md the media descriptor
 * \return the same object
 */
LIBAPOI_API libapoi_media_t *libapoi_media_retain( libapoi_media_t *p_md );

/**
 * Decrement the reference count of a media descriptor object. If the
 * reference count is 0, then libapoi_media_release() will release the
 * media descriptor object. If the media descriptor object has been released it
 * should not be used again.
 *
 * \param p_md the media descriptor
 */
LIBAPOI_API void libapoi_media_release( libapoi_media_t *p_md );


/**
 * Get the media resource locator (mrl) from a media descriptor object
 *
 * \param p_md a media descriptor object
 * \return string with mrl of media descriptor object
 */
LIBAPOI_API char *libapoi_media_get_mrl( libapoi_media_t *p_md );

/**
 * Duplicate a media descriptor object.
 *
 * \warning the duplicated media won't share forthcoming updates from the
 * original one.
 *
 * \param p_md a media descriptor object.
 */
LIBAPOI_API libapoi_media_t *libapoi_media_duplicate( libapoi_media_t *p_md );

/**
 * Read the meta of the media.
 *
 * Note, you need to call libapoi_media_parse_request() or play the media
 * at least once before calling this function.
 * If the media has not yet been parsed this will return NULL.
 *
 * \see libapoi_MediaMetaChanged
 *
 * \param p_md the media descriptor
 * \param e_meta the meta to read
 * \return the media's meta
 */
LIBAPOI_API char *libapoi_media_get_meta( libapoi_media_t *p_md,
                                             libapoi_meta_t e_meta );

/**
 * Set the meta of the media (this function will not save the meta, call
 * libapoi_media_save_meta in order to save the meta)
 *
 * \param p_md the media descriptor
 * \param e_meta the meta to write
 * \param psz_value the media's meta
 */
LIBAPOI_API void libapoi_media_set_meta( libapoi_media_t *p_md,
                                           libapoi_meta_t e_meta,
                                           const char *psz_value );

/**
 * Read the meta extra of the media.
 *
 * If the media has not yet been parsed this will return NULL.
 *
 * \see libapoi_media_parse
 * \see libapoi_media_parse_with_options
 *
 * \param p_md the media descriptor
 * \param psz_name the meta extra to read (nonnullable)
 * \return the media's meta extra or NULL
 */
LIBAPOI_API char *libapoi_media_get_meta_extra( libapoi_media_t *p_md,
                                              const char *psz_name );

/**
 * Set the meta of the media (this function will not save the meta, call
 * libapoi_media_save_meta in order to save the meta)
 *
 * \param p_md the media descriptor
 * \param psz_name the meta extra to write (nonnullable)
 * \param psz_value the media's meta extra (nullable)
 * Removed from meta extra if set to NULL
 */
LIBAPOI_API void libapoi_media_set_meta_extra( libapoi_media_t *p_md,
                                             const char *psz_name,
                                             const char *psz_value );

/**
 * Read the meta extra names of the media.
 *
 * \param p_md the media descriptor
 * \param pppsz_names the media's meta extra name array
 * you can access the elements using the return value (count)
 * must be released with libapoi_media_meta_extra_names_release()
 * \return the meta extra count
 */
LIBAPOI_API unsigned libapoi_media_get_meta_extra_names( libapoi_media_t *p_md,
                                                       char ***pppsz_names );

/**
 * Release a media meta extra names
 *
 * \param ppsz_names meta extra names array to release
 * \param i_count number of elements in the array
 */
LIBAPOI_API void libapoi_media_meta_extra_names_release( char **ppsz_names,
                                                       unsigned i_count );

/**
 * Save the meta previously set
 *
 * \param inst LibAPOI instance
 * \param p_md the media descriptor
 * \return true if the write operation was successful
 */
LIBAPOI_API int libapoi_media_save_meta( libapoi_instance_t *inst,
                                       libapoi_media_t *p_md );

/**
 * Get the current statistics about the media
 * \param p_md media descriptor object
 * \param p_stats structure that contain the statistics about the media
 *                 (this structure must be allocated by the caller)
 * \retval true statistics are available
 * \retval false otherwise
 */
LIBAPOI_API bool libapoi_media_get_stats(libapoi_media_t *p_md,
                                       libapoi_media_stats_t *p_stats);

/* The following method uses libapoi_media_list_t, however, media_list usage is optional
 * and this is here for convenience */
#define VLC_FORWARD_DECLARE_OBJECT(a) struct a

/**
 * Get subitems of media descriptor object. This will increment
 * the reference count of supplied media descriptor object. Use
 * libapoi_media_list_release() to decrement the reference counting.
 *
 * \param p_md media descriptor object
 * \return list of media descriptor subitems or NULL
 */
LIBAPOI_API VLC_FORWARD_DECLARE_OBJECT(libapoi_media_list_t *)
libapoi_media_subitems( libapoi_media_t *p_md );

/**
 * Get event manager from media descriptor object.
 * NOTE: this function doesn't increment reference counting.
 *
 * \param p_md a media descriptor object
 * \return event manager object
 */
LIBAPOI_API libapoi_event_manager_t *
    libapoi_media_event_manager( libapoi_media_t *p_md );

/**
 * Get duration (in ms) of media descriptor object item.
 *
 * Note, you need to call libapoi_media_parse_request() or play the media
 * at least once before calling this function.
 * Not doing this will result in an undefined result.
 *
 * \param p_md media descriptor object
 * \return duration of media item or -1 on error
 */
LIBAPOI_API libapoi_time_t
   libapoi_media_get_duration( libapoi_media_t *p_md );

/**
 * Get a 'stat' value of media descriptor object item.
 *
 * \note 'stat' values are currently only parsed by directory accesses. This
 * mean that only sub medias of a directory media, parsed with
 * libapoi_media_parse_request() can have valid 'stat' properties.
 * \version LibAPOI 4.0.0 and later.
 *
 * \param p_md media descriptor object
 * \param type a valid libapoi_media_stat_ define
 * \param out field in which the value will be stored
 * \return 1 on success, 0 if not found, -1 on error.
 */
LIBAPOI_API int
   libapoi_media_get_filestat( libapoi_media_t *p_md, unsigned type, uint64_t *out );

/**
 * Parse the media asynchronously with options.
 *
 * This fetches (local or network) art, meta data and/or tracks information.
 *
 * To track when this is over you can listen to libapoi_MediaParsedChanged
 * event. However if this functions returns an error, you will not receive any
 * events.
 *
 * It uses a flag to specify parse options (see libapoi_media_parse_flag_t). All
 * these flags can be combined. By default, media is parsed if it's a local
 * file.
 *
 * \note Parsing can be aborted with libapoi_media_parse_stop().
 *
 * \see libapoi_MediaParsedChanged
 * \see libapoi_media_get_meta
 * \see libapoi_media_get_tracklist
 * \see libapoi_media_get_parsed_status
 * \see libapoi_media_parse_flag_t
 *
 * \param inst LibAPOI instance that is to parse the media
 * \param p_md media descriptor object
 * \param parse_flag parse options:
 * \param timeout maximum time allowed to preparse the media. If -1, the
 * default "preparse-timeout" option will be used as a timeout. If 0, it will
 * wait indefinitely. If > 0, the timeout will be used (in milliseconds).
 * \return -1 in case of error, 0 otherwise
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API int
libapoi_media_parse_request( libapoi_instance_t *inst, libapoi_media_t *p_md,
                            libapoi_media_parse_flag_t parse_flag,
                            int timeout );

/**
 * Stop the parsing of the media
 *
 * When the media parsing is stopped, the libapoi_MediaParsedChanged event will
 * be sent with the libapoi_media_parsed_status_timeout status.
 *
 * \see libapoi_media_parse_request()
 *
 * \param inst LibAPOI instance that is to cease or give up parsing the media
 * \param p_md media descriptor object
 * \version LibAPOI 3.0.0 or later
 */
LIBAPOI_API void
libapoi_media_parse_stop( libapoi_instance_t *inst, libapoi_media_t *p_md );

/**
 * Get Parsed status for media descriptor object.
 *
 * \see libapoi_MediaParsedChanged
 * \see libapoi_media_parsed_status_t
 * \see libapoi_media_parse_request()
 *
 * \param p_md media descriptor object
 * \return a value of the libapoi_media_parsed_status_t enum
 * \version LibAPOI 3.0.0 or later
 */
LIBAPOI_API libapoi_media_parsed_status_t
   libapoi_media_get_parsed_status( libapoi_media_t *p_md );

/**
 * Sets media descriptor's user_data. user_data is specialized data
 * accessed by the host application, VLC.framework uses it as a pointer to
 * an native object that references a libapoi_media_t pointer
 *
 * \param p_md media descriptor object
 * \param p_new_user_data pointer to user data
 */
LIBAPOI_API void
    libapoi_media_set_user_data( libapoi_media_t *p_md, void *p_new_user_data );

/**
 * Get media descriptor's user_data. user_data is specialized data
 * accessed by the host application, VLC.framework uses it as a pointer to
 * an native object that references a libapoi_media_t pointer
 *
 * \see libapoi_media_set_user_data
 *
 * \param p_md media descriptor object
 */
LIBAPOI_API void *libapoi_media_get_user_data( libapoi_media_t *p_md );

/**
 * Get the track list for one type
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \note You need to call libapoi_media_parse_request() or play the media
 * at least once before calling this function.  Not doing this will result in
 * an empty list.
 *
 * \see libapoi_media_tracklist_count
 * \see libapoi_media_tracklist_at
 *
 * \param p_md media descriptor object
 * \param type type of the track list to request
 *
 * \return a valid libapoi_media_tracklist_t or NULL in case of error, if there
 * is no track for a category, the returned list will have a size of 0, delete
 * with libapoi_media_tracklist_delete()
 */
LIBAPOI_API libapoi_media_tracklist_t *
libapoi_media_get_tracklist( libapoi_media_t *p_md, libapoi_track_type_t type );

/**
 * Get codec description from media elementary stream
 *
 * Note, you need to call libapoi_media_parse_request() or play the media
 * at least once before calling this function.
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \see libapoi_media_track_t
 *
 * \param i_type i_type from libapoi_media_track_t
 * \param i_codec i_codec or i_original_fourcc from libapoi_media_track_t
 *
 * \return codec description
 */
LIBAPOI_API
const char *libapoi_media_get_codec_description( libapoi_track_type_t i_type,
                                                uint32_t i_codec );

/**
 * Get the media type of the media descriptor object
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \see libapoi_media_type_t
 *
 * \param p_md media descriptor object
 *
 * \return media type
 */
LIBAPOI_API
libapoi_media_type_t libapoi_media_get_type( libapoi_media_t *p_md );

/**
 * \brief libapoi_media_thumbnail_request_t An opaque thumbnail request object
 */
typedef struct libapoi_media_thumbnail_request_t libapoi_media_thumbnail_request_t;

typedef enum libapoi_thumbnailer_seek_speed_t
{
    libapoi_media_thumbnail_seek_precise,
    libapoi_media_thumbnail_seek_fast,
} libapoi_thumbnailer_seek_speed_t;

/**
 * \brief libapoi_media_request_thumbnail_by_time Start an asynchronous thumbnail generation
 *
 * If the request is successfully queued, the libapoi_MediaThumbnailGenerated is
 * guaranteed to be emitted (except if the request is destroyed early by the
 * user).
 * The resulting thumbnail size can either be:
 * - Hardcoded by providing both width & height. In which case, the image will
 *   be stretched to match the provided aspect ratio, or cropped if crop is true.
 * - Derived from the media aspect ratio if only width or height is provided and
 *   the other one is set to 0.
 *
 * \param inst LibAPOI instance to generate the thumbnail with
 * \param md media descriptor object
 * \param time The time at which the thumbnail should be generated
 * \param speed The seeking speed \sa{libapoi_thumbnailer_seek_speed_t}
 * \param width The thumbnail width
 * \param height the thumbnail height
 * \param crop Should the picture be cropped to preserve source aspect ratio
 * \param picture_type The thumbnail picture type \sa{libapoi_picture_type_t}
 * \param timeout A timeout value in ms, or 0 to disable timeout
 *
 * \return A valid opaque request object, or NULL in case of failure.
 * It must be released by libapoi_media_thumbnail_request_destroy() and
 * can be cancelled by calling it early.
 *
 * \version libapoi 4.0 or later
 *
 * \see libapoi_picture_t
 * \see libapoi_picture_type_t
 */
LIBAPOI_API libapoi_media_thumbnail_request_t*
libapoi_media_thumbnail_request_by_time( libapoi_instance_t *inst,
                                        libapoi_media_t *md, libapoi_time_t time,
                                        libapoi_thumbnailer_seek_speed_t speed,
                                        unsigned int width, unsigned int height,
                                        bool crop, libapoi_picture_type_t picture_type,
                                        libapoi_time_t timeout );

/**
 * \brief libapoi_media_request_thumbnail_by_pos Start an asynchronous thumbnail generation
 *
 * If the request is successfully queued, the libapoi_MediaThumbnailGenerated is
 * guaranteed to be emitted (except if the request is destroyed early by the
 * user).
 * The resulting thumbnail size can either be:
 * - Hardcoded by providing both width & height. In which case, the image will
 *   be stretched to match the provided aspect ratio, or cropped if crop is true.
 * - Derived from the media aspect ratio if only width or height is provided and
 *   the other one is set to 0.
 *
 * \param inst LibAPOI instance to generate the thumbnail with
 * \param md media descriptor object
 * \param pos The position at which the thumbnail should be generated
 * \param speed The seeking speed \sa{libapoi_thumbnailer_seek_speed_t}
 * \param width The thumbnail width
 * \param height the thumbnail height
 * \param crop Should the picture be cropped to preserve source aspect ratio
 * \param picture_type The thumbnail picture type \sa{libapoi_picture_type_t}
 * \param timeout A timeout value in ms, or 0 to disable timeout
 *
 * \return A valid opaque request object, or NULL in case of failure.
 * It must be released by libapoi_media_thumbnail_request_destroy().
 *
 * \version libapoi 4.0 or later
 *
 * \see libapoi_picture_t
 * \see libapoi_picture_type_t
 */
LIBAPOI_API libapoi_media_thumbnail_request_t*
libapoi_media_thumbnail_request_by_pos( libapoi_instance_t *inst,
                                       libapoi_media_t *md, double pos,
                                       libapoi_thumbnailer_seek_speed_t speed,
                                       unsigned int width, unsigned int height,
                                       bool crop, libapoi_picture_type_t picture_type,
                                       libapoi_time_t timeout );

/**
 * @brief libapoi_media_thumbnail_destroy destroys a thumbnail request
 * @param p_req An opaque thumbnail request object.
 *
 * This will also cancel the thumbnail request, no events will be emitted after
 * this call.
 */
LIBAPOI_API void
libapoi_media_thumbnail_request_destroy( libapoi_media_thumbnail_request_t *p_req );

/**
 * Add a slave to the current media.
 *
 * A slave is an external input source that may contains an additional subtitle
 * track (like a .srt) or an additional audio track (like a .ac3).
 *
 * \note This function must be called before the media is parsed (via
 * libapoi_media_parse_request()) or before the media is played (via
 * libapoi_media_player_play())
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \param p_md media descriptor object
 * \param i_type subtitle or audio
 * \param i_priority from 0 (low priority) to 4 (high priority)
 * \param psz_uri Uri of the slave (should contain a valid scheme).
 *
 * \return 0 on success, -1 on error.
 */
LIBAPOI_API
int libapoi_media_slaves_add( libapoi_media_t *p_md,
                             libapoi_media_slave_type_t i_type,
                             unsigned int i_priority,
                             const char *psz_uri );

/**
 * Clear all slaves previously added by libapoi_media_slaves_add() or
 * internally.
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \param p_md media descriptor object
 */
LIBAPOI_API
void libapoi_media_slaves_clear( libapoi_media_t *p_md );

/**
 * Get a media descriptor's slave list
 *
 * The list will contain slaves parsed by VLC or previously added by
 * libapoi_media_slaves_add(). The typical use case of this function is to save
 * a list of slave in a database for a later use.
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \see libapoi_media_slaves_add
 *
 * \param p_md media descriptor object
 * \param ppp_slaves address to store an allocated array of slaves (must be
 * freed with libapoi_media_slaves_release()) [OUT]
 *
 * \return the number of slaves (zero on error)
 */
LIBAPOI_API
unsigned int libapoi_media_slaves_get( libapoi_media_t *p_md,
                                      libapoi_media_slave_t ***ppp_slaves );

/**
 * Release a media descriptor's slave list
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \param pp_slaves slave array to release
 * \param i_count number of elements in the array
 */
LIBAPOI_API
void libapoi_media_slaves_release( libapoi_media_slave_t **pp_slaves,
                                  unsigned int i_count );

/** @}*/

# ifdef __cplusplus
}
# endif

#endif /* VLC_LIBAPOI_MEDIA_H */
