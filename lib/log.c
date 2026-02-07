/*****************************************************************************
 * log.c: libapoi new API log functions
 *****************************************************************************
 * Copyright (C) 2005 VLC authors and VideoLAN
 *
 *
 * Authors: Damien Fouilleul <damienf@videolan.org>
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
#include <apoi/apoi.h>
#include "libapoi_internal.h"
#include <vlc_common.h>
#include <vlc_interface.h>

/*** Logging core dispatcher ***/

void libapoi_log_get_context(const libapoi_log_t *ctx,
                            const char **restrict module,
                            const char **restrict file,
                            unsigned *restrict line)
{
    if (module != NULL)
        *module = ctx->psz_module;
    if (file != NULL)
        *file = ctx->file;
    if (line != NULL)
        *line = ctx->line;
}

void libapoi_log_get_object(const libapoi_log_t *ctx,
                           const char **restrict name,
                           const char **restrict header,
                           uintptr_t *restrict id)
{
    if (name != NULL)
        *name = (ctx->psz_object_type != NULL)
                ? ctx->psz_object_type : "generic";
    if (header != NULL)
        *header = ctx->psz_header;
    if (id != NULL)
        *id = ctx->i_object_id;
}

static void libapoi_logf (void *data, int level, const vlc_log_t *item,
                         const char *fmt, va_list ap)
{
    libapoi_instance_t *inst = data;

    switch (level)
    {
        case VLC_MSG_INFO: level = LIBAPOI_NOTICE;  break;
        case VLC_MSG_ERR:  level = LIBAPOI_ERROR;   break;
        case VLC_MSG_WARN: level = LIBAPOI_WARNING; break;
        case VLC_MSG_DBG:  level = LIBAPOI_DEBUG;   break;
    }

    inst->log.cb (inst->log.data, level, item, fmt, ap);
}

static const struct vlc_logger_operations libapoi_log_ops = {
    libapoi_logf, NULL
};

void libapoi_log_unset (libapoi_instance_t *inst)
{
    vlc_LogSet (inst->p_libapoi_int, NULL, NULL);
}

void libapoi_log_set (libapoi_instance_t *inst, libapoi_log_cb cb, void *data)
{
    libapoi_log_unset (inst); /* <- Barrier before modifying the callback */
    inst->log.cb = cb;
    inst->log.data = data;
    vlc_LogSet(inst->p_libapoi_int, &libapoi_log_ops, inst);
}

/*** Helpers for logging to files ***/
static void libapoi_log_file (void *data, int level, const libapoi_log_t *log,
                             const char *fmt, va_list ap)
{
    FILE *stream = data;

    flockfile (stream);
    vfprintf (stream, fmt, ap);
    fputc ('\n', stream);
    funlockfile (stream);
    (void) level; (void) log;
}

void libapoi_log_set_file (libapoi_instance_t *inst, FILE *stream)
{
    libapoi_log_set (inst, libapoi_log_file, stream);
}
