/*****************************************************************************
 * libapoi_media_player.h:  libapoi_media_player external API
 *****************************************************************************
 * Copyright (C) 1998-2024 VLC authors and VideoLAN
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Jean-Paul Saman <jpsaman@videolan.org>
 *          Pierre d'Herbemont <pdherbemont@videolan.org>
 *          Maxime Chapelet <umxprime at videolabs dot io>
 *          Alexandre Janniaux <ajanni@videolabs.io>
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

#ifndef VLC_LIBAPOI_MEDIA_PLAYER_H
#define VLC_LIBAPOI_MEDIA_PLAYER_H 1

/* Definitions of enum properties for video */
#include "libapoi_video.h"

# ifdef __cplusplus
extern "C" {
# else
#  include <stdbool.h>
# endif

typedef struct libapoi_video_viewpoint_t libapoi_video_viewpoint_t;
typedef enum libapoi_media_slave_type_t libapoi_media_slave_type_t;
typedef struct libapoi_media_t libapoi_media_t;
typedef struct libapoi_media_track_t libapoi_media_track_t;
typedef struct libapoi_media_tracklist_t libapoi_media_tracklist_t;
typedef enum libapoi_track_type_t libapoi_track_type_t;
typedef struct libapoi_renderer_item_t libapoi_renderer_item_t;
typedef enum libapoi_state_t libapoi_state_t;

/** \defgroup libapoi_media_player LibAPOI media player
 * \ingroup libapoi
 * A LibAPOI media player plays one media (usually in a custom drawable).
 * @{
 * \file
 * LibAPOI simple media player external API
 */

typedef struct libapoi_media_player_t libapoi_media_player_t;

/**
 * Description for titles
 */
enum
{
    libapoi_title_menu          = 0x01,
    libapoi_title_interactive   = 0x02
};

typedef struct libapoi_title_description_t
{
    int64_t i_duration; /**< duration in milliseconds */
    char *psz_name; /**< title name */
    unsigned i_flags; /**< info if item was recognized as a menu, interactive or plain content by the demuxer */
} libapoi_title_description_t;

/**
 * Description for chapters
 */
typedef struct libapoi_chapter_description_t
{
    int64_t i_time_offset; /**< time-offset of the chapter in milliseconds */
    int64_t i_duration; /**< duration of the chapter in milliseconds */
    char *psz_name; /**< chapter name */
} libapoi_chapter_description_t;

/**
 * Description for audio output. It contains
 * name, description and pointer to next record.
 */
typedef struct libapoi_audio_output_t
{
    char *psz_name;
    char *psz_description;
    struct libapoi_audio_output_t *p_next;

} libapoi_audio_output_t;

/**
 * Description for audio output device.
 */
typedef struct libapoi_audio_output_device_t
{
    struct libapoi_audio_output_device_t *p_next; /**< Next entry in list */
    char *psz_device; /**< Device identifier string */
    char *psz_description; /**< User-friendly device description */
    /* More fields may be added here in later versions */
} libapoi_audio_output_device_t;

/**
 * Marq options definition
 */
typedef enum libapoi_video_marquee_option_t {
    libapoi_marquee_Enable = 0,
    libapoi_marquee_Text,                  /** string argument */
    libapoi_marquee_Color,
    libapoi_marquee_Opacity,
    libapoi_marquee_Position,
    libapoi_marquee_Refresh,
    libapoi_marquee_Size,
    libapoi_marquee_Timeout,
    libapoi_marquee_X,
    libapoi_marquee_Y
} libapoi_video_marquee_option_t;

/**
 * Navigation mode
 */
typedef enum libapoi_navigate_mode_t
{
    libapoi_navigate_activate = 0,
    libapoi_navigate_up,
    libapoi_navigate_down,
    libapoi_navigate_left,
    libapoi_navigate_right,
    libapoi_navigate_popup
} libapoi_navigate_mode_t;

/**
 * Enumeration of values used to set position (e.g. of video title).
 */
typedef enum libapoi_position_t {
    libapoi_position_disable=-1,
    libapoi_position_center,
    libapoi_position_left,
    libapoi_position_right,
    libapoi_position_top,
    libapoi_position_top_left,
    libapoi_position_top_right,
    libapoi_position_bottom,
    libapoi_position_bottom_left,
    libapoi_position_bottom_right
} libapoi_position_t;

/**
 * Enumeration of values used to set the video fitting inside the display area.
 */
typedef enum libapoi_video_fit_mode_t {
    libapoi_video_fit_none = 0,    /**< Explicit zoom set by \ref libapoi_video_set_scale */
    libapoi_video_fit_smaller, /**< Fit inside / to smallest display dimension */
    libapoi_video_fit_larger,  /**< Fit outside / to largest display dimension */
    libapoi_video_fit_width,   /**< Fit to display width */
    libapoi_video_fit_height,  /**< Fit to display height */
} libapoi_video_fit_mode_t;

/**
 * Enumeration of teletext keys than can be passed via
 * libapoi_video_set_teletext()
 */
typedef enum libapoi_teletext_key_t {
    libapoi_teletext_key_red = 'r' << 16,
    libapoi_teletext_key_green = 'g' << 16,
    libapoi_teletext_key_yellow = 'y' << 16,
    libapoi_teletext_key_blue = 'b' << 16,
    libapoi_teletext_key_index = 'i' << 16,
} libapoi_teletext_key_t;

/**
 * A to B loop state
 */
typedef enum libapoi_abloop_t {
    libapoi_abloop_none,
    libapoi_abloop_a,
    libapoi_abloop_b,
} libapoi_abloop_t;

/**
 * Opaque equalizer handle.
 *
 * Equalizer settings can be applied to a media player.
 */
typedef struct libapoi_equalizer_t libapoi_equalizer_t;

/**
 * Create an empty Media Player object
 *
 * \param p_libapoi_instance the libapoi instance in which the Media Player
 *        should be created.
 * \return a new media player object, or NULL on error.
 * It must be released by libapoi_media_player_release().
 */
LIBAPOI_API libapoi_media_player_t * libapoi_media_player_new( libapoi_instance_t *p_libapoi_instance );

/**
 * Create a Media Player object from a Media
 *
 * \param inst LibAPOI instance to create a media player with
 * \param p_md the media. Afterwards the p_md can be safely
 *        destroyed.
 * \return a new media player object, or NULL on error.
 * It must be released by libapoi_media_player_release().
 */
LIBAPOI_API libapoi_media_player_t * libapoi_media_player_new_from_media( libapoi_instance_t *inst, libapoi_media_t *p_md );

/**
 * Release a media_player after use
 * Decrement the reference count of a media player object. If the
 * reference count is 0, then libapoi_media_player_release() will
 * release the media player object. If the media player object
 * has been released, then it should not be used again.
 *
 * \param p_mi the Media Player to free
 */
LIBAPOI_API void libapoi_media_player_release( libapoi_media_player_t *p_mi );

/**
 * Retain a reference to a media player object. Use
 * libapoi_media_player_release() to decrement reference count.
 *
 * \param p_mi media player object
 * \return the same object
 */
LIBAPOI_API libapoi_media_player_t *libapoi_media_player_retain( libapoi_media_player_t *p_mi );

/**
 * Set the media that will be used by the media_player. If any,
 * previous md will be released.
 *
 * \note The user should listen to the libapoi_MediaPlayerMediaChanged event, to
 * know when the new media is actually used by the player (or to known that the
 * older media is no longer used).
 *
 * \param p_mi the Media Player
 * \param p_md the Media. Afterwards the p_md can be safely
 *        destroyed.
 */
LIBAPOI_API void libapoi_media_player_set_media( libapoi_media_player_t *p_mi,
                                               libapoi_media_t *p_md );

/**
 * Get the media used by the media_player.
 *
 * \warning Calling this function just after libapoi_media_player_set_media()
 * will return the media that was just set, but this media might not be
 * currently used internally by the player. To detect such case, the user
 * should listen to the libapoi_MediaPlayerMediaChanged event.
 *
 * \param p_mi the Media Player
 * \return the media associated with p_mi, or NULL if no
 *         media is associated
 */
LIBAPOI_API libapoi_media_t * libapoi_media_player_get_media( libapoi_media_player_t *p_mi );

/**
 * Get the Event Manager from which the media player send event.
 *
 * \param p_mi the Media Player
 * \return the event manager associated with p_mi
 */
LIBAPOI_API libapoi_event_manager_t * libapoi_media_player_event_manager ( libapoi_media_player_t *p_mi );

/**
 * is_playing
 *
 * \param p_mi the Media Player
 * \retval true media player is playing
 * \retval false media player is not playing
 */
LIBAPOI_API bool libapoi_media_player_is_playing(libapoi_media_player_t *p_mi);

/**
 * Play
 *
 * \param p_mi the Media Player
 * \return 0 if playback started (and was already started), or -1 on error.
 */
LIBAPOI_API int libapoi_media_player_play ( libapoi_media_player_t *p_mi );

/**
 * Pause or resume (no effect if there is no media)
 *
 * \param mp the Media Player
 * \param do_pause play/resume if zero, pause if non-zero
 * \version LibAPOI 1.1.1 or later
 */
LIBAPOI_API void libapoi_media_player_set_pause ( libapoi_media_player_t *mp,
                                                    int do_pause );

/**
 * Toggle pause (no effect if there is no media)
 *
 * \param p_mi the Media Player
 */
LIBAPOI_API void libapoi_media_player_pause ( libapoi_media_player_t *p_mi );

/**
 * Stop asynchronously
 *
 * \note This function is asynchronous. In case of success, the user should
 * wait for the libapoi_MediaPlayerStopped event to know when the stop is
 * finished.
 *
 * \param p_mi the Media Player
 * \return 0 if the player is being stopped, -1 otherwise (no-op)
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API int libapoi_media_player_stop_async ( libapoi_media_player_t *p_mi );

/**
 * Set a renderer to the media player
 *
 * \note must be called before the first call of libapoi_media_player_play() to
 * take effect.
 *
 * \see libapoi_renderer_discoverer_new
 *
 * \param p_mi the Media Player
 * \param p_item an item discovered by libapoi_renderer_discoverer_start()
 * \return 0 on success, -1 on error.
 * \version LibAPOI 3.0.0 or later
 */
LIBAPOI_API int libapoi_media_player_set_renderer( libapoi_media_player_t *p_mi,
                                                 libapoi_renderer_item_t *p_item );

/**
 * Enumeration of the Video color primaries.
 */
typedef enum libapoi_video_color_primaries_t {
    libapoi_video_primaries_BT601_525 = 1,
    libapoi_video_primaries_BT601_625 = 2,
    libapoi_video_primaries_BT709     = 3,
    libapoi_video_primaries_BT2020    = 4,
    libapoi_video_primaries_DCI_P3    = 5,
    libapoi_video_primaries_BT470_M   = 6,
} libapoi_video_color_primaries_t;

/**
 * Enumeration of the Video color spaces.
 */
typedef enum libapoi_video_color_space_t {
    libapoi_video_colorspace_BT601  = 1,
    libapoi_video_colorspace_BT709  = 2,
    libapoi_video_colorspace_BT2020 = 3,
} libapoi_video_color_space_t;

/**
 * Enumeration of the Video transfer functions.
 */
typedef enum libapoi_video_transfer_func_t {
    libapoi_video_transfer_func_LINEAR     = 1,
    libapoi_video_transfer_func_SRGB       = 2,
    libapoi_video_transfer_func_BT470_BG   = 3,
    libapoi_video_transfer_func_BT470_M    = 4,
    libapoi_video_transfer_func_BT709      = 5,
    libapoi_video_transfer_func_PQ         = 6,
    libapoi_video_transfer_func_SMPTE_240  = 7,
    libapoi_video_transfer_func_HLG        = 8,
} libapoi_video_transfer_func_t;


/**
 * Callback prototype to allocate and lock a picture buffer.
 *
 * Whenever a new video frame needs to be decoded, the lock callback is
 * invoked. Depending on the video chroma, one or three pixel planes of
 * adequate dimensions must be returned via the second parameter. Those
 * planes must be aligned on 32-bytes boundaries.
 *
 * \param[in] opaque private pointer as passed to libapoi_video_set_callbacks()
 * \param[out] planes start address of the pixel planes (LibAPOI allocates the array
 *             of void pointers, this callback must initialize the array)
 * \return a private pointer for the display and unlock callbacks to identify
 *         the picture buffers
 */
typedef void *(*libapoi_video_lock_cb)(void *opaque, void **planes);

/**
 * Callback prototype to unlock a picture buffer.
 *
 * When the video frame decoding is complete, the unlock callback is invoked.
 * This callback might not be needed at all. It is only an indication that the
 * application can now read the pixel values if it needs to.
 *
 * \note A picture buffer is unlocked after the picture is decoded,
 * but before the picture is displayed.
 *
 * \param[in] opaque private pointer as passed to libapoi_video_set_callbacks()
 * \param[in] picture private pointer returned from the @ref libapoi_video_lock_cb
 *                callback
 * \param[in] planes pixel planes as defined by the @ref libapoi_video_lock_cb
 *               callback (this parameter is only for convenience)
 */
typedef void (*libapoi_video_unlock_cb)(void *opaque, void *picture,
                                       void *const *planes);

/**
 * Callback prototype to display a picture.
 *
 * When the video frame needs to be shown, as determined by the media playback
 * clock, the display callback is invoked.
 *
 * \param[in] opaque private pointer as passed to libapoi_video_set_callbacks()
 * \param[in] picture private pointer returned from the @ref libapoi_video_lock_cb
 *                callback
 */
typedef void (*libapoi_video_display_cb)(void *opaque, void *picture);

/**
 * Callback prototype to configure picture buffers format.
 * This callback gets the format of the video as output by the video decoder
 * and the chain of video filters (if any). It can opt to change any parameter
 * as it needs. In that case, LibAPOI will attempt to convert the video format
 * (rescaling and chroma conversion) but these operations can be CPU intensive.
 *
 * \param[in,out] opaque pointer to the private pointer passed to
 *                libapoi_video_set_callbacks()
 * \param[in,out] chroma pointer to the 4 bytes video format identifier
 * \param[in,out] width pointer to the buffer width in pixels
 * \param[in,out] height pointer to the buffer height in pixels
 * \param[out] pitches table of scanline pitches in bytes for each pixel plane
 *                (the table is allocated by LibAPOI)
 * \param[out] lines table of scanlines count for each plane
 * \return the number of picture buffers allocated, 0 indicates failure
 *
 * \version LibAPOI 4.0.0 and later.
 * \param[in] width  pointer to display width - 1 in pixels
 * \param[in] height pointer to display height - 1 in pixels
 *
 * \note
 * For each pixels plane, the scanline pitch must be bigger than or equal to
 * the number of bytes per pixel multiplied by the pixel width.
 * Similarly, the number of scanlines must be bigger than of equal to
 * the pixel height.
 * Furthermore, we recommend that pitches and lines be multiple of 32
 * to not break assumptions that might be held by optimized code
 * in the video decoders, video filters and/or video converters.
 */
typedef unsigned (*libapoi_video_format_cb)(void **opaque, char *chroma,
                                           unsigned *width, unsigned *height,
                                           unsigned *pitches,
                                           unsigned *lines);

/**
 * Callback prototype to configure picture buffers format.
 *
 * \param[in] opaque private pointer as passed to libapoi_video_set_format_callbacks()
 *               (and possibly modified by @ref libapoi_video_format_cb)
 */
typedef void (*libapoi_video_cleanup_cb)(void *opaque);


/**
 * Set callbacks and private data to render decoded video to a custom area
 * in memory.
 * Use libapoi_video_set_format() or libapoi_video_set_format_callbacks()
 * to configure the decoded format.
 *
 * \warning Rendering video into custom memory buffers is considerably less
 * efficient than rendering in a custom window as normal.
 *
 * For optimal performances, VLC media player renders into a custom window, and
 * does not use this function and associated callbacks. It is <b>highly
 * recommended</b> that other LibAPOI-based application do likewise.
 * To embed video in a window, use libapoi_media_player_set_xwindow() or
 * equivalent depending on the operating system.
 *
 * If window embedding does not fit the application use case, then a custom
 * LibAPOI video output display plugin is required to maintain optimal video
 * rendering performances.
 *
 * The following limitations affect performance:
 * - Hardware video decoding acceleration will either be disabled completely,
 *   or require (relatively slow) copy from video/DSP memory to main memory.
 * - Sub-pictures (subtitles, on-screen display, etc.) must be blent into the
 *   main picture by the CPU instead of the GPU.
 * - Depending on the video format, pixel format conversion, picture scaling,
 *   cropping and/or picture re-orientation, must be performed by the CPU
 *   instead of the GPU.
 * - Memory copying is required between LibAPOI reference picture buffers and
 *   application buffers (between lock and unlock callbacks).
 *
 * \param mp the media player
 * \param lock callback to lock video memory (must not be NULL)
 * \param unlock callback to unlock video memory (or NULL if not needed)
 * \param display callback to display video (or NULL if not needed)
 * \param opaque private pointer for the three callbacks (as first parameter)
 * \version LibAPOI 1.1.1 or later
 */
LIBAPOI_API
void libapoi_video_set_callbacks( libapoi_media_player_t *mp,
                                 libapoi_video_lock_cb lock,
                                 libapoi_video_unlock_cb unlock,
                                 libapoi_video_display_cb display,
                                 void *opaque );

/**
 * Set decoded video chroma and dimensions.
 * This only works in combination with libapoi_video_set_callbacks(),
 * and is mutually exclusive with libapoi_video_set_format_callbacks().
 *
 * \param mp the media player
 * \param chroma a four-characters string identifying the chroma
 *               (e.g. "RV32" or "YUYV")
 * \param width pixel width
 * \param height pixel height
 * \param pitch line pitch (in bytes)
 * \version LibAPOI 1.1.1 or later
 * \bug All pixel planes are expected to have the same pitch.
 * To use the YCbCr color space with chrominance subsampling,
 * consider using libapoi_video_set_format_callbacks() instead.
 */
LIBAPOI_API
void libapoi_video_set_format( libapoi_media_player_t *mp, const char *chroma,
                              unsigned width, unsigned height,
                              unsigned pitch );

/**
 * Set decoded video chroma and dimensions. This only works in combination with
 * libapoi_video_set_callbacks().
 *
 * \param mp the media player
 * \param setup callback to select the video format (cannot be NULL)
 * \param cleanup callback to release any allocated resources (or NULL)
 * \version LibAPOI 2.0.0 or later
 */
LIBAPOI_API
void libapoi_video_set_format_callbacks( libapoi_media_player_t *mp,
                                        libapoi_video_format_cb setup,
                                        libapoi_video_cleanup_cb cleanup );


typedef struct libapoi_video_setup_device_cfg_t
{
    bool hardware_decoding; /** set if D3D11_CREATE_DEVICE_VIDEO_SUPPORT is needed for D3D11 */
} libapoi_video_setup_device_cfg_t;

typedef struct libapoi_video_setup_device_info_t
{
    union {
        struct {
            void *device_context; /** ID3D11DeviceContext* */
            void *context_mutex; /** Windows Mutex HANDLE to protect ID3D11DeviceContext usage */
        } d3d11;
        struct {
            void *device;         /** IDirect3D9* */
            int  adapter;         /** Adapter to use with the IDirect3D9* */
        } d3d9;
    };
} libapoi_video_setup_device_info_t;

/**
 * Callback prototype called to initialize user data.
 * Setup the rendering environment.
 *
 * \param[in,out] opaque private pointer passed to the @a libapoi_video_set_output_callbacks()
 *               on input. The callback can change this value on output to be
 *               passed to all the other callbacks set on @a libapoi_video_set_output_callbacks().
 * \param[in] cfg requested configuration of the video device
 * \param[out] out libapoi_video_setup_device_info_t* to fill
 * \return true on success
 * \version LibAPOI 4.0.0 or later
 *
 * For \ref libapoi_video_engine_d3d9 the output must be a IDirect3D9*.
 * A reference to this object is held until the \ref libapoi_video_output_cleanup_cb is called.
 * the device must be created with D3DPRESENT_PARAMETERS.hDeviceWindow set to 0.
 *
 * For \ref libapoi_video_engine_d3d11 the output must be a ID3D11DeviceContext*.
 * A reference to this object is held until the \ref libapoi_video_output_cleanup_cb is called.
 * The ID3D11Device used to create ID3D11DeviceContext must have multithreading enabled.
 *
 * If the ID3D11DeviceContext is used outside of the callbacks called by libapoi, the host
 * MUST use a mutex to protect the access to the ID3D11DeviceContext of libapoi. This mutex
 * value is set on d3d11.context_mutex. If the ID3D11DeviceContext is not used outside of
 * the callbacks, the mutex d3d11.context_mutex may be NULL.
 */
typedef bool (*libapoi_video_output_setup_cb)(void **opaque,
                                      const libapoi_video_setup_device_cfg_t *cfg,
                                      libapoi_video_setup_device_info_t *out);


/**
 * Callback prototype called to release user data
 *
 * \param[in] opaque private pointer set on the opaque parameter of @a libapoi_video_output_setup_cb()
 * \version LibAPOI 4.0.0 or later
 */
typedef void (*libapoi_video_output_cleanup_cb)(void* opaque);

typedef struct libapoi_video_render_cfg_t
{
    /** rendering video width in pixel */
    unsigned width;
    /** rendering video height in pixel */
    unsigned height;
    /** rendering video bit depth in bits per channel */
    unsigned bitdepth;
    /** video is full range or studio/limited range */
    bool full_range;
    /** video color space */
    libapoi_video_color_space_t colorspace;
    /** video color primaries */
    libapoi_video_color_primaries_t primaries;
    /** video transfer function */
    libapoi_video_transfer_func_t transfer;
    /** device used for rendering, IDirect3DDevice9* for D3D9 */
    void *device;
} libapoi_video_render_cfg_t;

typedef struct libapoi_video_output_cfg_t
{
    union {
        /** The rendering DXGI_FORMAT for \ref libapoi_video_engine_d3d11. */
        int dxgi_format;
        /** The rendering D3DFORMAT for \ref libapoi_video_engine_d3d9. */
        uint32_t d3d9_format;
        /** The rendering GLint GL_RGBA or GL_RGB for
         * \ref libapoi_video_engine_opengl and for
         * \ref libapoi_video_engine_gles2. */
        int opengl_format;
        /** currently unused */
        void *p_surface;
        struct {
            /** Pointer to an ANativeWindow, used for video rendering */
            void *video;
            /** Pointer to an ANativeWindow, used for subtitles rendering, if
             * blending subtitles into the video surface is not possible (when
             * using MediaCodec with direct hw rendering) */
            void *subtitle;
        } anw;
    };
    /** Video is full range or studio/limited range. */
    bool full_range;
    /** video color space */
    libapoi_video_color_space_t colorspace;
    /** video color primaries */
    libapoi_video_color_primaries_t primaries;
    /** video transfer function */
    libapoi_video_transfer_func_t transfer;
    /** video surface orientation */
    libapoi_video_orient_t orientation;
} libapoi_video_output_cfg_t;

/**
 * Callback prototype called on video size changes.
 * Update the rendering output setup.
 *
 * \param[in] opaque private pointer set on the opaque parameter of @a libapoi_video_output_setup_cb()
 * \param[in] cfg configuration of the video that will be rendered
 * \param[out] output configuration describing with how the rendering is setup
 * \version LibAPOI 4.0.0 or later
 *
 * \note the configuration device for Direct3D9 is the IDirect3DDevice9 that VLC
 *       uses to render. The host must set a Render target and call Present()
 *       when it needs the drawing from VLC to be done. This object is not valid
 *       anymore after Cleanup is called.
 * Tone mapping, range and color conversion will be done depending on the
 * values set in the output structure. It can be ignored in the \ref
 * libapoi_video_engine_anw case.
 */
typedef bool (*libapoi_video_update_output_cb)(void* opaque, const libapoi_video_render_cfg_t *cfg,
                                              libapoi_video_output_cfg_t *output );


/**
 * Callback prototype called after performing drawing calls.
 *
 * This callback is called outside of libapoi_video_makeCurrent_cb current/not-current
 * calls.
 *
 * \param[in] opaque private pointer set on the opaque parameter of @a libapoi_video_output_setup_cb()
 * \version LibAPOI 4.0.0 or later
 */
typedef void (*libapoi_video_swap_cb)(void* opaque);

/**
 * Callback prototype to set up the OpenGL context for rendering.
 * Tell the host the rendering is about to start/has finished.
 *
 * \param[in] opaque private pointer set on the opaque parameter of @a libapoi_video_output_setup_cb()
 * \param[in] enter true to set the context as current, false to unset it
 * \return true on success
 * \version LibAPOI 4.0.0 or later
 *
 * On Direct3D11 the following may change on the provided ID3D11DeviceContext*
 * between \p enter being true and \p enter being false:
 * - IASetPrimitiveTopology()
 * - IASetInputLayout()
 * - IASetVertexBuffers()
 * - IASetIndexBuffer()
 * - VSSetConstantBuffers()
 * - VSSetShader()
 * - PSSetSamplers()
 * - PSSetConstantBuffers()
 * - PSSetShaderResources()
 * - PSSetShader()
 * - RSSetViewports()
 * - DrawIndexed()
 */
typedef bool (*libapoi_video_makeCurrent_cb)(void* opaque, bool enter);

/**
 * Callback prototype to load opengl functions
 *
 * \param[in] opaque private pointer set on the opaque parameter of @a libapoi_video_output_setup_cb()
 * \param fct_name name of the opengl function to load
 * \return a pointer to the named OpenGL function the NULL otherwise
 * \version LibAPOI 4.0.0 or later
 */
typedef void* (*libapoi_video_getProcAddress_cb)(void* opaque, const char* fct_name);

typedef struct libapoi_video_frame_hdr10_metadata_t
{
    /* similar to CTA-861-G with ranges from H265, based on SMPTE ST 2086 mastering display color volume */
    uint16_t RedPrimary[2];   /**< [5,37 000] normalized x / [5,42 000] y chromacity in increments of 0.00002, 0=unknown */
    uint16_t GreenPrimary[2]; /**< [5,37 000] normalized x / [5,42 000] y chromacity in increments of 0.00002, 0=unknown */
    uint16_t BluePrimary[2];  /**< [5,37 000] normalized x / [5,42 000] y chromacity in increments of 0.00002, 0=unknown */
    uint16_t WhitePoint[2];   /**< [5,37 000] normalized x / [5,42 000] y white point in increments of 0.00002, 0=unknown */
    unsigned int MaxMasteringLuminance; /**< [50 000, 100 000 000] maximum luminance in 0.0001 cd/m², 0=unknown */
    unsigned int MinMasteringLuminance; /**< [1, 50 000] minimum luminance in 0.0001 cd/m², 0=unknown */
    uint16_t MaxContentLightLevel;      /**< [1, 50 000] Maximum Content Light Level in cd/m², 0=unknown */
    uint16_t MaxFrameAverageLightLevel; /**< [1, 50 000] Maximum Frame-Average Light Level in cd/m², 0=unknown */
} libapoi_video_frame_hdr10_metadata_t;

typedef enum libapoi_video_metadata_type_t {
    libapoi_video_metadata_frame_hdr10, /**< libapoi_video_frame_hdr10_metadata_t */
} libapoi_video_metadata_type_t;

/**
 * Callback prototype to receive metadata before rendering.
 *
 * \param[in] opaque private pointer passed to the @a libapoi_video_set_output_callbacks()
 * \param[in] type type of data passed in metadata
 * \param[in] metadata the type of metadata
 * \version LibAPOI 4.0.0 or later
 */
typedef void (*libapoi_video_frameMetadata_cb)(void* opaque, libapoi_video_metadata_type_t type, const void *metadata);

/**
 * Enumeration of the Video engine to be used on output.
 * can be passed to @a libapoi_video_set_output_callbacks
 */
typedef enum libapoi_video_engine_t {
    /** Disable rendering engine */
    libapoi_video_engine_disable,
    libapoi_video_engine_opengl,
    libapoi_video_engine_gles2,
    /** Direct3D11 rendering engine */
    libapoi_video_engine_d3d11,
    /** Direct3D9 rendering engine */
    libapoi_video_engine_d3d9,

    /**
     * Android ANativeWindow. It can be set in \ref libapoi_video_output_cfg_t
     * from the \ref libapoi_video_update_output_cb callback. The ANativeWindow
     * can be created via:
     *  - 'ANativeWindow_fromSurface': from a JAVA SurfaceView
     *  - 'AImageReader_getWindow()': from an 'AImageReader' created with the
     *  following arguments: \verbatim
     AImageReader_newWithUsage(1, 1 AIMAGE_FORMAT_PRIVATE,
                               AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
                               maxImages, &reader);
     \endverbatim
     * The width and height from \ref libapoi_video_render_cfg_t should be
     * ignored as the video size is overridden by the producer (MediaCodec or
     * EGL vout).
     */
    libapoi_video_engine_anw,
} libapoi_video_engine_t;


/** Callback type that can be called to request a render size changes.
 *
 * libapoi will provide a callback of this type when calling \ref libapoi_video_output_set_window_cb.
 *
 * \param report_opaque parameter passed to \ref libapoi_video_output_set_window_cb. [IN]
 * \param width new rendering width requested. [IN]
 * \param height new rendering height requested. [IN]
 */
typedef void( *libapoi_video_output_resize_cb )( void *report_opaque, unsigned width, unsigned height );


/**
 * Enumeration of the different mouse buttons that can be reported for user interaction
 * can be passed to \ref libapoi_video_output_mouse_press_cb and \ref libapoi_video_output_mouse_release_cb.
 */
typedef enum libapoi_video_output_mouse_button_t {
    libapoi_video_output_mouse_button_left = 0,
    libapoi_video_output_mouse_button_middle = 1,
    libapoi_video_output_mouse_button_right = 2
} libapoi_video_output_mouse_button_t;


/** Callback type that can be called to notify the mouse position when hovering the render surface.
 *
 * libapoi will provide a callback of this type when calling \ref libapoi_video_output_set_window_cb.
 *
 * The position (0,0) denotes the top left corner, bottom right corner position
 * is (width,height) as reported by \ref libapoi_video_output_resize_cb.
 *
 * \param opaque parameter passed to \ref libapoi_video_output_set_window_cb. [IN]
 * \param x horizontal mouse position in \ref libapoi_video_output_resize_cb coordinates. [IN]
 * \param y vertical mouse position in \ref libapoi_video_output_resize_cb coordinates. [IN]
 */
typedef void (*libapoi_video_output_mouse_move_cb)(void *opaque, int x, int y);

/** Callback type that can be called to notify when a mouse button is pressed in the rendering surface.
 *
 * libapoi will provide a callback of this type when calling \ref libapoi_video_output_set_window_cb.
 *
 * The button event will be reported at the last position provided by \ref libapoi_video_output_mouse_move_cb
 *
 * \param opaque parameter passed to \ref libapoi_video_output_set_window_cb. [IN]
 * \param button represent the button pressed, see \ref libapoi_video_output_mouse_button_t for available buttons. [IN]
 */
typedef void (*libapoi_video_output_mouse_press_cb)(void *opaque, libapoi_video_output_mouse_button_t button);

/** Callback type that can be called to notify when a mouse button is released in the rendering surface.
 *
 * libapoi will provide a callback of this type when calling \ref libapoi_video_output_set_window_cb.
 *
 * The button event will be reported at the last position provided by \ref libapoi_video_output_mouse_move_cb.
 *
 * \param opaque parameter passed to \ref libapoi_video_output_set_window_cb. [IN]
 * \param button represent the button released, see \ref libapoi_video_output_mouse_button_t for available buttons. [IN]
 */
typedef void (*libapoi_video_output_mouse_release_cb)(void *opaque, libapoi_video_output_mouse_button_t button);

/** Set the callback to call when the host app resizes the rendering area.
 *
 * This allows text rendering and aspect ratio to be handled properly when the host
 * rendering size changes and to provide mouse.
 *
 * It may be called before the \ref libapoi_video_output_setup_cb callback.
 *
 * \warning These callbacks cannot be called concurrently, the caller is responsible for serialization
 *
 * \param[in] opaque private pointer set on the opaque parameter of @a libapoi_video_output_setup_cb()
 * \param[in] report_size_change callback which must be called when the host size changes.
 *            The callback is valid until another call to \ref libapoi_video_output_set_window_cb
 *            is done. This may be called from any thread.
 * \param[in] report_mouse_move callback which must be called when the mouse position change on the video surface.
 *            The coordinates are relative to the size reported through the \p report_size_change.
 *            This may be called from any thread.
 * \param[in] report_mouse_pressed callback which must be called when a mouse button is pressed on the video surface,
 *            The position of the event is the last position reported by the report_mouse_move callback. This may be
 *            called from any thread.
 * \param[in] report_mouse_released callback which must be called when a mouse button is released on the video surface,
 *            The position of the event is the last position reported by the report_mouse_move callback. This may be
 *            called from any thread.
 * \param[in] report_opaque private pointer to pass to the \p report_size_change callback.
 */
typedef void( *libapoi_video_output_set_window_cb )( void *opaque,
                                            libapoi_video_output_resize_cb report_size_change,
                                            libapoi_video_output_mouse_move_cb report_mouse_move,
                                            libapoi_video_output_mouse_press_cb report_mouse_pressed,
                                            libapoi_video_output_mouse_release_cb report_mouse_released,
                                            void *report_opaque );

/** Tell the host the rendering for the given plane is about to start
 *
 * \param[in] opaque private pointer set on the opaque parameter of @a libapoi_video_output_setup_cb()
 * \param plane number of the rendering plane to select
 * \param output handle of the rendering output for the given plane
 * \return true on success
 * \version LibAPOI 4.0.0 or later
 *
 * \note This is only used with \ref libapoi_video_engine_d3d11.
 *
 * The output parameter receives the ID3D11RenderTargetView* to use for rendering
 * the plane.
 *
 * If this callback is not used (set to NULL in @a libapoi_video_set_output_callbacks())
 * OMSetRenderTargets has to be set during the @a libapoi_video_makeCurrent_cb()
 * entering call.
 *
 * The number of planes depend on the DXGI_FORMAT returned during the
 * @a libapoi_video_update_output_cb() call. It's usually one plane except for
 * semi-planar formats like DXGI_FORMAT_NV12 or DXGI_FORMAT_P010.
 *
 * This callback is called between libapoi_video_makeCurrent_cb current/not-current
 * calls.
 */
typedef bool( *libapoi_video_output_select_plane_cb )( void *opaque, size_t plane, void *output );

/**
 * Set callbacks and data to render decoded video to a custom texture
 *
 * \warning VLC will perform video rendering in its own thread and at its own rate,
 * You need to provide your own synchronisation mechanism.
 *
 * \param mp the media player
 * \param engine the GPU engine to use
 * \param setup_cb callback called to initialize user data
 * \param cleanup_cb callback called to clean up user data
 * \param window_cb callback called to setup the window
 * \param update_output_cb callback to get the rendering format of the host (cannot be NULL)
 * \param swap_cb callback called after rendering a video frame (can only be
 * NULL when using \ref libapoi_video_engine_anw)
 * \param makeCurrent_cb callback called to enter/leave the rendering context
 * (can only be NULL when using \ref libapoi_video_engine_anw)
 * \param getProcAddress_cb opengl function loading callback (cannot be NULL
 * for \ref libapoi_video_engine_opengl and for \ref libapoi_video_engine_gles2)
 * \param metadata_cb callback to provide frame metadata (D3D11 only)
 * \param select_plane_cb callback to select different D3D11 rendering targets
 * \param opaque private pointer passed to callbacks
 *
 * \note the \p setup_cb and \p cleanup_cb may be called more than once per
 * playback.
 *
 * \retval true engine selected and callbacks set
 * \retval false engine type unknown, callbacks not set
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API
bool libapoi_video_set_output_callbacks( libapoi_media_player_t *mp,
                                        libapoi_video_engine_t engine,
                                        libapoi_video_output_setup_cb setup_cb,
                                        libapoi_video_output_cleanup_cb cleanup_cb,
                                        libapoi_video_output_set_window_cb window_cb,
                                        libapoi_video_update_output_cb update_output_cb,
                                        libapoi_video_swap_cb swap_cb,
                                        libapoi_video_makeCurrent_cb makeCurrent_cb,
                                        libapoi_video_getProcAddress_cb getProcAddress_cb,
                                        libapoi_video_frameMetadata_cb metadata_cb,
                                        libapoi_video_output_select_plane_cb select_plane_cb,
                                        void* opaque );

/**
 * Helper to setup output_callbacks for \ref libapoi_video_engine_anw
 */
static inline bool
libapoi_video_set_anw_callbacks( libapoi_media_player_t *mp,
                                libapoi_video_output_setup_cb setup_cb,
                                libapoi_video_output_cleanup_cb cleanup_cb,
                                libapoi_video_update_output_cb update_output_cb,
                                void *opaque )
{
    return libapoi_video_set_output_callbacks( mp, libapoi_video_engine_anw,
                                              setup_cb, cleanup_cb, NULL,
                                              update_output_cb, NULL, NULL,
                                              NULL, NULL, NULL, opaque );
}

/**
 * Set the handler where the media player should display its video output.
 *
 * The drawable is an `NSObject` that require responding to two selectors
 * like in this protocol:
 *
 * @code{.m}
 * @protocol VLCDrawable <NSObject>
 * - (void)addSubview:(VLCView *)view;
 * - (CGRect)bounds;
 * @end
 * @endcode
 *
 * In this protocol `VLCView` type can either be a `UIView` or a `NSView` type
 * class.
 * VLCDrawable protocol conformance isn't mandatory but a drawable must respond
 * to both `addSubview:` and `bounds` selectors.
 *
 * Additionally, a drawable can also conform to the `VLCPictureInPictureDrawable`
 * protocol to allow picture in picture support :
 *
 * @code{.m}
 * @protocol VLCPictureInPictureMediaControlling <NSObject>
 * - (void)play;
 * - (void)pause;
 * - (void)seekBy:(int64_t)offset completion:(dispatch_block_t)completion;;
 * - (int64_t)mediaLength;
 * - (int64_t)mediaTime;
 * - (BOOL)isMediaSeekable;
 * - (BOOL)isMediaPlaying;
 * @end
 *
 * @protocol VLCPictureInPictureWindowControlling <NSObject>
 * - (void)startPictureInPicture;
 * - (void)stopPictureInPicture;
 * - (void)invalidatePlaybackState;
 * @end
 *
 * @protocol VLCPictureInPictureDrawable <NSObject>
 * - (id<VLCPictureInPictureMediaControlling>) mediaController;
 * - (void (^)(id<VLCPictureInPictureWindowControlling>)) pictureInPictureReady;
 * @end
 * @endcode
 *
 * Be aware that full `VLCPictureInPictureDrawable` conformance is mandatory to
 * enable picture in picture support and that time values in
 * `VLCPictureInPictureMediaControlling` methods are expressed in milliseconds.
 *
 * If you want to use it along with Qt see the QMacCocoaViewContainer. Then
 * the following code should work:
 * @code{.mm}
 * {
 *     NSView *video = [[NSView alloc] init];
 *     QMacCocoaViewContainer *container = new QMacCocoaViewContainer(video, parent);
 *     libapoi_media_player_set_nsobject(mp, video);
 *     [video release];
 * }
 * @endcode
 *
 * You can find a live example in VLCVideoView in VLCKit.framework.
 *
 * \param p_mi the Media Player
 * \param drawable the drawable that is either an NSView, a UIView or any
 * NSObject responding to `addSubview:` and `bounds` selectors
 */
LIBAPOI_API void libapoi_media_player_set_nsobject ( libapoi_media_player_t *p_mi, void * drawable );

/**
 * Get the NSView handler previously set with libapoi_media_player_set_nsobject().
 *
 * \param p_mi the Media Player
 * \return the NSView handler or 0 if none where set
 */
LIBAPOI_API void * libapoi_media_player_get_nsobject ( libapoi_media_player_t *p_mi );

/**
 * Set an X Window System drawable where the media player should render its
 * video output. The call takes effect when the playback starts. If it is
 * already started, it might need to be stopped before changes apply.
 * If LibAPOI was built without X11 output support, then this function has no
 * effects.
 *
 * By default, LibAPOI will capture input events on the video rendering area.
 * Use libapoi_video_set_mouse_input() and libapoi_video_set_key_input() to
 * disable that and deliver events to the parent window / to the application
 * instead. By design, the X11 protocol delivers input events to only one
 * recipient.
 *
 * \warning
 * The application must call the XInitThreads() function from Xlib before
 * libapoi_new(), and before any call to XOpenDisplay() directly or via any
 * other library. Failure to call XInitThreads() will seriously impede LibAPOI
 * performance. Calling XOpenDisplay() before XInitThreads() will eventually
 * crash the process. That is a limitation of Xlib.
 *
 * \param p_mi media player
 * \param drawable X11 window ID
 *
 * \note
 * The specified identifier must correspond to an existing Input/Output class
 * X11 window. Pixmaps are <b>not</b> currently supported. The default X11
 * server is assumed, i.e. that specified in the DISPLAY environment variable.
 *
 * \warning
 * LibAPOI can deal with invalid X11 handle errors, however some display drivers
 * (EGL, GLX, VA and/or VDPAU) can unfortunately not. Thus the window handle
 * must remain valid until playback is stopped, otherwise the process may
 * abort or crash.
 *
 * \bug
 * No more than one window handle per media player instance can be specified.
 * If the media has multiple simultaneously active video tracks, extra tracks
 * will be rendered into external windows beyond the control of the
 * application.
 */
LIBAPOI_API void libapoi_media_player_set_xwindow(libapoi_media_player_t *p_mi,
                                                uint32_t drawable);

/**
 * Get the X Window System window identifier previously set with
 * libapoi_media_player_set_xwindow(). Note that this will return the identifier
 * even if VLC is not currently using it (for instance if it is playing an
 * audio-only input).
 *
 * \param p_mi the Media Player
 * \return an X window ID, or 0 if none where set.
 */
LIBAPOI_API uint32_t libapoi_media_player_get_xwindow ( libapoi_media_player_t *p_mi );

/**
 * Set a Win32/Win64 API window handle (HWND) where the media player should
 * render its video output. If LibAPOI was built without Win32/Win64 API output
 * support, then this has no effects.
 *
 * \warning the HWND must have the WS_CLIPCHILDREN set in its style.
 *
 * \param p_mi the Media Player
 * \param drawable windows handle of the drawable
 */
LIBAPOI_API void libapoi_media_player_set_hwnd ( libapoi_media_player_t *p_mi, void *drawable );

/**
 * Get the Windows API window handle (HWND) previously set with
 * libapoi_media_player_set_hwnd(). The handle will be returned even if LibAPOI
 * is not currently outputting any video to it.
 *
 * \param p_mi the Media Player
 * \return a window handle or NULL if there are none.
 */
LIBAPOI_API void *libapoi_media_player_get_hwnd ( libapoi_media_player_t *p_mi );

/**
 * Set the android context.
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \param p_mi the media player
 * \param p_awindow_handler org.videolan.libapoi.AWindow jobject owned by the
 *        org.videolan.libapoi.MediaPlayer class from the libapoi-android project.
 */
LIBAPOI_API void libapoi_media_player_set_android_context( libapoi_media_player_t *p_mi,
                                                         void *p_awindow_handler );

/**
 * Callback prototype for audio playback.
 *
 * The LibAPOI media player decodes and post-processes the audio signal
 * asynchronously (in an internal thread). Whenever audio samples are ready
 * to be queued to the output, this callback is invoked.
 *
 * The number of samples provided per invocation may depend on the file format,
 * the audio coding algorithm, the decoder plug-in, the post-processing
 * filters and timing. Application must not assume a certain number of samples.
 *
 * The exact format of audio samples is determined by libapoi_audio_set_format()
 * or libapoi_audio_set_format_callbacks() as is the channels layout.
 *
 * Note that the number of samples is per channel. For instance, if the audio
 * track sampling rate is 48000 Hz, then 1200 samples represent 25 milliseconds
 * of audio signal - regardless of the number of audio channels.
 *
 * \param[in] data data pointer as passed to libapoi_audio_set_callbacks()
 * \param[in] samples pointer to a table of audio samples to play back
 * \param count number of audio samples to play back
 * \param pts expected play time stamp (see libapoi_delay())
 */
typedef void (*libapoi_audio_play_cb)(void *data, const void *samples,
                                     unsigned count, int64_t pts);

/**
 * Callback prototype for audio pause.
 *
 * LibAPOI invokes this callback to pause audio playback.
 *
 * \note The pause callback is never called if the audio is already paused.
 * \param[in] data data pointer as passed to libapoi_audio_set_callbacks()
 * \param pts time stamp of the pause request (should be elapsed already)
 */
typedef void (*libapoi_audio_pause_cb)(void *data, int64_t pts);

/**
 * Callback prototype for audio resumption.
 *
 * LibAPOI invokes this callback to resume audio playback after it was
 * previously paused.
 *
 * \note The resume callback is never called if the audio is not paused.
 * \param[in] data data pointer as passed to libapoi_audio_set_callbacks()
 * \param pts time stamp of the resumption request (should be elapsed already)
 */
typedef void (*libapoi_audio_resume_cb)(void *data, int64_t pts);

/**
 * Callback prototype for audio buffer flush.
 *
 * LibAPOI invokes this callback if it needs to discard all pending buffers and
 * stop playback as soon as possible. This typically occurs when the media is
 * stopped.
 *
 * \param[in] data data pointer as passed to libapoi_audio_set_callbacks()
 */
typedef void (*libapoi_audio_flush_cb)(void *data, int64_t pts);

/**
 * Callback prototype for audio buffer drain.
 *
 * LibAPOI may invoke this callback when the decoded audio track is ending.
 * There will be no further decoded samples for the track, but playback should
 * nevertheless continue until all already pending buffers are rendered.
 *
 * \param[in] data data pointer as passed to libapoi_audio_set_callbacks()
 */
typedef void (*libapoi_audio_drain_cb)(void *data);

/**
 * Callback prototype for audio volume change.
 * \param[in] data data pointer as passed to libapoi_audio_set_callbacks()
 * \param volume software volume (1. = nominal, 0. = mute)
 * \param mute muted flag
 */
typedef void (*libapoi_audio_set_volume_cb)(void *data,
                                           float volume, bool mute);

/**
 * Sets callbacks and private data for decoded audio.
 *
 * Use libapoi_audio_set_format() or libapoi_audio_set_format_callbacks()
 * to configure the decoded audio format.
 *
 * \note The audio callbacks override any other audio output mechanism.
 * If the callbacks are set, LibAPOI will <b>not</b> output audio in any way.
 *
 * \param mp the media player
 * \param play callback to play audio samples (must not be NULL)
 * \param pause callback to pause playback (or NULL to ignore)
 * \param resume callback to resume playback (or NULL to ignore)
 * \param flush callback to flush audio buffers (or NULL to ignore)
 * \param drain callback to drain audio buffers (or NULL to ignore)
 * \param opaque private pointer for the audio callbacks (as first parameter)
 * \version LibAPOI 2.0.0 or later
 */
LIBAPOI_API
void libapoi_audio_set_callbacks( libapoi_media_player_t *mp,
                                 libapoi_audio_play_cb play,
                                 libapoi_audio_pause_cb pause,
                                 libapoi_audio_resume_cb resume,
                                 libapoi_audio_flush_cb flush,
                                 libapoi_audio_drain_cb drain,
                                 void *opaque );

/**
 * Set callbacks and private data for decoded audio. This only works in
 * combination with libapoi_audio_set_callbacks().
 * Use libapoi_audio_set_format() or libapoi_audio_set_format_callbacks()
 * to configure the decoded audio format.
 *
 * \param mp the media player
 * \param set_volume callback to apply audio volume,
 *                   or NULL to apply volume in software
 * \version LibAPOI 2.0.0 or later
 */
LIBAPOI_API
void libapoi_audio_set_volume_callback( libapoi_media_player_t *mp,
                                       libapoi_audio_set_volume_cb set_volume );

/**
 * Callback prototype to setup the audio playback.
 *
 * This is called when the media player needs to create a new audio output.
 * \param[in,out] opaque pointer to the data pointer passed to
 *               libapoi_audio_set_callbacks()
 * \param[in,out] format 4 bytes sample format
 * \param[in,out] rate sample rate
 * \param[in,out] channels channels count
 * \return 0 on success, anything else to skip audio playback
 */
typedef int (*libapoi_audio_setup_cb)(void **opaque, char *format, unsigned *rate,
                                     unsigned *channels);

/**
 * Callback prototype for audio playback cleanup.
 *
 * This is called when the media player no longer needs an audio output.
 * \param[in] opaque data pointer as passed to libapoi_audio_set_callbacks()
 */
typedef void (*libapoi_audio_cleanup_cb)(void *opaque);

/**
 * Sets decoded audio format via callbacks.
 *
 * This only works in combination with libapoi_audio_set_callbacks().
 *
 * \param mp the media player
 * \param setup callback to select the audio format (cannot be NULL)
 * \param cleanup callback to release any allocated resources (or NULL)
 * \version LibAPOI 2.0.0 or later
 */
LIBAPOI_API
void libapoi_audio_set_format_callbacks( libapoi_media_player_t *mp,
                                        libapoi_audio_setup_cb setup,
                                        libapoi_audio_cleanup_cb cleanup );

/**
 * Sets a fixed decoded audio format.
 *
 * This only works in combination with libapoi_audio_set_callbacks(),
 * and is mutually exclusive with libapoi_audio_set_format_callbacks().
 *
 * The supported formats are:
 * - "S16N" for signed 16-bit PCM
 * - "S32N" for signed 32-bit PCM
 * - "FL32" for single precision IEEE 754
 *
 * All supported formats use the native endianness.
 * If there are more than one channel, samples are interleaved.
 *
 * \param mp the media player
 * \param format a four-characters string identifying the sample format
 * \param rate sample rate (expressed in Hz)
 * \param channels channels count
 * \version LibAPOI 2.0.0 or later
 */
LIBAPOI_API
void libapoi_audio_set_format( libapoi_media_player_t *mp, const char *format,
                              unsigned rate, unsigned channels );

/** \bug This might go away ... to be replaced by a broader system */

/**
 * Get the current movie length (in ms).
 *
 * \param p_mi the Media Player
 * \return the movie length (in ms), or -1 if there is no media.
 */
LIBAPOI_API libapoi_time_t libapoi_media_player_get_length( libapoi_media_player_t *p_mi );

/**
 * Get the current movie time (in ms).
 *
 * \param p_mi the Media Player
 * \return the movie time (in ms), or -1 if there is no media.
 */
LIBAPOI_API libapoi_time_t libapoi_media_player_get_time( libapoi_media_player_t *p_mi );

/**
 * Set the movie time (in ms).
 *
 * This has no effect if no media is being played.
 * Not all formats and protocols support this.
 *
 * \param p_mi the Media Player
 * \param i_time the movie time (in ms).
 * \param b_fast prefer fast seeking or precise seeking
 * \return 0 on success, -1 on error
 */
LIBAPOI_API int libapoi_media_player_set_time( libapoi_media_player_t *p_mi,
                                             libapoi_time_t i_time, bool b_fast );

/**
 * Jump the movie time (in ms).
 *
 * This will trigger a precise and relative seek (from the current time).
 * This has no effect if no media is being played.
 * Not all formats and protocols support this.
 *
 * \param p_mi the Media Player
 * \param i_time the movie time (in ms).
 * \return 0 on success, -1 on error
 * \version LibAPOI 4.0.0 and later.
 */
LIBAPOI_API int libapoi_media_player_jump_time( libapoi_media_player_t *p_mi,
                                              libapoi_time_t i_time );

/**
 * Get movie position as percentage between 0.0 and 1.0.
 *
 * \param p_mi the Media Player
 * \return movie position, or -1. in case of error
 */
LIBAPOI_API double libapoi_media_player_get_position( libapoi_media_player_t *p_mi );

/**
 * Set movie position as percentage between 0.0 and 1.0.
 * This has no effect if playback is not enabled.
 * This might not work depending on the underlying input format and protocol.
 *
 * \param p_mi the Media Player
 * \param b_fast prefer fast seeking or precise seeking
 * \param f_pos the position
 * \return 0 on success, -1 on error
 */
LIBAPOI_API int libapoi_media_player_set_position( libapoi_media_player_t *p_mi,
                                                 double f_pos, bool b_fast );

/**
 * Enable A to B loop for the current media by setting the start time and end
 * time
 *
 * The B time must be higher than the A time.
 *
 * \param p_mi the Media Player
 * \param a_time start time for the loop (in ms)
 * \param b_time end time for the loop (in ms)
 * \return 0 on success, -1 on error
 * \version LibAPOI 4.0.0 and later.
 */
LIBAPOI_API int
libapoi_media_player_set_abloop_time( libapoi_media_player_t *p_mi,
                                     libapoi_time_t a_time, libapoi_time_t b_time );

/**
 * Enable A to B loop for the current media by setting the start position and
 * end position
 *
 * The B position must be higher than the A position.
 *
 * \param p_mi the Media Player
 * \param a_pos start position for the loop
 * \param b_pos end position for the loop
 * \return 0 on success, -1 on error
 * \version LibAPOI 4.0.0 and later.
 */
LIBAPOI_API int
libapoi_media_player_set_abloop_position( libapoi_media_player_t *p_mi,
                                         double a_pos, double b_pos );

/**
 * Reset/remove the A to B loop for the current media
 *
 * \param p_mi the Media Player
 * \return 0 on success, -1 on error
 * \version LibAPOI 4.0.0 and later.
 */
LIBAPOI_API int
libapoi_media_player_reset_abloop( libapoi_media_player_t *p_mi );

/**
 * Get the A to B loop status
 *
 * @note If the returned status is VLC_PLAYER_ABLOOP_A, then a_time and a_pos
 * will be valid. If the returned status is VLC_PLAYER_ABLOOP_B, then all
 * output parameters are valid. If the returned status is
 * VLC_PLAYER_ABLOOP_NONE, then all output parameters are invalid.
 *
 * @see vlc_player_cbs.on_atobloop_changed
 *
 * \param p_mi the Media Player
 * \param a_time A time (in ms) or -1 (if the media doesn't have valid times)
 * \param a_pos A position
 * \param b_time B time (in ms) or -1 (if the media doesn't have valid times)
 * \param b_pos B position
 * \return A to B loop status
 * \version LibAPOI 4.0.0 and later.
 */
LIBAPOI_API libapoi_abloop_t
libapoi_media_player_get_abloop( libapoi_media_player_t *p_mi,
                                libapoi_time_t *a_time, double *a_pos,
                                libapoi_time_t *b_time, double *b_pos );
/**
 * Set movie chapter (if applicable).
 *
 * \param p_mi the Media Player
 * \param i_chapter chapter number to play
 */
LIBAPOI_API void libapoi_media_player_set_chapter( libapoi_media_player_t *p_mi, int i_chapter );

/**
 * Get movie chapter.
 *
 * \param p_mi the Media Player
 * \return chapter number currently playing, or -1 if there is no media.
 */
LIBAPOI_API int libapoi_media_player_get_chapter( libapoi_media_player_t *p_mi );

/**
 * Get movie chapter count
 *
 * \param p_mi the Media Player
 * \return number of chapters in movie, or -1.
 */
LIBAPOI_API int libapoi_media_player_get_chapter_count( libapoi_media_player_t *p_mi );

/**
 * Get title chapter count
 *
 * \param p_mi the Media Player
 * \param i_title title
 * \return number of chapters in title, or -1
 */
LIBAPOI_API int libapoi_media_player_get_chapter_count_for_title(
                       libapoi_media_player_t *p_mi, int i_title );

/**
 * Set movie title
 *
 * \param p_mi the Media Player
 * \param i_title title number to play
 */
LIBAPOI_API void libapoi_media_player_set_title( libapoi_media_player_t *p_mi, int i_title );

/**
 * Get movie title
 *
 * \param p_mi the Media Player
 * \return title number currently playing, or -1
 */
LIBAPOI_API int libapoi_media_player_get_title( libapoi_media_player_t *p_mi );

/**
 * Get movie title count
 *
 * \param p_mi the Media Player
 * \return title number count, or -1
 */
LIBAPOI_API int libapoi_media_player_get_title_count( libapoi_media_player_t *p_mi );

/**
 * Set previous chapter (if applicable)
 *
 * \param p_mi the Media Player
 */
LIBAPOI_API void libapoi_media_player_previous_chapter( libapoi_media_player_t *p_mi );

/**
 * Set next chapter (if applicable)
 *
 * \param p_mi the Media Player
 */
LIBAPOI_API void libapoi_media_player_next_chapter( libapoi_media_player_t *p_mi );

/**
 * Get the requested movie play rate.
 * @warning Depending on the underlying media, the requested rate may be
 * different from the real playback rate.
 *
 * \param p_mi the Media Player
 * \return movie play rate
 */
LIBAPOI_API float libapoi_media_player_get_rate( libapoi_media_player_t *p_mi );

/**
 * Set movie play rate
 *
 * \param p_mi the Media Player
 * \param rate movie play rate to set
 * \return -1 if an error was detected, 0 otherwise (but even then, it might
 * not actually work depending on the underlying media protocol)
 */
LIBAPOI_API int libapoi_media_player_set_rate( libapoi_media_player_t *p_mi, float rate );

/**
 * Get current movie state
 *
 * \param p_mi the Media Player
 * \return the current state of the media player (playing, paused, ...) \see libapoi_state_t
 */
LIBAPOI_API libapoi_state_t libapoi_media_player_get_state( libapoi_media_player_t *p_mi );

/**
 * How many video outputs does this media player have?
 *
 * \param p_mi the media player
 * \return the number of video outputs
 */
LIBAPOI_API unsigned libapoi_media_player_has_vout( libapoi_media_player_t *p_mi );

/**
 * Is this media player seekable?
 *
 * \param p_mi the media player
 * \retval true media player can seek
 * \retval false media player cannot seek
 */
LIBAPOI_API bool libapoi_media_player_is_seekable(libapoi_media_player_t *p_mi);

/**
 * Can this media player be paused?
 *
 * \param p_mi the media player
 * \retval true media player can be paused
 * \retval false media player cannot be paused
 */
LIBAPOI_API bool libapoi_media_player_can_pause(libapoi_media_player_t *p_mi);

/**
 * Check if the current program is scrambled
 *
 * \param p_mi the media player
 * \retval true current program is scrambled
 * \retval false current program is not scrambled
 *
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API bool libapoi_media_player_program_scrambled( libapoi_media_player_t *p_mi );

/**
 * Display the next frame (if supported)
 *
 * \param p_mi the media player
 */
LIBAPOI_API void libapoi_media_player_next_frame( libapoi_media_player_t *p_mi );

/**
 * Navigate through DVD Menu
 *
 * \param p_mi the Media Player
 * \param navigate the Navigation mode
 * \version libVLC 2.0.0 or later
 */
LIBAPOI_API void libapoi_media_player_navigate( libapoi_media_player_t* p_mi,
                                              unsigned navigate );

/**
 * Set if, and how, the video title will be shown when media is played.
 *
 * \param p_mi the media player
 * \param position position at which to display the title, or libapoi_position_disable to prevent the title from being displayed
 * \param timeout title display timeout in milliseconds (ignored if libapoi_position_disable)
 * \version libVLC 2.1.0 or later
 */
LIBAPOI_API void libapoi_media_player_set_video_title_display( libapoi_media_player_t *p_mi, libapoi_position_t position, unsigned int timeout );

/**
 * Get the track list for one type
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \note You need to call libapoi_media_parse_request() or play the media
 * at least once before calling this function.  Not doing this will result in
 * an empty list.
 *
 * \note This track list is a snapshot of the current tracks when this function
 * is called. If a track is updated after this call, the user will need to call
 * this function again to get the updated track.
 *
 *
 * The track list can be used to get track information and to select specific
 * tracks.
 *
 * \param p_mi the media player
 * \param type type of the track list to request
 * \param selected filter only selected tracks if true (return all tracks, even
 * selected ones if false)
 *
 * \return a valid libapoi_media_tracklist_t or NULL in case of error, if there
 * is no track for a category, the returned list will have a size of 0, delete
 * with libapoi_media_tracklist_delete()
 */
LIBAPOI_API libapoi_media_tracklist_t *
libapoi_media_player_get_tracklist( libapoi_media_player_t *p_mi,
                                   libapoi_track_type_t type, bool selected );

/**
 * Get the selected track for one type
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \warning More than one tracks can be selected for one type. In that case,
 * libapoi_media_player_get_tracklist() should be used.
 *
 * \param p_mi the media player
 * \param type type of the selected track
 *
 * \return a valid track or NULL if there is no selected tracks for this type,
 * release it with libapoi_media_track_release().
 */
LIBAPOI_API libapoi_media_track_t *
libapoi_media_player_get_selected_track( libapoi_media_player_t *p_mi,
                                        libapoi_track_type_t type );

/*
 * Get a track from a track id
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * This function can be used to get the last updated information of a track.
 *
 * \param p_mi the media player
 * \param psz_id valid string representing a track id (cf. psz_id from \ref
 * libapoi_media_track_t)
 *
 * \return a valid track or NULL if there is currently no tracks identified by
 * the string id, release it with libapoi_media_track_release().
 */
LIBAPOI_API libapoi_media_track_t *
libapoi_media_player_get_track_from_id( libapoi_media_player_t *p_mi,
                                       const char *psz_id );


/**
 * Select a track
 *
 * This will unselected the current track.
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \note Use libapoi_media_player_select_tracks() for multiple selection
 *
 * \warning Only use a \ref libapoi_media_track_t retrieved with \ref libapoi_media_player_get_tracklist
 *
 * \param p_mi the media player
 * \param track track to select, can't be NULL
 */
LIBAPOI_API void
libapoi_media_player_select_track( libapoi_media_player_t *p_mi,
                                  const libapoi_media_track_t *track );

/**
 * Unselect all tracks for a given type
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \param p_mi the media player
 * \param type type to unselect
 */
LIBAPOI_API void
libapoi_media_player_unselect_track_type( libapoi_media_player_t *p_mi,
                                         libapoi_track_type_t type );

/**
 * Select multiple tracks for one type
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \note The internal track list can change between the calls of
 * libapoi_media_player_get_tracklist() and
 * libapoi_media_player_set_tracks(). If a track selection change but the
 * track is not present anymore, the player will just ignore it.
 *
 * \note selecting multiple audio tracks is currently not supported.
 *
 * \warning Only use a \ref libapoi_media_track_t retrieved with \ref libapoi_media_player_get_tracklist
 *
 * \param p_mi the media player
 * \param type type of the selected track
 * \param tracks pointer to the track array, or NULL if track_count is 0
 * \param track_count number of tracks in the track array
 */
LIBAPOI_API void
libapoi_media_player_select_tracks( libapoi_media_player_t *p_mi,
                                   libapoi_track_type_t type,
                                   const libapoi_media_track_t **tracks,
                                   size_t track_count );

/**
 * Select tracks by their string identifier
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * This function can be used pre-select a list of tracks before starting the
 * player. It has only effect for the current media. It can also be used when
 * the player is already started.
 *
 * 'str_ids' can contain more than one track id, delimited with ','. "" or any
 * invalid track id will cause the player to unselect all tracks of that
 * category. NULL will disable the preference for newer tracks without
 * unselecting any current tracks.
 *
 * Example:
 * - (libapoi_track_video, "video/1,video/2") will select these 2 video tracks.
 * If there is only one video track with the id "video/0", no tracks will be
 * selected.
 * - (libapoi_track_type_t, "${slave_url_md5sum}/spu/0) will select one spu
 * added by an input slave with the corresponding url.
 *
 * \note The string identifier of a track can be found via psz_id from \ref
 * libapoi_media_track_t
 *
 * \note selecting multiple audio tracks is currently not supported.
 *
 * \warning Only use a \ref libapoi_media_track_t id retrieved with \ref libapoi_media_player_get_tracklist
 *
 * \param p_mi the media player
 * \param type type to select
 * \param psz_ids list of string identifier or NULL
 */
LIBAPOI_API void
libapoi_media_player_select_tracks_by_ids( libapoi_media_player_t *p_mi,
                                          libapoi_track_type_t type,
                                          const char *psz_ids );

/**
 * Add a slave to the current media player.
 *
 * \note If the player is playing, the slave will be added directly. This call
 * will also update the slave list of the attached libapoi_media_t.
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \see libapoi_media_slaves_add
 *
 * \param p_mi the media player
 * \param i_type subtitle or audio
 * \param psz_uri Uri of the slave (should contain a valid scheme).
 * \param b_select True if this slave should be selected when it's loaded
 *
 * \return 0 on success, -1 on error.
 */
LIBAPOI_API
int libapoi_media_player_add_slave( libapoi_media_player_t *p_mi,
                                   libapoi_media_slave_type_t i_type,
                                   const char *psz_uri, bool b_select );

typedef struct libapoi_player_program_t
{
    /** Id used for libapoi_media_player_select_program() */
    int i_group_id;
    /** Program name, always valid */
    char *psz_name;
    /** True if the program is selected */
    bool b_selected;
    /** True if the program is scrambled */
    bool b_scrambled;
} libapoi_player_program_t;

/**
 * Opaque struct containing a list of program
 */
typedef struct libapoi_player_programlist_t libapoi_player_programlist_t;

/**
 * Delete a program struct
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \param program returned by libapoi_media_player_get_selected_program() or
 * libapoi_media_player_get_program_from_id()
 *
 */
LIBAPOI_API void
libapoi_player_program_delete( libapoi_player_program_t *program );

/**
 * Get the number of programs in a programlist
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \param list valid programlist
 *
 * \return number of programs, or 0 if the list is empty
 */
LIBAPOI_API size_t
libapoi_player_programlist_count( const libapoi_player_programlist_t *list );

/**
 * Get a program at a specific index
 *
 * \warning The behaviour is undefined if the index is not valid.
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \param list valid programlist
 * \param index valid index in the range [0; count[
 *
 * \return a valid program (can't be NULL if libapoi_player_programlist_count()
 * returned a valid count)
 */
LIBAPOI_API libapoi_player_program_t *
libapoi_player_programlist_at( libapoi_player_programlist_t *list, size_t index );

/**
 * Release a programlist
 *
 * \note program structs from the list are also deleted.
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \see libapoi_media_player_get_programlist
 *
 * \param list valid programlist
 */
LIBAPOI_API void
libapoi_player_programlist_delete( libapoi_player_programlist_t *list );

/**
 * Select program with a given program id.
 *
 * \note program ids are sent via the libapoi_MediaPlayerProgramAdded event or
 * can be fetch via libapoi_media_player_get_programlist()
 *
 * \version LibAPOI 4.0.0 or later
 *
 * \param p_mi opaque media player handle
 * \param i_group_id program id
 */
LIBAPOI_API void libapoi_media_player_select_program_id( libapoi_media_player_t *p_mi, int i_group_id);

/**
 * Get the selected program
 *
 * \version LibAPOI 4.0.0 or later
 *
 * \param p_mi opaque media player handle
 *
 * \return a valid program struct or NULL if no programs are selected. The
 * program need to be freed with libapoi_player_program_delete().
 */
LIBAPOI_API libapoi_player_program_t *
libapoi_media_player_get_selected_program( libapoi_media_player_t *p_mi);

/**
 * Get a program struct from a program id
 *
 * \version LibAPOI 4.0.0 or later
 *
 * \param p_mi opaque media player handle
 * \param i_group_id program id
 *
 * \return a valid program struct or NULL if the i_group_id is not found. The
 * program need to be freed with libapoi_player_program_delete().
 */
LIBAPOI_API libapoi_player_program_t *
libapoi_media_player_get_program_from_id( libapoi_media_player_t *p_mi, int i_group_id );

/**
 * Get the program list
 *
 * \version LibAPOI 4.0.0 and later.
 * \note This program list is a snapshot of the current programs when this
 * function is called. If a program is updated after this call, the user will
 * need to call this function again to get the updated program.
 *
 * The program list can be used to get program information and to select
 * specific programs.
 *
 * \param p_mi the media player
 *
 * \return a valid libapoi_media_programlist_t or NULL in case of error or empty
 * list, delete with libapoi_media_programlist_delete()
 */
LIBAPOI_API libapoi_player_programlist_t *
libapoi_media_player_get_programlist( libapoi_media_player_t *p_mi );


/** \defgroup libapoi_video LibAPOI video controls
 * @{
 */

/**
 * Toggle fullscreen status on non-embedded video outputs.
 *
 * @warning The same limitations applies to this function
 * as to libapoi_set_fullscreen().
 *
 * \param p_mi the media player
 */
LIBAPOI_API void libapoi_toggle_fullscreen( libapoi_media_player_t *p_mi );

/**
 * Enable or disable fullscreen.
 *
 * @warning With most window managers, only a top-level windows can be in
 * full-screen mode. Hence, this function will not operate properly if
 * libapoi_media_player_set_xwindow() was used to embed the video in a
 * non-top-level window. In that case, the embedding window must be reparented
 * to the root window <b>before</b> fullscreen mode is enabled. You will want
 * to reparent it back to its normal parent when disabling fullscreen.
 *
 * \note This setting applies to any and all current or future active video
 * tracks and windows for the given media player. The choice of fullscreen
 * output for each window is left to the operating system.
 *
 * \param p_mi the media player
 * \param b_fullscreen boolean for fullscreen status
 */
LIBAPOI_API void libapoi_set_fullscreen(libapoi_media_player_t *p_mi, bool b_fullscreen);

/**
 * Get current fullscreen status.
 *
 * \param p_mi the media player
 * \return the fullscreen status (boolean)
 *
 * \retval false media player is windowed
 * \retval true media player is in fullscreen mode
 */
LIBAPOI_API bool libapoi_get_fullscreen( libapoi_media_player_t *p_mi );

/**
 * Enable or disable key press events handling, according to the LibAPOI hotkeys
 * configuration. By default and for historical reasons, keyboard events are
 * handled by the LibAPOI video widget.
 *
 * \note On X11, there can be only one subscriber for key press and mouse
 * click events per window. If your application has subscribed to those events
 * for the X window ID of the video widget, then LibAPOI will not be able to
 * handle key presses and mouse clicks in any case.
 *
 * \warning This function is only implemented for X11 and Win32 at the moment.
 *
 * \param p_mi the media player
 * \param on true to handle key press events, false to ignore them.
 */
LIBAPOI_API
void libapoi_video_set_key_input( libapoi_media_player_t *p_mi, unsigned on );

/**
 * Enable or disable mouse click events handling. By default, those events are
 * handled. This is needed for DVD menus to work, as well as a few video
 * filters such as "puzzle".
 *
 * \see libapoi_video_set_key_input().
 *
 * \warning This function is only implemented for X11 and Win32 at the moment.
 *
 * \param p_mi the media player
 * \param on true to handle mouse click events, false to ignore them.
 */
LIBAPOI_API
void libapoi_video_set_mouse_input( libapoi_media_player_t *p_mi, unsigned on );

/**
 * Get the pixel dimensions of a video.
 *
 * \param p_mi media player
 * \param num number of the video (starting from, and most commonly 0)
 * \param[out] px pointer to get the pixel width
 * \param[out] py pointer to get the pixel height
 * \return 0 on success, -1 if the specified video does not exist
 */
LIBAPOI_API
int libapoi_video_get_size( libapoi_media_player_t *p_mi, unsigned num,
                           unsigned *px, unsigned *py );

/**
 * Get the mouse pointer coordinates over a video.
 * Coordinates are expressed in terms of the decoded video resolution,
 * <b>not</b> in terms of pixels on the screen/viewport (to get the latter,
 * you can query your windowing system directly).
 *
 * Either of the coordinates may be negative or larger than the corresponding
 * dimension of the video, if the cursor is outside the rendering area.
 *
 * @warning The coordinates may be out-of-date if the pointer is not located
 * on the video rendering area. LibAPOI does not track the pointer if it is
 * outside of the video widget.
 *
 * @note LibAPOI does not support multiple pointers (it does of course support
 * multiple input devices sharing the same pointer) at the moment.
 *
 * \param p_mi media player
 * \param num number of the video (starting from, and most commonly 0)
 * \param[out] px pointer to get the abscissa
 * \param[out] py pointer to get the ordinate
 * \return 0 on success, -1 if the specified video does not exist
 */
LIBAPOI_API
int libapoi_video_get_cursor( libapoi_media_player_t *p_mi, unsigned num,
                             int *px, int *py );

/**
 * Get the current video scaling factor.
 * See also libapoi_video_set_scale().
 *
 * \param p_mi the media player
 * \return the currently configured zoom factor, or 0. if the video is set
 * to fit to the output window/drawable automatically.
 */
LIBAPOI_API float libapoi_video_get_scale( libapoi_media_player_t *p_mi );

/**
 * Set the video scaling factor. That is the ratio of the number of pixels on
 * screen to the number of pixels in the original decoded video in each
 * dimension. Zero is a special value; it will adjust the video to the output
 * window/drawable (in windowed mode) or the entire screen.
 *
 * Note that not all video outputs support scaling.
 *
 * \param p_mi the media player
 * \param f_factor the scaling factor, or zero
 */
LIBAPOI_API void libapoi_video_set_scale( libapoi_media_player_t *p_mi, float f_factor );

/**
 * Get current video aspect ratio.
 *
 * \param p_mi the media player
 * \return the video aspect ratio or NULL if unspecified
 * (the result must be released with free() or libapoi_free()).
 */
LIBAPOI_API char *libapoi_video_get_aspect_ratio( libapoi_media_player_t *p_mi );

/**
 * Set new video aspect ratio.
 *
 * \param p_mi the media player
 * \param psz_aspect new video aspect-ratio, "fill" to fill the window or NULL to reset to source aspect ratio
 * \note Invalid aspect ratios are ignored.
 */
LIBAPOI_API void libapoi_video_set_aspect_ratio( libapoi_media_player_t *p_mi, const char *psz_aspect );

/**
 * Get current video display fit mode.
 *
 * \version LibAPOI 4.0.0 or later
 *
 * \param p_mi the media player
 * \return the video display fit mode.
 */
LIBAPOI_API libapoi_video_fit_mode_t libapoi_video_get_display_fit( libapoi_media_player_t *p_mi );

/**
 * Set new video display fit.
 *
 * \version LibAPOI 4.0.0 or later
 *
 * \param p_mi the media player
 * \param fit new display fit mode
 * \note Invalid fit mode are ignored.
 */
LIBAPOI_API void libapoi_video_set_display_fit( libapoi_media_player_t *p_mi, libapoi_video_fit_mode_t fit );

/**
 * Create a video viewpoint structure.
 *
 * \version LibAPOI 3.0.0 and later
 *
 * \return video viewpoint or NULL
 *         (the result must be released with free()).
 */
LIBAPOI_API libapoi_video_viewpoint_t *libapoi_video_new_viewpoint(void);

/**
 * Update the video viewpoint information.
 *
 * \note It is safe to call this function before the media player is started.
 *
 * \version LibAPOI 3.0.0 and later
 *
 * \param p_mi the media player
 * \param p_viewpoint video viewpoint allocated via libapoi_video_new_viewpoint()
 * \param b_absolute if true replace the old viewpoint with the new one. If
 * false, increase/decrease it.
 * \return -1 in case of error, 0 otherwise
 *
 * \note the values are set asynchronously, it will be used by the next frame displayed.
 */
LIBAPOI_API int libapoi_video_update_viewpoint( libapoi_media_player_t *p_mi,
                                              const libapoi_video_viewpoint_t *p_viewpoint,
                                              bool b_absolute);

/**
 * Video stereo modes
 */
typedef enum libapoi_video_stereo_mode_t {
    libapoi_VideoStereoAuto = 0,
    libapoi_VideoStereoStereo,
    libapoi_VideoStereoLeftEye,
    libapoi_VideoStereoRightEye,
    libapoi_VideoStereoSideBySide,
} libapoi_video_stereo_mode_t;

/**
 * Get current video stereo mode.
 *
 * \param p_mi the media player
 * \return the video stereo mode.
 */
LIBAPOI_API libapoi_video_stereo_mode_t libapoi_video_get_video_stereo_mode(
                                              libapoi_media_player_t *p_mi );

/**
 * Set new video stereo mode.
 *
 * \param p_mi the media player
 * \param i_mode new video stereo mode
 */
LIBAPOI_API void libapoi_video_set_video_stereo_mode( libapoi_media_player_t *p_mi,
                                      const libapoi_video_stereo_mode_t i_mode );

/**
 * Get the current subtitle delay. Positive values means subtitles are being
 * displayed later, negative values earlier.
 *
 * \param p_mi media player
 * \return time (in microseconds) the display of subtitles is being delayed
 * \version LibAPOI 2.0.0 or later
 */
LIBAPOI_API int64_t libapoi_video_get_spu_delay( libapoi_media_player_t *p_mi );

/**
 * Get the current subtitle text scale
 *
 * The scale factor is expressed as a percentage of the default size, where
 * 1.0 represents 100 percent.
 *
 * \param p_mi media player
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API float libapoi_video_get_spu_text_scale( libapoi_media_player_t *p_mi );

/**
 * Set the subtitle text scale.
 *
 * The scale factor is expressed as a percentage of the default size, where
 * 1.0 represents 100 percent.
 *
 * A value of 0.5 would result in text half the normal size, and a value of 2.0
 * would result in text twice the normal size.
 *
 * The minimum acceptable value for the scale factor is 0.1.
 *
 * The maximum is 5.0 (five times normal size).
 *
 * \param p_mi media player
 * \param f_scale scale factor in the range [0.1;5.0] (default: 1.0)
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API void libapoi_video_set_spu_text_scale( libapoi_media_player_t *p_mi, float f_scale );

/**
 * Set the subtitle delay. This affects the timing of when the subtitle will
 * be displayed. Positive values result in subtitles being displayed later,
 * while negative values will result in subtitles being displayed earlier.
 *
 * The subtitle delay will be reset to zero each time the media changes.
 *
 * \param p_mi media player
 * \param i_delay time (in microseconds) the display of subtitles should be delayed
 * \return 0 on success, -1 on error
 * \version LibAPOI 2.0.0 or later
 */
LIBAPOI_API int libapoi_video_set_spu_delay( libapoi_media_player_t *p_mi, int64_t i_delay );

/**
 * Get the full description of available titles
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \param p_mi the media player
 * \param[out] titles address to store an allocated array of title descriptions
 *             descriptions (must be freed with libapoi_title_descriptions_release()
 *             by the caller)
 *
 * \return the number of titles (-1 on error)
 */
LIBAPOI_API int libapoi_media_player_get_full_title_descriptions( libapoi_media_player_t *p_mi,
                                                                libapoi_title_description_t ***titles );

/**
 * Release a title description
 *
 * \version LibAPOI 3.0.0 and later
 *
 * \param p_titles title description array to release
 * \param i_count number of title descriptions to release
 */
LIBAPOI_API
    void libapoi_title_descriptions_release( libapoi_title_description_t **p_titles,
                                            unsigned i_count );

/**
 * Get the full description of available chapters
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \param p_mi the media player
 * \param i_chapters_of_title index of the title to query for chapters (uses current title if set to -1)
 * \param[out] pp_chapters address to store an allocated array of chapter descriptions
 *             descriptions (must be freed with libapoi_chapter_descriptions_release()
 *             by the caller)
 *
 * \return the number of chapters (-1 on error)
 */
LIBAPOI_API int libapoi_media_player_get_full_chapter_descriptions( libapoi_media_player_t *p_mi,
                                                                  int i_chapters_of_title,
                                                                  libapoi_chapter_description_t *** pp_chapters );

/**
 * Release a chapter description
 *
 * \version LibAPOI 3.0.0 and later
 *
 * \param p_chapters chapter description array to release
 * \param i_count number of chapter descriptions to release
 */
LIBAPOI_API
void libapoi_chapter_descriptions_release( libapoi_chapter_description_t **p_chapters,
                                          unsigned i_count );

/**
 * Set/unset the video crop ratio.
 *
 * This function forces a crop ratio on any and all video tracks rendered by
 * the media player. If the display aspect ratio of a video does not match the
 * crop ratio, either the top and bottom, or the left and right of the video
 * will be cut out to fit the crop ratio.
 *
 * For instance, a ratio of 1:1 will force the video to a square shape.
 *
 * To disable video crop, set a crop ratio with zero as denominator.
 *
 * A call to this function overrides any previous call to any of
 * libapoi_video_set_crop_ratio(), libapoi_video_set_crop_border() and/or
 * libapoi_video_set_crop_window().
 *
 * \see libapoi_video_set_aspect_ratio()
 *
 * \param mp the media player
 * \param num crop ratio numerator (ignored if denominator is 0)
 * \param den crop ratio denominator (or 0 to unset the crop ratio)
 *
 * \version LibAPOI 4.0.0 and later
 */
LIBAPOI_API
void libapoi_video_set_crop_ratio(libapoi_media_player_t *mp,
                                 unsigned num, unsigned den);

/**
 * Set the video crop window.
 *
 * This function selects a sub-rectangle of video to show. Any pixels outside
 * the rectangle will not be shown.
 *
 * To unset the video crop window, use libapoi_video_set_crop_ratio() or
 * libapoi_video_set_crop_border().
 *
 * A call to this function overrides any previous call to any of
 * libapoi_video_set_crop_ratio(), libapoi_video_set_crop_border() and/or
 * libapoi_video_set_crop_window().
 *
 * \param mp the media player
 * \param x abscissa (i.e. leftmost sample column offset) of the crop window
 * \param y ordinate (i.e. topmost sample row offset) of the crop window
 * \param width sample width of the crop window (cannot be zero)
 * \param height sample height of the crop window (cannot be zero)
 *
 * \version LibAPOI 4.0.0 and later
 */
LIBAPOI_API
void libapoi_video_set_crop_window(libapoi_media_player_t *mp,
                                  unsigned x, unsigned y,
                                  unsigned width, unsigned height);

/**
 * Set the video crop borders.
 *
 * This function selects the size of video edges to be cropped out.
 *
 * To unset the video crop borders, set all borders to zero.
 *
 * A call to this function overrides any previous call to any of
 * libapoi_video_set_crop_ratio(), libapoi_video_set_crop_border() and/or
 * libapoi_video_set_crop_window().
 *
 * \param mp the media player
 * \param left number of sample columns to crop on the left
 * \param right number of sample columns to crop on the right
 * \param top number of sample rows to crop on the top
 * \param bottom number of sample rows to corp on the bottom
 *
 * \version LibAPOI 4.0.0 and later
 */
LIBAPOI_API
void libapoi_video_set_crop_border(libapoi_media_player_t *mp,
                                  unsigned left, unsigned right,
                                  unsigned top, unsigned bottom);

/**
 * Get current teletext page requested or 0 if it's disabled.
 *
 * Teletext is disabled by default, call libapoi_video_set_teletext() to enable
 * it.
 *
 * \param p_mi the media player
 * \return the current teletext page requested.
 */
LIBAPOI_API int libapoi_video_get_teletext( libapoi_media_player_t *p_mi );

/**
 * Set new teletext page to retrieve.
 *
 * This function can also be used to send a teletext key.
 *
 * \param p_mi the media player
 * \param i_page teletex page number requested. This value can be 0 to disable
 * teletext, a number in the range ]0;1000[ to show the requested page, or a
 * \ref libapoi_teletext_key_t. 100 is the default teletext page.
 */
LIBAPOI_API void libapoi_video_set_teletext( libapoi_media_player_t *p_mi, int i_page );

/**
 * Set teletext background transparency.
 *
 * \param p_mi the media player
 * \param transparent whether background should be transparent.
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API void libapoi_video_set_teletext_transparency( libapoi_media_player_t *p_mi, bool transparent );

/**
 * Get teletext background transparency.
 *
 * \param p_mi the media player
 * \retval true teletext has transparent background
 * \retval false teletext has opaque background
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API bool libapoi_video_get_teletext_transparency( libapoi_media_player_t *p_mi );

/**
 * Take a snapshot of the current video window.
 *
 * If i_width AND i_height is 0, original size is used.
 * If i_width XOR i_height is 0, original aspect-ratio is preserved.
 *
 * \param p_mi media player instance
 * \param num number of video output (typically 0 for the first/only one)
 * \param psz_filepath the path of a file or a folder to save the screenshot into
 * \param i_width the snapshot's width
 * \param i_height the snapshot's height
 * \return 0 on success, -1 if the video was not found
 */
LIBAPOI_API
int libapoi_video_take_snapshot( libapoi_media_player_t *p_mi, unsigned num,
                                const char *psz_filepath, unsigned int i_width,
                                unsigned int i_height );

/**
 * Gets the deinterlacing parameters.
 *
 * If \p modep is not NULL, it will be set to a heap-allocated nul-terminated
 * character string indicating the current deinterlacing algorithm name.
 * If no algorithm is selected or if allocation fails, it be set to NULL.
 * The value should be freed with the C run-time's free() function to avoid
 * leaking.
 *
 * \param mpi media player instance
 * \param modep storage space for hold the mode name (or NULL) [OUT]
 * \retval -1 deinterlacing is selected automatically
 * \retval 0 deinterlacing is forcefully disabled
 * \retval 1 deinterlacing is forcefully enabled
 */
LIBAPOI_API int libapoi_video_get_deinterlace(libapoi_media_player_t *mp,
                                            char **modep);

/**
 * Enable or disable deinterlace filter
 *
 * \param p_mi libapoi media player
 * \param deinterlace state -1: auto (default), 0: disabled, 1: enabled
 * \param psz_mode type of deinterlace filter, NULL for current/default filter
 * \version LibAPOI 4.0.0 and later
 * \return 0 on success, -1 if the mode was not recognised
 */
LIBAPOI_API int libapoi_video_set_deinterlace( libapoi_media_player_t *p_mi,
                                              int deinterlace,
                                              const char *psz_mode );

/**
 * Get an integer marquee option value
 *
 * \param p_mi libapoi media player
 * \param option marq option to get \see libapoi_video_marquee_option_t
 */
LIBAPOI_API int libapoi_video_get_marquee_int( libapoi_media_player_t *p_mi,
                                                 unsigned option );

/**
 * Enable, disable or set an integer marquee option
 *
 * Setting libapoi_marquee_Enable has the side effect of enabling (arg !0)
 * or disabling (arg 0) the marq filter.
 *
 * \param p_mi libapoi media player
 * \param option marq option to set \see libapoi_video_marquee_option_t
 * \param i_val marq option value
 */
LIBAPOI_API void libapoi_video_set_marquee_int( libapoi_media_player_t *p_mi,
                                                  unsigned option, int i_val );

/**
 * Set a marquee string option
 *
 * \param p_mi libapoi media player
 * \param option marq option to set \see libapoi_video_marquee_option_t
 * \param psz_text marq option value
 */
LIBAPOI_API void libapoi_video_set_marquee_string( libapoi_media_player_t *p_mi,
                                                     unsigned option, const char *psz_text );

/** option values for libapoi_video_{get,set}_logo_{int,string} */
enum libapoi_video_logo_option_t {
    libapoi_logo_enable,
    libapoi_logo_file,           /**< string argument, "file,d,t;file,d,t;..." */
    libapoi_logo_x,
    libapoi_logo_y,
    libapoi_logo_delay,
    libapoi_logo_repeat,
    libapoi_logo_opacity,
    libapoi_logo_position
};

/**
 * Get integer logo option.
 *
 * \param p_mi libapoi media player instance
 * \param option logo option to get, values of libapoi_video_logo_option_t
 */
LIBAPOI_API int libapoi_video_get_logo_int( libapoi_media_player_t *p_mi,
                                              unsigned option );

/**
 * Set logo option as integer. Options that take a different type value
 * are ignored.
 * Passing libapoi_logo_enable as option value has the side effect of
 * starting (arg !0) or stopping (arg 0) the logo filter.
 *
 * \param p_mi libapoi media player instance
 * \param option logo option to set, values of libapoi_video_logo_option_t
 * \param value logo option value
 */
LIBAPOI_API void libapoi_video_set_logo_int( libapoi_media_player_t *p_mi,
                                               unsigned option, int value );

/**
 * Set logo option as string. Options that take a different type value
 * are ignored.
 *
 * \param p_mi libapoi media player instance
 * \param option logo option to set, values of libapoi_video_logo_option_t
 * \param psz_value logo option value
 */
LIBAPOI_API void libapoi_video_set_logo_string( libapoi_media_player_t *p_mi,
                                      unsigned option, const char *psz_value );


/** option values for libapoi_video_{get,set}_adjust_{int,float,bool} */
enum libapoi_video_adjust_option_t {
    libapoi_adjust_Enable = 0,
    libapoi_adjust_Contrast,
    libapoi_adjust_Brightness,
    libapoi_adjust_Hue,
    libapoi_adjust_Saturation,
    libapoi_adjust_Gamma
};

/**
 * Get integer adjust option.
 *
 * \param p_mi libapoi media player instance
 * \param option adjust option to get, values of libapoi_video_adjust_option_t
 * \version LibAPOI 1.1.1 and later.
 */
LIBAPOI_API int libapoi_video_get_adjust_int( libapoi_media_player_t *p_mi,
                                                unsigned option );

/**
 * Set adjust option as integer. Options that take a different type value
 * are ignored.
 * Passing libapoi_adjust_enable as option value has the side effect of
 * starting (arg !0) or stopping (arg 0) the adjust filter.
 *
 * \param p_mi libapoi media player instance
 * \param option adjust option to set, values of libapoi_video_adjust_option_t
 * \param value adjust option value
 * \version LibAPOI 1.1.1 and later.
 */
LIBAPOI_API void libapoi_video_set_adjust_int( libapoi_media_player_t *p_mi,
                                                 unsigned option, int value );

/**
 * Get float adjust option.
 *
 * \param p_mi libapoi media player instance
 * \param option adjust option to get, values of libapoi_video_adjust_option_t
 * \version LibAPOI 1.1.1 and later.
 */
LIBAPOI_API float libapoi_video_get_adjust_float( libapoi_media_player_t *p_mi,
                                                    unsigned option );

/**
 * Set adjust option as float. Options that take a different type value
 * are ignored.
 *
 * \param p_mi libapoi media player instance
 * \param option adjust option to set, values of libapoi_video_adjust_option_t
 * \param value adjust option value
 * \version LibAPOI 1.1.1 and later.
 */
LIBAPOI_API void libapoi_video_set_adjust_float( libapoi_media_player_t *p_mi,
                                                   unsigned option, float value );
/**
 * Change the projection mode used for rendering the source.
 *
 * This changes how the source is mapped to the output w.r.t. 360 playback.
 *
 * \param p_mi libapoi media player instance
 * \param projection_mode the considered projection mode for the source
 * \version LibAPOI 4.0.0 and later.
 */
LIBAPOI_API void
libapoi_video_set_projection_mode(libapoi_media_player_t *player,
                                 libapoi_video_projection_t projection_mode);

/**
 * Remove previously set projection mode.
 *
 * Remove the effects from previous call to libapoi_video_set_projection_mode.
 *
 * \param p_mi libapoi media player instance
 * \version LibAPOI 4.0.0 and later.
 */
LIBAPOI_API void
libapoi_video_unset_projection_mode(libapoi_media_player_t *player);

/** @} video */

/** \defgroup libapoi_audio LibAPOI audio controls
 * @{
 */

/**
 * Audio stereo modes
 */
typedef enum libapoi_audio_output_stereomode_t {
    libapoi_AudioStereoMode_Unset   = 0,
    libapoi_AudioStereoMode_Stereo  = 1,
    libapoi_AudioStereoMode_RStereo = 2,
    libapoi_AudioStereoMode_Left    = 3,
    libapoi_AudioStereoMode_Right   = 4,
    libapoi_AudioStereoMode_Dolbys  = 5,
    libapoi_AudioStereoMode_Mono  = 7,
} libapoi_audio_output_stereomode_t;

/**
 * Audio mix modes
 */
typedef enum libapoi_audio_output_mixmode_t {
    libapoi_AudioMixMode_Unset       = 0,
    libapoi_AudioMixMode_Stereo      = 1,
    libapoi_AudioMixMode_Binaural    = 2,
    libapoi_AudioMixMode_4_0         = 3,
    libapoi_AudioMixMode_5_1         = 4,
    libapoi_AudioMixMode_7_1         = 5,
} libapoi_audio_output_mixmode_t;

/**
 * Gets the list of available audio output modules.
 *
 * \param p_instance libapoi instance
 * \return list of available audio outputs. It must be freed with
*          \see libapoi_audio_output_list_release \see libapoi_audio_output_t .
 *         In case of error, NULL is returned.
 */
LIBAPOI_API libapoi_audio_output_t *
libapoi_audio_output_list_get( libapoi_instance_t *p_instance );

/**
 * Frees the list of available audio output modules.
 *
 * \param p_list list with audio outputs for release
 */
LIBAPOI_API
void libapoi_audio_output_list_release( libapoi_audio_output_t *p_list );

/**
 * Selects an audio output module.
 * \note Any change will take be effect only after playback is stopped and
 * restarted. Audio output cannot be changed while playing.
 *
 * \param p_mi media player
 * \param psz_name name of audio output,
 *               use psz_name of \see libapoi_audio_output_t
 * \return 0 if function succeeded, -1 on error
 */
LIBAPOI_API int libapoi_audio_output_set( libapoi_media_player_t *p_mi,
                                        const char *psz_name );

/**
 * Gets a list of potential audio output devices.
 *
 * See also libapoi_audio_output_device_set().
 *
 * \note Not all audio outputs support enumerating devices.
 * The audio output may be functional even if the list is empty (NULL).
 *
 * \note The list may not be exhaustive.
 *
 * \warning Some audio output devices in the list might not actually work in
 * some circumstances. By default, it is recommended to not specify any
 * explicit audio device.
 *
 * \param mp media player
 * \return A NULL-terminated linked list of potential audio output devices.
 * It must be freed with libapoi_audio_output_device_list_release()
 * \version LibAPOI 2.2.0 or later.
 */
LIBAPOI_API libapoi_audio_output_device_t *
libapoi_audio_output_device_enum( libapoi_media_player_t *mp );

#if defined (__GNUC__) && !defined (__clang__)
__attribute__((unused))
__attribute__((noinline))
__attribute__((error("Use libapoi_audio_output_device_enum() instead")))
static libapoi_audio_output_device_t *
libapoi_audio_output_device_list_get( libapoi_instance_t *p_instance,
                                     const char *aout )
{
    (void) p_instance; (void) aout;
    return NULL;
}
#endif

/**
 * Frees a list of available audio output devices.
 *
 * \param p_list list with audio outputs for release
 * \version LibAPOI 2.1.0 or later.
 */
LIBAPOI_API void libapoi_audio_output_device_list_release(
                                        libapoi_audio_output_device_t *p_list );

/**
 * Configures an explicit audio output device.
 *
 * A list of adequate potential device strings can be obtained with
 * libapoi_audio_output_device_enum().
 *
 * \note This function does not select the specified audio output plugin.
 * libapoi_audio_output_set() is used for that purpose.
 *
 * \warning The syntax for the device parameter depends on the audio output.
 *
 * Some audio output modules require further parameters (e.g. a channels map
 * in the case of ALSA).
 *
 * \version This function originally expected three parameters.
 * The middle parameter was removed from LibAPOI 4.0 onward.
  *
 * \param mp media player
 * \param device_id device identifier string
 *               (see \ref libapoi_audio_output_device_t::psz_device)
 *
 * \return If the change of device was requested successfully, zero is returned
 * (the actual change is asynchronous and not guaranteed to succeed).
 * On error, a non-zero value is returned.
 */
LIBAPOI_API int libapoi_audio_output_device_set( libapoi_media_player_t *mp,
                                                const char *device_id );

/**
 * Get the current audio output device identifier.
 *
 * This complements libapoi_audio_output_device_set().
 *
 * \warning The initial value for the current audio output device identifier
 * may not be set or may be some unknown value. A LibAPOI application should
 * compare this value against the known device identifiers (e.g. those that
 * were previously retrieved by a call to libapoi_audio_output_device_enum) to
 * find the current audio output device.
 *
 * It is possible that the selected audio output device changes (an external
 * change) without a call to libapoi_audio_output_device_set. That may make this
 * method unsuitable to use if a LibAPOI application is attempting to track
 * dynamic audio device changes as they happen.
 *
 * \param mp media player
 * \return the current audio output device identifier
 *         NULL if no device is selected or in case of error
 *         (the result must be released with free()).
 * \version LibAPOI 3.0.0 or later.
 */
LIBAPOI_API char *libapoi_audio_output_device_get( libapoi_media_player_t *mp );

/**
 * Toggle mute status.
 *
 * \param p_mi media player
 * \warning Toggling mute atomically is not always possible: On some platforms,
 * other processes can mute the VLC audio playback stream asynchronously. Thus,
 * there is a small race condition where toggling will not work.
 * See also the limitations of libapoi_audio_set_mute().
 */
LIBAPOI_API void libapoi_audio_toggle_mute( libapoi_media_player_t *p_mi );

/**
 * Get current mute status.
 *
 * \param p_mi media player
 * \return the mute status (boolean) if defined, -1 if undefined/unapplicable
 */
LIBAPOI_API int libapoi_audio_get_mute( libapoi_media_player_t *p_mi );

/**
 * Set mute status.
 *
 * \param p_mi media player
 * \param status If status is true then mute, otherwise unmute
 * \warning This function does not always work. If there are no active audio
 * playback stream, the mute status might not be available. If digital
 * pass-through (S/PDIF, HDMI...) is in use, muting may be unapplicable. Also
 * some audio output plugins do not support muting at all.
 * \note To force silent playback, disable all audio tracks. This is more
 * efficient and reliable than mute.
 */
LIBAPOI_API void libapoi_audio_set_mute( libapoi_media_player_t *p_mi, int status );

/**
 * Get current software audio volume.
 *
 * \param p_mi media player
 * \return the software volume in percents
 * (0 = mute, 100 = nominal / 0dB)
 */
LIBAPOI_API int libapoi_audio_get_volume( libapoi_media_player_t *p_mi );

/**
 * Set current software audio volume.
 *
 * \param p_mi media player
 * \param i_volume the volume in percents (0 = mute, 100 = 0dB)
 * \return 0 if the volume was set, -1 if it was out of range
 */
LIBAPOI_API int libapoi_audio_set_volume( libapoi_media_player_t *p_mi, int i_volume );

/**
 * Get current audio stereo-mode.
 *
 * \param p_mi media player
 * \return the audio stereo-mode, \see libapoi_audio_output_stereomode_t
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API libapoi_audio_output_stereomode_t libapoi_audio_get_stereomode( libapoi_media_player_t *p_mi );

/**
 * Set current audio stereo-mode.
 *
 * \param p_mi media player
 * \param mode the audio stereo-mode, \see libapoi_audio_output_stereomode_t
 * \return 0 on success, -1 on error
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API int libapoi_audio_set_stereomode( libapoi_media_player_t *p_mi,
                                            libapoi_audio_output_stereomode_t mode );

/**
 * Get current audio mix-mode.
 *
 * \param p_mi media player
 * \return the audio mix-mode, \see libapoi_audio_output_mixmode_t
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API libapoi_audio_output_mixmode_t libapoi_audio_get_mixmode( libapoi_media_player_t *p_mi );

/**
 * Set current audio mix-mode.
 *
 * By default (::libapoi_AudioMixMode_Unset), the audio output will keep its
 * original channel configuration (play stereo as stereo, or 5.1 as 5.1). Yet,
 * the OS and Audio API might refuse a channel configuration and asks VLC to
 * adapt (Stereo played as 5.1 or vice-versa).
 *
 * This function allows to force a channel configuration, it will only work if
 * the OS and Audio API accept this configuration (otherwise, it won't have any
 * effects). Here are some examples:
 *  - Play multi-channels (5.1, 7.1...) as stereo (::libapoi_AudioMixMode_Stereo)
 *  - Play Stereo or 5.1 as 7.1 (::libapoi_AudioMixMode_7_1)
 *  - Play multi-channels as stereo with a binaural effect
 *  (::libapoi_AudioMixMode_Binaural). It might be selected automatically if the
 *  OS and Audio API can detect if a headphone is plugged.
 *
 * \param p_mi media player
 * \param mode the audio mix-mode, \see libapoi_audio_output_mixmode_t
 * \return 0 on success, -1 on error
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API int libapoi_audio_set_mixmode( libapoi_media_player_t *p_mi,
                                         libapoi_audio_output_mixmode_t mode );


/**
 * Get current audio delay.
 *
 * \param p_mi media player
 * \return the audio delay (microseconds)
 * \version LibAPOI 1.1.1 or later
 */
LIBAPOI_API int64_t libapoi_audio_get_delay( libapoi_media_player_t *p_mi );

/**
 * Set current audio delay. The audio delay will be reset to zero each time the media changes.
 *
 * \param p_mi media player
 * \param i_delay the audio delay (microseconds)
 * \return 0 on success, -1 on error
 * \version LibAPOI 1.1.1 or later
 */
LIBAPOI_API int libapoi_audio_set_delay( libapoi_media_player_t *p_mi, int64_t i_delay );

/**
 * Get the number of equalizer presets.
 *
 * \return number of presets
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API unsigned libapoi_audio_equalizer_get_preset_count( void );

/**
 * Get the name of a particular equalizer preset.
 *
 * This name can be used, for example, to prepare a preset label or menu in a user
 * interface.
 *
 * \param u_index index of the preset, counting from zero
 * \return preset name, or NULL if there is no such preset
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API const char *libapoi_audio_equalizer_get_preset_name( unsigned u_index );

/**
 * Get the number of distinct frequency bands for an equalizer.
 *
 * \return number of frequency bands
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API unsigned libapoi_audio_equalizer_get_band_count( void );

/**
 * Get a particular equalizer band frequency.
 *
 * This value can be used, for example, to create a label for an equalizer band control
 * in a user interface.
 *
 * \param u_index index of the band, counting from zero
 * \return equalizer band frequency (Hz), or -1 if there is no such band
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API float libapoi_audio_equalizer_get_band_frequency( unsigned u_index );

/**
 * Create a new default equalizer, with all frequency values zeroed.
 *
 * The new equalizer can subsequently be applied to a media player by invoking
 * libapoi_media_player_set_equalizer().
 *
 * The returned handle should be freed via libapoi_audio_equalizer_release() when
 * it is no longer needed.
 *
 * \return opaque equalizer handle, or NULL on error
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API libapoi_equalizer_t *libapoi_audio_equalizer_new( void );

/**
 * Create a new equalizer, with initial frequency values copied from an existing
 * preset.
 *
 * The new equalizer can subsequently be applied to a media player by invoking
 * libapoi_media_player_set_equalizer().
 *
 * The returned handle should be freed via libapoi_audio_equalizer_release() when
 * it is no longer needed.
 *
 * \param u_index index of the preset, counting from zero
 * \return opaque equalizer handle, or NULL on error
 *         (it must be released with libapoi_audio_equalizer_release())
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API libapoi_equalizer_t *libapoi_audio_equalizer_new_from_preset( unsigned u_index );

/**
 * Release a previously created equalizer instance.
 *
 * The equalizer was previously created by using libapoi_audio_equalizer_new() or
 * libapoi_audio_equalizer_new_from_preset().
 *
 * It is safe to invoke this method with a NULL p_equalizer parameter for no effect.
 *
 * \param p_equalizer opaque equalizer handle, or NULL
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API void libapoi_audio_equalizer_release( libapoi_equalizer_t *p_equalizer );

/**
 * Set a new pre-amplification value for an equalizer.
 *
 * The new equalizer settings are subsequently applied to a media player by invoking
 * libapoi_media_player_set_equalizer().
 *
 * The supplied amplification value will be clamped to the -20.0 to +20.0 range.
 *
 * \param p_equalizer valid equalizer handle, must not be NULL
 * \param f_preamp preamp value (-20.0 to 20.0 Hz)
 * \return zero on success, -1 on error
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API int libapoi_audio_equalizer_set_preamp( libapoi_equalizer_t *p_equalizer, float f_preamp );

/**
 * Get the current pre-amplification value from an equalizer.
 *
 * \param p_equalizer valid equalizer handle, must not be NULL
 * \return preamp value (Hz)
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API float libapoi_audio_equalizer_get_preamp( libapoi_equalizer_t *p_equalizer );

/**
 * Set a new amplification value for a particular equalizer frequency band.
 *
 * The new equalizer settings are subsequently applied to a media player by invoking
 * libapoi_media_player_set_equalizer().
 *
 * The supplied amplification value will be clamped to the -20.0 to +20.0 range.
 *
 * \param p_equalizer valid equalizer handle, must not be NULL
 * \param f_amp amplification value (-20.0 to 20.0 Hz)
 * \param u_band index, counting from zero, of the frequency band to set
 * \return zero on success, -1 on error
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API int libapoi_audio_equalizer_set_amp_at_index( libapoi_equalizer_t *p_equalizer, float f_amp, unsigned u_band );

/**
 * Get the amplification value for a particular equalizer frequency band.
 *
 * \param p_equalizer valid equalizer handle, must not be NULL
 * \param u_band index, counting from zero, of the frequency band to get
 * \return amplification value (Hz); NaN if there is no such frequency band
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API float libapoi_audio_equalizer_get_amp_at_index( libapoi_equalizer_t *p_equalizer, unsigned u_band );

/**
 * Apply new equalizer settings to a media player.
 *
 * The equalizer is first created by invoking libapoi_audio_equalizer_new() or
 * libapoi_audio_equalizer_new_from_preset().
 *
 * It is possible to apply new equalizer settings to a media player whether the media
 * player is currently playing media or not.
 *
 * Invoking this method will immediately apply the new equalizer settings to the audio
 * output of the currently playing media if there is any.
 *
 * If there is no currently playing media, the new equalizer settings will be applied
 * later if and when new media is played.
 *
 * Equalizer settings will automatically be applied to subsequently played media.
 *
 * To disable the equalizer for a media player invoke this method passing NULL for the
 * p_equalizer parameter.
 *
 * The media player does not keep a reference to the supplied equalizer so it is safe
 * for an application to release the equalizer reference any time after this method
 * returns.
 *
 * \param p_mi opaque media player handle
 * \param p_equalizer opaque equalizer handle, or NULL to disable the equalizer for this media player
 * \return zero on success, -1 on error
 * \version LibAPOI 2.2.0 or later
 */
LIBAPOI_API int libapoi_media_player_set_equalizer( libapoi_media_player_t *p_mi, libapoi_equalizer_t *p_equalizer );

/**
 * Media player roles.
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * See \ref libapoi_media_player_set_role()
 */
typedef enum libapoi_media_player_role {
    libapoi_role_None = 0, /**< Don't use a media player role */
    libapoi_role_Music,   /**< Music (or radio) playback */
    libapoi_role_Video, /**< Video playback */
    libapoi_role_Communication, /**< Speech, real-time communication */
    libapoi_role_Game, /**< Video game */
    libapoi_role_Notification, /**< User interaction feedback */
    libapoi_role_Animation, /**< Embedded animation (e.g. in web page) */
    libapoi_role_Production, /**< Audio editing/production */
    libapoi_role_Accessibility, /**< Accessibility */
    libapoi_role_Test /** Testing */
#define libapoi_role_Last libapoi_role_Test
} libapoi_media_player_role_t;

/**
 * Gets the media role.
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \param p_mi media player
 * \return the media player role (\ref libapoi_media_player_role_t)
 */
LIBAPOI_API int libapoi_media_player_get_role(libapoi_media_player_t *p_mi);

/**
 * Sets the media role.
 *
 * \param p_mi media player
 * \param role the media player role (\ref libapoi_media_player_role_t)
 * \return 0 on success, -1 on error
 */
LIBAPOI_API int libapoi_media_player_set_role(libapoi_media_player_t *p_mi,
                                            unsigned role);

/**
 * Start/stop recording
 *
 * \note The user should listen to the libapoi_MediaPlayerRecordChanged event,
 * to monitor the recording state.
 *
 * \version LibAPOI 4.0.0 and later.
 *
 * \param p_mi media player
 * \param enable true to start recording, false to stop
 * \param dir_path path of the recording directory or NULL (use default path),
 * has only an effect when first enabling recording.
 */
LIBAPOI_API void libapoi_media_player_record(libapoi_media_player_t *p_mi,
                                           bool enable, const char *dir_path);

/** @} audio */

/** \defgroup libapoi_media_player_watch_time LibAPOI media player time watch API
 * @{
 */

/**
 * Media Player timer point
 *
 * \note ts and system_date values should not be used directly by the user.
 * libapoi_media_player_time_point_interpolate() will read these values and
 * return an interpolated ts.
 *
 * @see libapoi_media_player_watch_time_on_update
 */
typedef struct libapoi_media_player_time_point_t
{
    /** Position in the range [0.0f;1.0] */
    double position;
    /** Rate of the player */
    double rate;
    /** Valid time, in us >= 0 or -1 */
    int64_t ts_us;
    /** Valid length, in us >= 1 or 0 */
    int64_t length_us;
    /**
     * System date, in us, of this record (always valid).
     * Based on libapoi_clock(). This date can be in the future or in the past.
     * The special value of INT64_MAX mean that the clock was paused when this
     * point was updated. In that case,
     * libapoi_media_player_time_point_interpolate() will return the current
     * ts/pos of this point (there is nothing to interpolate).
     * */
    int64_t system_date_us;
} libapoi_media_player_time_point_t;

/**
 * Callback prototype that notify when the player state or time changed.
 *
 * Get notified when the time is updated by the input or output source. The
 * input source is the 'demux' or the 'access_demux'. The output source are
 * audio and video outputs: an update is received each time a video frame is
 * displayed or an audio sample is written. The delay between each updates may
 * depend on the input and source type (it can be every 5ms, 30ms, 1s or
 * 10s...). Users of this timer may need to update the position at a higher
 * frequency from their own mainloop via
 * libapoi_media_player_time_point_interpolate().
 *
 * \warning It is forbidden to call any Media Player functions from here.
 *
 * \param value always valid, the time corresponding to the state
 * \param data opaque pointer set by libapoi_media_player_watch_time()
 */
typedef void (*libapoi_media_player_watch_time_on_update)(
        const libapoi_media_player_time_point_t *value, void *data);

/**
 * Callback prototype that notify when the timer is paused.
 *
 * This event is sent when the player is paused or stopping. The player
 * user should stop its "interpolate" timer.
 *
 * \note libapoi_media_player_watch_time_on_update() can be called when paused
 * for those 2 reasons:
 * - playback is resumed (libapoi_media_player_time_point_t.system_date is valid)
 * - a track, likely video (next-frame) is outputted when paused
 *   (libapoi_media_player_time_point_t.system_date = INT64_MAX)
 *
 * \warning It is forbidden to call any Media Player functions from here.
 *
 * \param system_date_us system date, in us, of this event, only valid (> 0)
 * when paused. It can be used to interpolate the last updated point to this
 * date in order to get the last paused ts/position.
 * \param data opaque pointer set by libapoi_media_player_watch_time()
 */
typedef void (*libapoi_media_player_watch_time_on_paused)(
        int64_t system_date_us, void *data);

/**
 * Callback prototype that notify when the player is seeking or finished
 * seeking
 *
 * \warning It is forbidden to call any Media Player functions from here.
 *
 * \note It is not possible to receive points via on_update() while seeking.
 *
 * \param value point of the seek request or NULL when seeking is finished
 * \param data opaque pointer set by libapoi_media_player_watch_time()
 */
typedef void (*libapoi_media_player_watch_time_on_seek)(
        const libapoi_media_player_time_point_t *value, void *data);

/**
 * Watch for times updates
 *
 * \warning Only one watcher can be registered at a time. Calling this function
 * a second time (if libapoi_media_player_unwatch_time() was not called
 * in-between) will fail.
 *
 * \param p_mi the media player
 * \param min_period_us corresponds to the minimum period, in us, between each
 * updates, use it to avoid flood from too many source updates, set it to 0 to
 * receive all updates.
 * \param on_update callback to listen to update events (must not be NULL)
 * \param on_paused callback to listen to paused events (can be NULL)
 * \param on_seek callback to listen to seek events (can be NULL)
 * \param cbs_data opaque pointer used by the callbacks
 * \return 0 on success, -1 on error (allocation error, or if already watching)
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API int
libapoi_media_player_watch_time(libapoi_media_player_t *p_mi,
                               int64_t min_period_us,
                               libapoi_media_player_watch_time_on_update on_update,
                               libapoi_media_player_watch_time_on_paused on_paused,
                               libapoi_media_player_watch_time_on_seek on_seek,
                               void *cbs_data);

/**
 * Unwatch time updates
 *
 * \param p_mi the media player
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API void
libapoi_media_player_unwatch_time(libapoi_media_player_t *p_mi);

/**
 * Interpolate a timer value to now

 * \param point time update obtained via the
 * libapoi_media_player_watch_time_on_update() callback
 * \param system_now_us current system date, in us, returned by libapoi_clock()
 * \param out_ts_us pointer where to set the interpolated ts, in us
 * \param out_pos pointer where to set the interpolated position
 * \return 0 in case of success, -1 if the interpolated ts is negative (could
 * happen during the buffering step)
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API int
libapoi_media_player_time_point_interpolate(const libapoi_media_player_time_point_t *point,
                                           int64_t system_now_us,
                                           int64_t *out_ts_us, double *out_pos);

/**
 * Get the date of the next interval
 *
 * Can be used to setup an UI timer in order to update some widgets at specific
 * interval. A next_interval of VLC_TICK_FROM_SEC(1) can be used to update a
 * time widget when the media reaches a new second.
 *
 * \note The media time doesn't necessarily correspond to the system time, that
 * is why this function is needed and uses the rate of the current point.
 *
 * \param point time update obtained via the
 * libapoi_media_player_watch_time_on_update()
 * \param system_now_us same system date used by
 * libapoi_media_player_time_point_interpolate()
 * \param interpolated_ts_us ts returned by
 * libapoi_media_player_time_point_interpolate()
 * \param next_interval_us next interval, in us
 * \return the absolute system date, in us,  of the next interval,
 * use libapoi_delay() to get a relative delay.
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API int64_t
libapoi_media_player_time_point_get_next_date(const libapoi_media_player_time_point_t *point,
                                             int64_t system_now_us,
                                             int64_t interpolated_ts_us,
                                             int64_t next_interval_us);

/** @} libapoi_media_player_watch_time */

/** \defgroup libapoi_media_player_concurrency LibAPOI media player concurrency API
 * @{
 */

/**
 * Lock the media_player internal lock

 * The lock is recursive, so it's safe to use it multiple times from the same
 * thread. You must call libapoi_media_player_unlock() the same number of times
 * you called libapoi_media_player_lock().
 *
 * Locking is not mandatory before calling a libapoi_media_player_t function
 * since they will automatically hold the lock internally.
 *
 * This lock can be used to synchronise user variables that interact with the
 * libapoi_media_player_t or can be used to call several functions together.
 *
 * \param mp media player object
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API void libapoi_media_player_lock( libapoi_media_player_t *mp );

/**
 * Unlock the media_player internal lock
 *
 * \see libapoi_media_player_lock
 *
 * \param mp media player object locked using /ref libapoi_media_player_lock
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API void libapoi_media_player_unlock( libapoi_media_player_t *mp );

/**
 * Wait for an event to be signalled
 *
 * \note this is equivalent to pthread_cond_wait() with the
 * libapoi_media_player_t internal mutex and condition variable. This function
 * may spuriously wake up even without libapoi_media_player_signal() being
 * called.
 *
 * \warning this function must not be called from any libapoi callbacks and
 * events. The lock should be held only one time before waiting.
 *
 * \param mp media player object locked using /ref libapoi_media_player_lock
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API void libapoi_media_player_wait( libapoi_media_player_t *mp );

/**
 * Signal all threads waiting for a signalling event
 *
 * \note this is equivalent to pthread_cond_broadcast() with the
 * libapoi_media_player_t internal condition variable.
 *
 * \param mp media player object locked using /ref libapoi_media_player_lock
 * \version LibAPOI 4.0.0 or later
 */
LIBAPOI_API void libapoi_media_player_signal( libapoi_media_player_t *mp );

/** @} libapoi_media_player_concurrency */

/** @} media_player */

# ifdef __cplusplus
}
# endif

#endif /* VLC_LIBAPOI_MEDIA_PLAYER_H */
