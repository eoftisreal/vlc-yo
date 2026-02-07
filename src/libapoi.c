/*****************************************************************************
 * libapoi.c: libapoi instances creation and deletion, interfaces handling
 *****************************************************************************
 * Copyright (C) 1998-2008 VLC authors and VideoLAN
 *
 * Authors: Vincent Seguin <seguin@via.ecp.fr>
 *          Samuel Hocevar <sam@zoy.org>
 *          Gildas Bazin <gbazin@videolan.org>
 *          Derk-Jan Hartman <hartman at videolan dot org>
 *          Rémi Denis-Courmont
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

/** \file
 * This file contains functions to create and destroy libapoi instances
 */

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_preparser.h>
#include "../lib/libapoi_internal.h"

#include "modules/modules.h"
#include "config/configuration.h"
#include "media_source/media_source.h"

#include <stdio.h>                                              /* sprintf() */
#include <string.h>
#include <stdlib.h>                                                /* free() */
#include <errno.h>

#include "config/vlc_getopt.h"

#include <vlc_playlist.h>
#include <vlc_interface.h>

#include <vlc_actions.h>
#include <vlc_charset.h>
#include <vlc_dialog.h>
#include <vlc_keystore.h>
#include <vlc_fs.h>
#include <vlc_cpu.h>
#include <vlc_url.h>
#include <vlc_modules.h>
#include <vlc_media_library.h>
#include <vlc_tracer.h>
#include "player/player.h"

#include "libapoi.h"

#include <vlc_vlm.h>

#include <assert.h>

/*****************************************************************************
 * Local prototypes
 *****************************************************************************/
static void GetFilenames  ( libapoi_int_t *, unsigned, const char *const [] );

/**
 * Allocate a blank libapoi instance, also setting the exit handler.
 * Vlc's threading system must have been initialized first
 */
libapoi_int_t * libapoi_InternalCreate( void )
{
    libapoi_int_t *p_libapoi;
    libapoi_priv_t *priv;

    /* Allocate a libapoi instance object */
    p_libapoi = (vlc_custom_create)( NULL, sizeof (*priv), "libapoi" );
    if( p_libapoi == NULL )
        return NULL;

    priv = libapoi_priv (p_libapoi);
    vlc_mutex_init(&priv->lock);
    priv->interfaces = NULL;
    priv->main_playlist = NULL;
    priv->p_vlm = NULL;
    priv->media_source_provider = NULL;

    vlc_ExitInit( &priv->exit );

    return p_libapoi;
}

static void libapoi_AddInterfaces(libapoi_int_t *libapoi, const char *varname)
{
    char *str = var_InheritString(libapoi, varname);
    if (str == NULL)
        return;

    char *state;
    char *intf = strtok_r(str, ":", &state);

    while (intf != NULL) {
        libapoi_InternalAddIntf(libapoi, intf);
        intf = strtok_r(NULL, ":", &state);
    }

    free(str);
}

/**
 * Initialize a libapoi instance
 * This function initializes a previously allocated libapoi instance:
 *  - CPU detection
 *  - gettext initialization
 *  - message queue, module bank and playlist initialization
 *  - configuration and commandline parsing
 */
