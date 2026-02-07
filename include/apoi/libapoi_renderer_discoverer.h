/*****************************************************************************
 * libapoi_renderer_discoverer.h:  libapoi external API
 *****************************************************************************
 * Copyright © 2016 VLC authors and VideoLAN
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

#ifndef VLC_LIBAPOI_RENDERER_DISCOVERER_H
#define VLC_LIBAPOI_RENDERER_DISCOVERER_H 1

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @defgroup libapoi_renderer_discoverer LibAPOI renderer discoverer
 * @ingroup libapoi
 * LibAPOI renderer discoverer finds available renderers available on the local
 * network
 * @{
 * @file
 * LibAPOI renderer discoverer external API
 */

typedef struct libapoi_renderer_discoverer_t libapoi_renderer_discoverer_t;

/**
 * Renderer discoverer description
 *
 * \see libapoi_renderer_discoverer_list_get()
 */
typedef struct libapoi_rd_description_t
{
    char *psz_name;
    char *psz_longname;
} libapoi_rd_description_t;

/** The renderer can render audio */
#define LIBAPOI_RENDERER_CAN_AUDIO 0x0001
/** The renderer can render video */
#define LIBAPOI_RENDERER_CAN_VIDEO 0x0002

/**
 * Renderer item
 *
 * This struct is passed by a @ref libapoi_event_t when a new renderer is added
 * or deleted.
 *
 * An item is valid until the @ref libapoi_RendererDiscovererItemDeleted event
 * is called with the same pointer.
 *
 * \see libapoi_renderer_discoverer_event_manager()
 */
typedef struct libapoi_renderer_item_t libapoi_renderer_item_t;


/**
 * Hold a renderer item, i.e. creates a new reference
 *
 * This functions need to called from the libapoi_RendererDiscovererItemAdded
 * callback if the libapoi user wants to use this item after. (for display or
 * for passing it to the mediaplayer for example).
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \return the current item
 */
LIBAPOI_API libapoi_renderer_item_t *
libapoi_renderer_item_hold(libapoi_renderer_item_t *p_item);

/**
 * Releases a renderer item, i.e. decrements its reference counter
 *
 * \version LibAPOI 3.0.0 or later
 */
LIBAPOI_API void
libapoi_renderer_item_release(libapoi_renderer_item_t *p_item);

/**
 * Get the human readable name of a renderer item
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \return the name of the item (can't be NULL, must *not* be freed)
 */
LIBAPOI_API const char *
libapoi_renderer_item_name(const libapoi_renderer_item_t *p_item);

/**
 * Get the type (not translated) of a renderer item. For now, the type can only
 * be "chromecast" ("upnp", "airplay" may come later).
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \return the type of the item (can't be NULL, must *not* be freed)
 */
LIBAPOI_API const char *
libapoi_renderer_item_type(const libapoi_renderer_item_t *p_item);

/**
 * Get the icon uri of a renderer item
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \return the uri of the item's icon (can be NULL, must *not* be freed)
 */
LIBAPOI_API const char *
libapoi_renderer_item_icon_uri(const libapoi_renderer_item_t *p_item);

/**
 * Get the flags of a renderer item
 *
 * \see LIBAPOI_RENDERER_CAN_AUDIO
 * \see LIBAPOI_RENDERER_CAN_VIDEO
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \return bitwise flag: capabilities of the renderer, see
 */
LIBAPOI_API int
libapoi_renderer_item_flags(const libapoi_renderer_item_t *p_item);

/**
 * Create a renderer discoverer object by name
 *
 * After this object is created, you should attach to events in order to be
 * notified of the discoverer events.
 *
 * You need to call libapoi_renderer_discoverer_start() in order to start the
 * discovery.
 *
 * \see libapoi_renderer_discoverer_event_manager()
 * \see libapoi_renderer_discoverer_start()
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \param p_inst libapoi instance
 * \param psz_name service name; use libapoi_renderer_discoverer_list_get() to
 * get a list of the discoverer names available in this libVLC instance
 * \return media discover object or NULL in case of error
 */
LIBAPOI_API libapoi_renderer_discoverer_t *
libapoi_renderer_discoverer_new( libapoi_instance_t *p_inst,
                                const char *psz_name );

/**
 * Release a renderer discoverer object
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \param p_rd renderer discoverer object
 */
LIBAPOI_API void
libapoi_renderer_discoverer_release( libapoi_renderer_discoverer_t *p_rd );

/**
 * Start renderer discovery
 *
 * To stop it, call libapoi_renderer_discoverer_stop() or
 * libapoi_renderer_discoverer_release() directly.
 *
 * \see libapoi_renderer_discoverer_stop()
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \param p_rd renderer discoverer object
 * \return -1 in case of error, 0 otherwise
 */
LIBAPOI_API int
libapoi_renderer_discoverer_start( libapoi_renderer_discoverer_t *p_rd );

/**
 * Stop renderer discovery.
 *
 * \see libapoi_renderer_discoverer_start()
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \param p_rd renderer discoverer object
 */
LIBAPOI_API void
libapoi_renderer_discoverer_stop( libapoi_renderer_discoverer_t *p_rd );

/**
 * Get the event manager of the renderer discoverer
 *
 * The possible events to attach are @ref libapoi_RendererDiscovererItemAdded
 * and @ref libapoi_RendererDiscovererItemDeleted.
 *
 * The @ref libapoi_renderer_item_t struct passed to event callbacks is owned by
 * VLC, users should take care of holding/releasing this struct for their
 * internal usage.
 *
 * \see libapoi_event_t.u.renderer_discoverer_item_added.item
 * \see libapoi_event_t.u.renderer_discoverer_item_removed.item
 *
 * \version LibAPOI 3.0.0 or later
 *
 * \return a valid event manager (can't fail)
 */
LIBAPOI_API libapoi_event_manager_t *
libapoi_renderer_discoverer_event_manager( libapoi_renderer_discoverer_t *p_rd );

/**
 * Get media discoverer services
 *
 * \see libapoi_renderer_list_release()
 *
 * \version LibAPOI 3.0.0 and later
 *
 * \param p_inst libapoi instance
 * \param ppp_services address to store an allocated array of renderer
 * discoverer services (must be freed with libapoi_renderer_list_release() by
 * the caller) [OUT]
 *
 * \return the number of media discoverer services (0 on error)
 */
LIBAPOI_API size_t
libapoi_renderer_discoverer_list_get( libapoi_instance_t *p_inst,
                                     libapoi_rd_description_t ***ppp_services );

/**
 * Release an array of media discoverer services
 *
 * \see libapoi_renderer_discoverer_list_get()
 *
 * \version LibAPOI 3.0.0 and later
 *
 * \param pp_services array to release
 * \param i_count number of elements in the array
 */
LIBAPOI_API void
libapoi_renderer_discoverer_list_release( libapoi_rd_description_t **pp_services,
                                         size_t i_count );

/** @} */

# ifdef __cplusplus
}
# endif

#endif
