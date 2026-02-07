/*****************************************************************************
 * libapoi_picture.h:  libapoi external API
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

#ifndef VLC_LIBAPOI_PICTURE_H
#define VLC_LIBAPOI_PICTURE_H 1

# ifdef __cplusplus
extern "C" {
# endif

typedef struct libapoi_picture_t libapoi_picture_t;
typedef struct libapoi_picture_list_t libapoi_picture_list_t;

typedef enum libapoi_picture_type_t
{
    libapoi_picture_Argb,
    libapoi_picture_Png,
    libapoi_picture_Jpg,
    libapoi_picture_WebP,
    libapoi_picture_Rgba,
} libapoi_picture_type_t;

/**
 * Increment the reference count of this picture.
 *
 * \see libapoi_picture_release()
 * \param pic A picture object
 * \return the same object
 */
LIBAPOI_API libapoi_picture_t *
libapoi_picture_retain( libapoi_picture_t* pic );

/**
 * Decrement the reference count of this picture.
 * When the reference count reaches 0, the picture will be released.
 * The picture must not be accessed after calling this function.
 *
 * \see libapoi_picture_retain
 * \param pic A picture object
 */
LIBAPOI_API void
libapoi_picture_release( libapoi_picture_t* pic );

/**
 * Saves this picture to a file. The image format is the same as the one
 * returned by \link libapoi_picture_type \endlink
 *
 * \param pic A picture object
 * \param path The path to the generated file
 * \return 0 in case of success, -1 otherwise
 */
LIBAPOI_API int
libapoi_picture_save( const libapoi_picture_t* pic, const char* path );

/**
 * Returns the image internal buffer, including potential padding.
 * The libapoi_picture_t owns the returned buffer, which must not be modified nor
 * freed.
 *
 * \param pic A picture object
 * \param size A pointer to a size_t that will hold the size of the buffer [required]
 * \return A pointer to the internal buffer.
 */
LIBAPOI_API const unsigned char*
libapoi_picture_get_buffer( const libapoi_picture_t* pic, size_t *size );

/**
 * Returns the picture type
 *
 * \param pic A picture object
 * \see libapoi_picture_type_t
 */
LIBAPOI_API libapoi_picture_type_t
libapoi_picture_type( const libapoi_picture_t* pic );

/**
 * Returns the image stride, ie. the number of bytes per line.
 * This can only be called on images of type libapoi_picture_Argb
 *
 * \param pic A picture object
 */
LIBAPOI_API unsigned int
libapoi_picture_get_stride( const libapoi_picture_t* pic );

/**
 * Returns the width of the image in pixels
 *
 * \param pic A picture object
 */
LIBAPOI_API unsigned int
libapoi_picture_get_width( const libapoi_picture_t* pic );

/**
 * Returns the height of the image in pixels
 *
 * \param pic A picture object
 */
LIBAPOI_API unsigned int
libapoi_picture_get_height( const libapoi_picture_t* pic );

/**
 * Returns the time at which this picture was generated, in milliseconds
 * \param pic A picture object
 */
LIBAPOI_API libapoi_time_t
libapoi_picture_get_time( const libapoi_picture_t* pic );

/**
 * Returns the number of pictures in the list
 */
LIBAPOI_API size_t libapoi_picture_list_count( const libapoi_picture_list_t* list );

/**
 * Returns the picture at the provided index.
 *
 * If the index is out of bound, the result is undefined.
 */
LIBAPOI_API libapoi_picture_t* libapoi_picture_list_at( const libapoi_picture_list_t* list,
                                                     size_t index );

/**
 * Destroys a picture list and releases the pictures it contains
 * \param list The list to destroy
 *
 * Calling this function with a NULL list is safe and will return immediately
 */
LIBAPOI_API void libapoi_picture_list_destroy( libapoi_picture_list_t* list );

# ifdef __cplusplus
}
# endif

#endif // VLC_LIBAPOI_PICTURE_H
