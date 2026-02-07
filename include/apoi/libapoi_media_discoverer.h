/*****************************************************************************
 * libapoi_media_discoverer.h:  libapoi external API
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

#ifndef VLC_LIBAPOI_MEDIA_DISCOVERER_H
#define VLC_LIBAPOI_MEDIA_DISCOVERER_H 1

# ifdef __cplusplus
extern "C" {
# endif

typedef struct libapoi_media_list_t libapoi_media_list_t;

/**
 * Category of a media discoverer
 * \see libapoi_media_discoverer_list_get()
 */
typedef enum libapoi_media_discoverer_category_t {
    /** devices, like portable music player */
    libapoi_media_discoverer_devices,
    /** LAN/WAN services, like Upnp, SMB, or SAP */
    libapoi_media_discoverer_lan,
    /** Podcasts */
    libapoi_media_discoverer_podcasts,
    /** Local directories, like Video, Music or Pictures directories */
    libapoi_media_discoverer_localdirs,
} libapoi_media_discoverer_category_t;

/**
 * Media discoverer description
 * \see libapoi_media_discoverer_list_get()
 */
typedef struct libapoi_media_discoverer_description_t {
    char *psz_name;
    char *psz_longname;
    libapoi_media_discoverer_category_t i_cat;
} libapoi_media_discoverer_description_t;

/** \defgroup libapoi_media_discoverer LibAPOI media discovery
 * \ingroup libapoi
 * LibAPOI media discovery finds available media via various means.
 * This corresponds to the service discovery functionality in VLC media player.
 * Different plugins find potential medias locally (e.g. user media directory),
 * from peripherals (e.g. video capture device), on the local network
 * (e.g. SAP) or on the Internet (e.g. Internet radios).
 * @{
 * \file
 * LibAPOI media discovery external API
 */

typedef struct libapoi_media_discoverer_t libapoi_media_discoverer_t;

/**
 * Create a media discoverer object by name.
 *
 * After this object is created, you should attach to media_list events in
 * order to be notified of new items discovered.
 *
 * You need to call libapoi_media_discoverer_start() in order to start the
 * discovery.
 *
 * \see libapoi_media_discoverer_media_list
 * \see libapoi_media_discoverer_start
 *
 * \param p_inst libapoi instance
 * \param psz_name service name; use libapoi_media_discoverer_list_get() to get
 * a list of the discoverer names available in this libVLC instance
 * \return media discover object or NULL in case of error
 * \version LibAPOI 3.0.0 or later
 */
LIBAPOI_API libapoi_media_discoverer_t *
libapoi_media_discoverer_new( libapoi_instance_t * p_inst,
                             const char * psz_name );

/**
 * Start media discovery.
 *
 * To stop it, call libapoi_media_discoverer_stop() or
 * libapoi_media_discoverer_list_release() directly.
 *
 * \see libapoi_media_discoverer_stop
 *
 * \param p_mdis media discover object
 * \return -1 in case of error, 0 otherwise
 * \version LibAPOI 3.0.0 or later
 */
LIBAPOI_API int
libapoi_media_discoverer_start( libapoi_media_discoverer_t * p_mdis );

/**
 * Stop media discovery.
 *
 * \see libapoi_media_discoverer_start
 *
 * \param p_mdis media discover object
 * \version LibAPOI 3.0.0 or later
 */
LIBAPOI_API void
libapoi_media_discoverer_stop( libapoi_media_discoverer_t * p_mdis );

/**
 * Release media discover object. If the reference count reaches 0, then
 * the object will be released.
 *
 * \param p_mdis media service discover object
 */
LIBAPOI_API void
libapoi_media_discoverer_release( libapoi_media_discoverer_t * p_mdis );

/**
 * Get media service discover media list.
 *
 * \param p_mdis media service discover object
 * \return list of media items
 */
LIBAPOI_API libapoi_media_list_t *
libapoi_media_discoverer_media_list( libapoi_media_discoverer_t * p_mdis );

/**
 * Query if media service discover object is running.
 *
 * \param p_mdis media service discover object
 *
 * \retval true running
 * \retval false not running
 */
LIBAPOI_API bool
libapoi_media_discoverer_is_running(libapoi_media_discoverer_t *p_mdis);

/**
 * Get media discoverer services by category
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \param p_inst libapoi instance
 * \param i_cat category of services to fetch
 * \param ppp_services address to store an allocated array of media discoverer
 * services (must be freed with libapoi_media_discoverer_list_release() by
 * the caller) [OUT]
 *
 * \return the number of media discoverer services (0 on error)
 */
LIBAPOI_API size_t
libapoi_media_discoverer_list_get( libapoi_instance_t *p_inst,
                                  libapoi_media_discoverer_category_t i_cat,
                                  libapoi_media_discoverer_description_t ***ppp_services );

/**
 * Release an array of media discoverer services
 *
 * \version LibAPOI 3.0.0 and later.
 *
 * \see libapoi_media_discoverer_list_get()
 *
 * \param pp_services array to release
 * \param i_count number of elements in the array
 */
LIBAPOI_API void
libapoi_media_discoverer_list_release( libapoi_media_discoverer_description_t **pp_services,
                                      size_t i_count );

/**@} */

# ifdef __cplusplus
}
# endif

#endif /* <apoi/libapoi.h> */