int libapoi_InternalInit( libapoi_int_t *p_libapoi, int i_argc,
                         const char *ppsz_argv[] )
{
    libapoi_priv_t *priv = libapoi_priv (p_libapoi);
    char        *psz_val;
    int          i_ret = VLC_EGENERIC;

    if (unlikely(vlc_LogPreinit(p_libapoi)))
        return VLC_ENOMEM;

    /* System specific initialization code */
    system_Init();

    /*
     * Initialize the module bank and load the core config only.
     */
    module_InitBank ();

    /*
     * Perform early check for commandline arguments that affect module loading
     * or vlc_threads_setup()
     */
    config_CmdLineEarlyScan( p_libapoi, i_argc, ppsz_argv );

    vlc_threads_setup (p_libapoi);

    /*
     * Load plugin data into the module bank.
     * We need to do this here such that option sets from plugins are added to
     * the config system in order that full commandline argument parsing and
     * saved settings handling can function properly.
     */
    module_LoadPlugins (p_libapoi);

    /*
     * Fully process command line settings.
     * Results are stored as runtime state within `p_libapoi` object variables.
     */
    int vlc_optind;
    if( config_LoadCmdLine( p_libapoi, i_argc, ppsz_argv, &vlc_optind ) )
        goto error;

    /*
     * Load saved settings into the config system, as applicable.
     */
    if( !var_InheritBool( p_libapoi, "ignore-config" ) )
    {
        if( var_InheritBool( p_libapoi, "reset-config" ) )
            config_SaveConfigFile( p_libapoi ); /* Save default config */
        else
            config_LoadConfigFile( p_libapoi );
    }

    vlc_LogInit(p_libapoi);

    char *tracer_name = var_InheritString(p_libapoi, "tracer");
    priv->tracer = vlc_tracer_Create(VLC_OBJECT(p_libapoi), tracer_name);
    free(tracer_name);

    /*
     * Support for gettext
     */
#if defined( ENABLE_NLS ) \
     && ( defined( HAVE_GETTEXT ) || defined( HAVE_INCLUDED_GETTEXT ) )
    vlc_bindtextdomain (PACKAGE_NAME);
#endif
    /*xgettext: Translate "C" to the language code: "fr", "en_GB", "nl", "ru"... */
    msg_Dbg( p_libapoi, "translation test: code is \"%s\"", _("C") );

    /*
     * Handle info requests such as for help or version text.
     */
    if (config_PrintHelp (p_libapoi))
    {
        libapoi_InternalCleanup (p_libapoi);
        exit(0);
    }

    i_ret = VLC_ENOMEM;

    if( libapoi_InternalDialogInit( p_libapoi ) != VLC_SUCCESS )
        goto error;
    if( libapoi_InternalKeystoreInit( p_libapoi ) != VLC_SUCCESS )
        msg_Warn( p_libapoi, "memory keystore init failed" );

    vlc_CPU_dump( VLC_OBJECT(p_libapoi) );

    if( var_InheritBool( p_libapoi, "media-library") )
    {
        priv->p_media_library = libapoi_MlCreate( p_libapoi );
        if ( priv->p_media_library == NULL )
            msg_Warn( p_libapoi, "Media library initialization failed" );
    }

    /*
     * Initialize hotkey handling
     */
    if( libapoi_InternalActionsInit( p_libapoi ) != VLC_SUCCESS )
        goto error;

    /*
     * Meta data handling
     */

    priv->media_source_provider = vlc_media_source_provider_New( VLC_OBJECT( p_libapoi ) );
    if( !priv->media_source_provider )
        goto error;

    /* variables for signalling creation of new files */
    var_Create( p_libapoi, "snapshot-file", VLC_VAR_STRING );
    var_Create( p_libapoi, "record-file", VLC_VAR_STRING );

    /* some default internal settings */
    var_Create( p_libapoi, "window", VLC_VAR_STRING );
    var_Create( p_libapoi, "vout-cb-type", VLC_VAR_INTEGER );

    /* NOTE: Because the playlist and interfaces start before this function
     * returns control to the application (DESIGN BUG!), all these variables
     * must be created (in place of libapoi_new()) and set to VLC defaults
     * (in place of VLC main()) *here*. */
    var_Create( p_libapoi, "user-agent", VLC_VAR_STRING );
    var_SetString( p_libapoi, "user-agent",
                   "APOI media player (LibAPOI "VERSION")" );
    var_Create( p_libapoi, "http-user-agent", VLC_VAR_STRING );
    var_SetString( p_libapoi, "http-user-agent",
                   "APOI/"PACKAGE_VERSION" LibAPOI/"PACKAGE_VERSION );
    var_Create( p_libapoi, "app-icon-name", VLC_VAR_STRING );
    var_SetString( p_libapoi, "app-icon-name", PACKAGE_NAME );
    var_Create( p_libapoi, "app-id", VLC_VAR_STRING );
    var_SetString( p_libapoi, "app-id", "org.apoi.apoi" );
    var_Create( p_libapoi, "app-version", VLC_VAR_STRING );
    var_SetString( p_libapoi, "app-version", PACKAGE_VERSION );

    /* System specific configuration */
    system_Configure( p_libapoi, i_argc - vlc_optind, ppsz_argv + vlc_optind );

#ifdef ENABLE_VLM
    /* Initialize VLM if vlm-conf is specified */
    char *psz_parser = var_InheritString( p_libapoi, "vlm-conf" );
    if( psz_parser )
    {
        priv->p_vlm = vlm_New( p_libapoi, psz_parser );
        if( !priv->p_vlm )
            msg_Err( p_libapoi, "VLM initialization failed" );
        free( psz_parser );
    }
#endif

    /*
     * Load background interfaces
     */
    libapoi_AddInterfaces(p_libapoi, "extraintf");
    libapoi_AddInterfaces(p_libapoi, "control");

#ifdef __APPLE__
    var_Create( p_libapoi, "drawable-view-top", VLC_VAR_INTEGER );
    var_Create( p_libapoi, "drawable-view-left", VLC_VAR_INTEGER );
    var_Create( p_libapoi, "drawable-view-bottom", VLC_VAR_INTEGER );
    var_Create( p_libapoi, "drawable-view-right", VLC_VAR_INTEGER );
    var_Create( p_libapoi, "drawable-clip-top", VLC_VAR_INTEGER );
    var_Create( p_libapoi, "drawable-clip-left", VLC_VAR_INTEGER );
    var_Create( p_libapoi, "drawable-clip-bottom", VLC_VAR_INTEGER );
    var_Create( p_libapoi, "drawable-clip-right", VLC_VAR_INTEGER );
    var_Create( p_libapoi, "drawable-nsobject", VLC_VAR_ADDRESS );
#endif

    /*
     * Get input filenames given as commandline arguments.
     * We assume that the remaining parameters are filenames
     * and their input options.
     */
    GetFilenames( p_libapoi, i_argc - vlc_optind, ppsz_argv + vlc_optind );

    /*
     * Get --open argument
     */
    psz_val = var_InheritString( p_libapoi, "open" );
    if ( psz_val != NULL )
    {
        intf_InsertItem( p_libapoi, psz_val, 0, NULL, 0 );
        free( psz_val );
    }

    /* Callbacks between interfaces */

    /* Create a variable for showing the right click menu */
    var_Create(p_libapoi, "intf-popupmenu", VLC_VAR_BOOL);

    /* Create a variable for showing the fullscreen interface */
    var_Create(p_libapoi, "intf-toggle-fscontrol", VLC_VAR_VOID);

    /* Create a variable for the Boss Key */
    var_Create(p_libapoi, "intf-boss", VLC_VAR_VOID);

    /* Create a variable for showing the main interface */
    var_Create(p_libapoi, "intf-show", VLC_VAR_VOID);

    return VLC_SUCCESS;

error:
    libapoi_InternalCleanup( p_libapoi );
    return i_ret;
}

