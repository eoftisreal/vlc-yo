/*****************************************************************************
 * picture.c:  libapoi API picture management
 *****************************************************************************
 * Copyright (C) 2018 VLC authors and VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#include <stdckdint.h>

#include <apoi/libapoi.h>
#include <apoi/libapoi_picture.h>
#include "libapoi_internal.h"

#include <vlc_atomic.h>
#include <vlc_picture.h>
#include <vlc_block.h>
#include <vlc_image.h>
#include <vlc_input.h>
#include <vlc_fs.h>

#include "picture_internal.h"

struct libapoi_picture_t
{
    vlc_atomic_rc_t rc;
    libapoi_picture_type_t type;
    block_t* converted;
    video_format_t fmt;
    libapoi_time_t time;
    input_attachment_t* attachment;
};

struct libapoi_picture_list_t
{
    size_t count;
    libapoi_picture_t* pictures[];
};

libapoi_picture_t* libapoi_picture_new( vlc_object_t* p_obj, picture_t* input,
                                      libapoi_picture_type_t type,
                                      unsigned int width, unsigned int height,
                                      bool crop )
{
    libapoi_picture_t *pic = malloc( sizeof( *pic ) );
    if ( unlikely( pic == NULL ) )
        return NULL;
    vlc_atomic_rc_init( &pic->rc );
    pic->type = type;
    pic->time = libapoi_time_from_vlc_tick( input->date );
    pic->attachment = NULL;

    static const vlc_fourcc_t table[] = {
        [libapoi_picture_Jpg] = VLC_CODEC_JPEG,
        [libapoi_picture_Png] = VLC_CODEC_PNG,
        [libapoi_picture_Argb] = VLC_CODEC_ARGB,
        [libapoi_picture_WebP] = VLC_CODEC_WEBP,
        [libapoi_picture_Rgba] = VLC_CODEC_RGBA,
    };
    assert(ARRAY_SIZE(table) > type && table[type] != 0);
    vlc_fourcc_t format = table[type];
    if ( picture_Export( p_obj, &pic->converted, &pic->fmt,
                         input, format, width, height, crop ) != VLC_SUCCESS )
    {
        free( pic );
        return NULL;
    }

    return pic;
}

static void libapoi_picture_block_release( block_t* block )
{
    free( block );
}

static const struct vlc_block_callbacks block_cbs =
{
    libapoi_picture_block_release,
};

static bool IsSupportedByLibAPOI(vlc_fourcc_t fcc)
{
    switch (fcc)
    {
        case VLC_CODEC_PNG:
        case VLC_CODEC_JPEG:
        case VLC_CODEC_WEBP:
            return true;
        default:
            return false;
    }
}

static libapoi_picture_t* libapoi_picture_from_attachment( input_attachment_t* attachment )
{
    vlc_fourcc_t fcc = image_Mime2Fourcc( attachment->psz_mime );
    if (!IsSupportedByLibAPOI(fcc))
        return NULL;

    libapoi_picture_t *pic = malloc( sizeof( *pic ) );
    if ( unlikely( pic == NULL ) )
        return NULL;
    pic->converted = block_New(&block_cbs, attachment->p_data,
                               attachment->i_data);
    if ( unlikely( pic->converted == NULL ) )
    {
        free(pic);
        return NULL;
    }
    vlc_atomic_rc_init( &pic->rc );
    pic->attachment = vlc_input_attachment_Hold( attachment );
    pic->time = VLC_TICK_INVALID;
    video_format_Init( &pic->fmt, 0 );
    switch ( fcc )
    {
    case VLC_CODEC_PNG:
        pic->type = libapoi_picture_Png;
        break;
    case VLC_CODEC_JPEG:
        pic->type = libapoi_picture_Jpg;
        break;
    case VLC_CODEC_WEBP:
        pic->type = libapoi_picture_WebP;
        break;
    default:
        vlc_assert_unreachable();
    }

    return pic;
}

libapoi_picture_t *libapoi_picture_retain( libapoi_picture_t* pic )
{
    vlc_atomic_rc_inc( &pic->rc );
    return pic;
}

void libapoi_picture_release( libapoi_picture_t* pic )
{
    if ( vlc_atomic_rc_dec( &pic->rc ) == false )
        return;
    video_format_Clean( &pic->fmt );
    if ( pic->converted )
        block_Release( pic->converted );
    if ( pic->attachment )
        vlc_input_attachment_Release( pic->attachment );
    free( pic );
}

int libapoi_picture_save( const libapoi_picture_t* pic, const char* path )
{
    FILE* file = vlc_fopen( path, "wb" );
    if ( !file )
        return -1;
    size_t res = fwrite( pic->converted->p_buffer,
                         pic->converted->i_buffer, 1, file );
    fclose( file );
    return res == 1 ? 0 : -1;
}

const unsigned char* libapoi_picture_get_buffer( const libapoi_picture_t* pic,
                                                size_t *size )
{
    assert( size != NULL );
    *size = pic->converted->i_buffer;
    return pic->converted->p_buffer;
}

libapoi_picture_type_t libapoi_picture_type( const libapoi_picture_t* pic )
{
    return pic->type;
}

unsigned int libapoi_picture_get_stride( const libapoi_picture_t *pic )
{
    assert( pic->type == libapoi_picture_Argb || pic->type == libapoi_picture_Rgba );
    return pic->fmt.i_width * 4;
}

unsigned int libapoi_picture_get_width( const libapoi_picture_t* pic )
{
    return pic->fmt.i_visible_width;
}

unsigned int libapoi_picture_get_height( const libapoi_picture_t* pic )
{
    return pic->fmt.i_visible_height;
}

libapoi_time_t libapoi_picture_get_time( const libapoi_picture_t* pic )
{
    return pic->time;
}

libapoi_picture_list_t* libapoi_picture_list_from_attachments( input_attachment_t* const* attachments,
                                                             size_t nb_attachments )
{
    size_t size = 0;
    libapoi_picture_list_t* list;
    if (ckd_mul(&size, nb_attachments, sizeof (libapoi_picture_t *)) ||
        ckd_add(&size, sizeof (*list), size))
        return NULL;

    list = malloc( size );
    if ( !list )
        return NULL;
    list->count = 0;
    for ( size_t i = 0; i < nb_attachments; ++i )
    {
        input_attachment_t* a = attachments[i];
        libapoi_picture_t *pic = libapoi_picture_from_attachment( a );
        if( !pic )
            continue;
        list->pictures[list->count] = pic;
        list->count++;
    }
    return list;
}

size_t libapoi_picture_list_count( const libapoi_picture_list_t* list )
{
    assert( list );
    return list->count;
}

libapoi_picture_t* libapoi_picture_list_at( const libapoi_picture_list_t* list,
                                          size_t index )
{
    assert( list );
    return list->pictures[index];
}

void libapoi_picture_list_destroy( libapoi_picture_list_t* list )
{
    if ( !list )
        return;
    for ( size_t i = 0; i < list->count; ++i )
        libapoi_picture_release( list->pictures[i] );
    free( list );
}
