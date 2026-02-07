/*****************************************************************************
 * libapoi_video.h: libapoi video-related enumerations
 *****************************************************************************
 * Copyright (C) 1998-2010 VLC authors and VideoLAN
 * Copyright (C) 2023      Videolabs
 *
 * Authors: Filippo Carone <littlejohn@videolan.org>
 *          Pierre d'Herbemont <pdherbemont@videolan.org>
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
#ifndef VLC_LIBAPOI_VIDEO_H
#define VLC_LIBAPOI_VIDEO_H 1

# ifdef __cplusplus
extern "C"{
# endif

typedef enum libapoi_video_orient_t
{
    libapoi_video_orient_top_left,       /**< Normal. Top line represents top, left column left. */
    libapoi_video_orient_top_right,      /**< Flipped horizontally */
    libapoi_video_orient_bottom_left,    /**< Flipped vertically */
    libapoi_video_orient_bottom_right,   /**< Rotated 180 degrees */
    libapoi_video_orient_left_top,       /**< Transposed */
    libapoi_video_orient_left_bottom,    /**< Rotated 90 degrees clockwise (or 270 anti-clockwise) */
    libapoi_video_orient_right_top,      /**< Rotated 90 degrees anti-clockwise */
    libapoi_video_orient_right_bottom    /**< Anti-transposed */
} libapoi_video_orient_t;

typedef enum libapoi_video_projection_t
{
    libapoi_video_projection_rectangular,
    libapoi_video_projection_equirectangular, /**< 360 spherical */

    libapoi_video_projection_cubemap_layout_standard = 0x100,
} libapoi_video_projection_t;

typedef enum libapoi_video_multiview_t
{
    libapoi_video_multiview_2d,                  /**< No stereoscopy: 2D picture. */
    libapoi_video_multiview_stereo_sbs,          /**< Side-by-side */
    libapoi_video_multiview_stereo_tb,           /**< Top-bottom */
    libapoi_video_multiview_stereo_row,          /**< Row sequential */
    libapoi_video_multiview_stereo_col,          /**< Column sequential */
    libapoi_video_multiview_stereo_frame,        /**< Frame sequential */
    libapoi_video_multiview_stereo_checkerboard, /**< Checkerboard pattern */
} libapoi_video_multiview_t;

# ifdef __cplusplus
} // extern "C"
# endif

#endif