/**
 * Cleanup a libapoi instance. The instance is not completely deallocated
 * \param p_libapoi the instance to clean
 */
void libapoi_InternalCleanup( libapoi_int_t *p_libapoi )
{
    libapoi_priv_t *priv = libapoi_priv (p_libapoi);

    /* Ask the interfaces to stop and destroy them */
    msg_Dbg( p_libapoi, "removing all interfaces" );
    intf_DestroyAll( p_libapoi );

#ifdef ENABLE_VLM
    /* Destroy VLM if created in libapoi_InternalInit */
    if( priv->p_vlm )
    {
        vlm_Delete( priv->p_vlm );
    }
#endif

#if !defined( _WIN32 ) && !defined( __OS2__ )
    char *pidfile = var_InheritString( p_libapoi, "pidfile" );
    if( pidfile != NULL )
    {
        msg_Dbg( p_libapoi, "removing PID file %s", pidfile );
        if( unlink( pidfile ) )
            msg_Warn( p_libapoi, "cannot remove PID file %s: %s",
                      pidfile, vlc_strerror_c(errno) );
        free( pidfile );
    }
#endif

    if (priv->main_playlist)
        vlc_playlist_Delete(priv->main_playlist);

    if ( priv->p_media_library )
        libapoi_MlRelease( priv->p_media_library );

    if( priv->media_source_provider )
        vlc_media_source_provider_Delete( priv->media_source_provider );

    libapoi_InternalDialogClean( p_libapoi );
    libapoi_InternalKeystoreClean( p_libapoi );
    libapoi_InternalActionsClean( p_libapoi );

    /* Save the configuration */
    if( !var_InheritBool( p_libapoi, "ignore-config" ) )
        config_AutoSaveConfigFile( p_libapoi );

    vlc_LogDestroy(p_libapoi->obj.logger);
    if (priv->tracer != NULL)
        vlc_tracer_Destroy(priv->tracer);
    /* Free module bank. It is refcounted, so we call this each time  */
    module_EndBank (true);
#if defined(_WIN32) || defined(__OS2__)
    system_End( );
#endif
}

/**
 * Destroy libapoi instance.
 * \param p_libapoi the instance to destroy
 */
void libapoi_InternalDestroy( libapoi_int_t *p_libapoi )
{
    vlc_object_delete(p_libapoi);
}

/*****************************************************************************
 * GetFilenames: parse command line options which are not flags
 *****************************************************************************
 * Parse command line for input files as well as their associated options.
 * An option always follows its associated input and begins with a ":".
 *****************************************************************************/
