/*****************************************************************************
 * renderer_discoverer.c: libapoi renderer API
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>

#include <apoi/libapoi.h>
#include <apoi/libapoi_renderer_discoverer.h>

#include <vlc_common.h>
#include <vlc_arrays.h>

#include "libapoi_internal.h"
#include "renderer_discoverer_internal.h"

struct libapoi_renderer_discoverer_t
{
    libapoi_event_manager_t  event_manager;
    vlc_object_t *          p_object;
    vlc_renderer_discovery_t *p_rd;

    int                     i_items;
    vlc_renderer_item_t **  pp_items;
    char                    name[];
};

static_assert( VLC_RENDERER_CAN_AUDIO == LIBAPOI_RENDERER_CAN_AUDIO &&
               VLC_RENDERER_CAN_VIDEO == LIBAPOI_RENDERER_CAN_VIDEO,
              "core/libapoi renderer flags mismatch" );

vlc_renderer_item_t *
libapoi_renderer_item_to_vlc( libapoi_renderer_item_t *p_item )
{
    return (vlc_renderer_item_t*) p_item;
}

static void renderer_discovery_item_added( vlc_renderer_discovery_t *rd,
                                           vlc_renderer_item_t *p_item )
{
    libapoi_renderer_discoverer_t *p_lrd = rd->owner.sys;

    vlc_renderer_item_hold( p_item );

    TAB_APPEND( p_lrd->i_items, p_lrd->pp_items, p_item );

    libapoi_event_t event = {
        .type = libapoi_RendererDiscovererItemAdded,
        .u.renderer_discoverer_item_added.item =
            (libapoi_renderer_item_t*) p_item,
    };
    libapoi_event_send( &p_lrd->event_manager, &event );
}

static void renderer_discovery_item_removed( vlc_renderer_discovery_t *rd,
                                             vlc_renderer_item_t *p_item )
{
    libapoi_renderer_discoverer_t *p_lrd = rd->owner.sys;

    int i_idx;
    TAB_FIND( p_lrd->i_items, p_lrd->pp_items, p_item, i_idx );
    assert( i_idx != -1 );
    TAB_ERASE( p_lrd->i_items, p_lrd->pp_items, i_idx );

    libapoi_event_t event = {
        .type = libapoi_RendererDiscovererItemDeleted,
        .u.renderer_discoverer_item_deleted.item =
            (libapoi_renderer_item_t*) p_item,
    };
    libapoi_event_send( &p_lrd->event_manager, &event );

    vlc_renderer_item_release( p_item );
}

libapoi_renderer_item_t *
libapoi_renderer_item_hold(libapoi_renderer_item_t *p_item)
{
    vlc_renderer_item_hold( (vlc_renderer_item_t *) p_item );
    return p_item;
}

void
libapoi_renderer_item_release(libapoi_renderer_item_t *p_item)
{
    vlc_renderer_item_release( (vlc_renderer_item_t *) p_item );
}

const char *
libapoi_renderer_item_name( const libapoi_renderer_item_t *p_item )
{
    return vlc_renderer_item_name( (const vlc_renderer_item_t *) p_item );
}

const char *
libapoi_renderer_item_type( const libapoi_renderer_item_t *p_item )
{
    return vlc_renderer_item_type( (const vlc_renderer_item_t *) p_item );
}

const char *
libapoi_renderer_item_icon_uri( const libapoi_renderer_item_t *p_item )
{
    return vlc_renderer_item_icon_uri( (const vlc_renderer_item_t *) p_item );
}

int
libapoi_renderer_item_flags( const libapoi_renderer_item_t *p_item )
{
    return vlc_renderer_item_flags( (const vlc_renderer_item_t *) p_item );
}

libapoi_renderer_discoverer_t *
libapoi_renderer_discoverer_new( libapoi_instance_t *p_inst,
                                const char *psz_name )
{
    size_t len = strlen( psz_name ) + 1;
    libapoi_renderer_discoverer_t *p_lrd = malloc( sizeof(*p_lrd) + len );

    if( unlikely(p_lrd == NULL) )
        return NULL;

    p_lrd->p_object = VLC_OBJECT(p_inst->p_libapoi_int);
    memcpy( p_lrd->name, psz_name, len );
    TAB_INIT( p_lrd->i_items, p_lrd->pp_items );
    p_lrd->p_rd = NULL;
    libapoi_event_manager_init( &p_lrd->event_manager, p_lrd );

    return p_lrd;
}

void
libapoi_renderer_discoverer_release( libapoi_renderer_discoverer_t *p_lrd )
{
    libapoi_renderer_discoverer_stop( p_lrd );
    libapoi_event_manager_destroy( &p_lrd->event_manager );
    free( p_lrd );
}

int
libapoi_renderer_discoverer_start( libapoi_renderer_discoverer_t *p_lrd )
{
    assert( p_lrd->p_rd == NULL );

    struct vlc_renderer_discovery_owner owner =
    {
        p_lrd,
        renderer_discovery_item_added,
        renderer_discovery_item_removed,
    };

    p_lrd->p_rd = vlc_rd_new( p_lrd->p_object, p_lrd->name, &owner );
    return p_lrd->p_rd != NULL ? 0 : -1;
}

void
libapoi_renderer_discoverer_stop( libapoi_renderer_discoverer_t *p_lrd )
{
    if( p_lrd->p_rd != NULL )
    {
        vlc_rd_release( p_lrd->p_rd );
        p_lrd->p_rd = NULL;
    }

    for( int i = 0; i < p_lrd->i_items; ++i )
        vlc_renderer_item_release( p_lrd->pp_items[i] );
    TAB_CLEAN( p_lrd->i_items, p_lrd->pp_items );
}

libapoi_event_manager_t *
libapoi_renderer_discoverer_event_manager( libapoi_renderer_discoverer_t *p_lrd )
{
    return &p_lrd->event_manager;
}

void
libapoi_renderer_discoverer_list_release( libapoi_rd_description_t **pp_services,
                                         size_t i_count )
{
    if( i_count > 0 )
    {
        for( size_t i = 0; i < i_count; ++i )
        {
            free( pp_services[i]->psz_name );
            free( pp_services[i]->psz_longname );
        }
        free( *pp_services );
        free( pp_services );
    }
}

size_t
libapoi_renderer_discoverer_list_get( libapoi_instance_t *p_inst,
                                     libapoi_rd_description_t ***ppp_services )
{
    assert( p_inst != NULL && ppp_services != NULL );

    /* Fetch all rd names, and longnames */
    char **ppsz_names, **ppsz_longnames;
    int i_ret = vlc_rd_get_names( p_inst->p_libapoi_int, &ppsz_names,
                                  &ppsz_longnames );

    if( i_ret != VLC_SUCCESS )
    {
        *ppp_services = NULL;
        return 0;
    }

    /* Count the number of sd matching our category (i_cat/i_core_cat) */
    size_t i_nb_services = 0;
    char **ppsz_name = ppsz_names;
    for( ; *ppsz_name != NULL; ppsz_name++ )
        i_nb_services++;

    libapoi_rd_description_t **pp_services = NULL,
                                              *p_services = NULL;
    if( i_nb_services > 0 )
    {
        /* Double alloc here, so that the caller iterates through pointers of
         * struct instead of structs. This allows us to modify the struct
         * without breaking the API. */

        pp_services =
            malloc( i_nb_services
                    * sizeof(libapoi_rd_description_t *) );
        p_services =
            malloc( i_nb_services
                    * sizeof(libapoi_rd_description_t) );
        if( pp_services == NULL || p_services == NULL )
        {
            free( pp_services );
            free( p_services );
            pp_services = NULL;
            p_services = NULL;
            i_nb_services = 0;
            /* Even if alloc fails, the next loop must be run in order to free
             * names returned by vlc_sd_GetNames */
        }
    }

    /* Fill output pp_services or free unused name, longname */
    char **ppsz_longname = ppsz_longnames;
    unsigned int i_service_idx = 0;
    libapoi_rd_description_t *p_service = p_services;
    for( ppsz_name = ppsz_names; *ppsz_name != NULL; ppsz_name++, ppsz_longname++ )
    {
        if( pp_services != NULL )
        {
            p_service->psz_name = *ppsz_name;
            p_service->psz_longname = *ppsz_longname;
            pp_services[i_service_idx++] = p_service++;
        }
        else
        {
            free( *ppsz_name );
            free( *ppsz_longname );
        }
    }
    free( ppsz_names );
    free( ppsz_longnames );

    *ppp_services = pp_services;
    return i_nb_services;
}