static void GetFilenames( libapoi_int_t *p_vlc, unsigned n,
                          const char *const args[] )
{
    while( n > 0 )
    {
        /* Count the input options */
        unsigned i_options = 0;

        while( args[--n][0] == ':' )
        {
            i_options++;
            if( n == 0 )
            {
                msg_Warn( p_vlc, "options %s without item", args[n] );
                return; /* syntax!? */
            }
        }

        char *mrl = NULL;
        if( strstr( args[n], "://" ) == NULL )
        {
            mrl = vlc_path2uri( args[n], NULL );
            if( !mrl )
                continue;
        }

        intf_InsertItem( p_vlc, (mrl != NULL) ? mrl : args[n], i_options,
                         ( i_options ? &args[n + 1] : NULL ),
                         VLC_INPUT_OPTION_TRUSTED );
        free( mrl );
    }
}

static void
PlaylistConfigureFromVariables(vlc_playlist_t *playlist, vlc_object_t *obj)
{
    enum vlc_playlist_playback_order order;
    if (var_InheritBool(obj, "random"))
        order = VLC_PLAYLIST_PLAYBACK_ORDER_RANDOM;
    else
        order = VLC_PLAYLIST_PLAYBACK_ORDER_NORMAL;

    /* repeat = repeat current; loop = repeat all */
    enum vlc_playlist_playback_repeat repeat;
    if (var_InheritBool(obj, "repeat"))
        repeat = VLC_PLAYLIST_PLAYBACK_REPEAT_CURRENT;
    else if (var_InheritBool(obj, "loop"))
        repeat = VLC_PLAYLIST_PLAYBACK_REPEAT_ALL;
    else
        repeat = VLC_PLAYLIST_PLAYBACK_REPEAT_NONE;

    enum vlc_playlist_media_stopped_action media_stopped_action;
    if (var_InheritBool(obj, "play-and-exit"))
        media_stopped_action = VLC_PLAYLIST_MEDIA_STOPPED_EXIT;
    else if (var_InheritBool(obj, "play-and-stop"))
        media_stopped_action = VLC_PLAYLIST_MEDIA_STOPPED_STOP;
    else
        media_stopped_action = VLC_PLAYLIST_MEDIA_STOPPED_CONTINUE;

    bool start_paused = var_InheritBool(obj, "start-paused");
    bool playlist_cork = var_InheritBool(obj, "playlist-cork");
    bool play_and_pause = var_InheritBool(obj, "play-and-pause");
    unsigned repeat_count = var_InheritInteger(obj, "input-repeat");

    vlc_playlist_Lock(playlist);
    vlc_playlist_SetPlaybackOrder(playlist, order);
    vlc_playlist_SetPlaybackRepeat(playlist, repeat);
    vlc_playlist_SetMediaStoppedAction(playlist, media_stopped_action);

    vlc_player_t *player = vlc_playlist_GetPlayer(playlist);

    /* the playlist and the player share the same lock, and this is not an
     * implementation detail */
    vlc_player_SetStartPaused(player, start_paused);
    vlc_player_SetPauseOnCork(player, playlist_cork);
    vlc_player_SetPlayAndPause(player, play_and_pause);
    vlc_player_SetRepeatCount(player, repeat_count);

    vlc_playlist_Unlock(playlist);
}

vlc_playlist_t *
libapoi_GetMainPlaylist(libapoi_int_t *libapoi)
{
    libapoi_priv_t *priv = libapoi_priv(libapoi);

    vlc_mutex_lock(&priv->lock);
    vlc_playlist_t *playlist = priv->main_playlist;
    if (priv->main_playlist == NULL)
    {
        bool auto_preparse = var_InheritBool(libapoi, "auto-preparse");
        enum vlc_playlist_preparsing rec = VLC_PLAYLIST_PREPARSING_DISABLED;
        int max_threads = 1;
        vlc_tick_t default_timeout = 0;

        if (auto_preparse)
        {
            rec = VLC_PLAYLIST_PREPARSING_COLLAPSE;

            char *rec_str = var_InheritString(libapoi, "recursive");
            if (rec_str != NULL)
            {
                if (!strcasecmp(rec_str, "none"))
                    rec = VLC_PLAYLIST_PREPARSING_ENABLED;
                else if (!strcasecmp(rec_str, "expand"))
                    rec = VLC_PLAYLIST_PREPARSING_RECURSIVE;
                free(rec_str);
            }
            max_threads = var_InheritInteger(libapoi, "preparse-threads");
            if (max_threads < 1)
                max_threads = 1;

            default_timeout =
                VLC_TICK_FROM_MS(var_InheritInteger(libapoi, "preparse-timeout"));
            if (default_timeout < 0)
                default_timeout = 0;
        }

        playlist = priv->main_playlist = vlc_playlist_New(VLC_OBJECT(libapoi),
                                                          rec, max_threads,
                                                          default_timeout);
        if (playlist)
            PlaylistConfigureFromVariables(playlist, VLC_OBJECT(libapoi));
    }
    vlc_mutex_unlock(&priv->lock);

    return playlist;
}
