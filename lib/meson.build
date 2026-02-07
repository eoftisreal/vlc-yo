libapoi_sources = [
    'core.c',
    'dialog.c',
    'renderer_discoverer.c',
    'error.c',
    'log.c',
    'video.c',
    'audio.c',
    'event.c',
    'media.c',
    'media_track.c',
    'media_player.c',
    'media_list.c',
    'media_list_path.h',
    'media_list_player.c',
    'media_discoverer.c',
    'picture.c'
]

libapoi_cargs = []
if host_system in ['windows', 'os/2']
    libapoi_cargs += ['-DLIBAPOI_DLL_EXPORT']
endif

libapoi = library('vlc',
    libapoi_sources, rev_target,
    include_directories: [vlc_include_dirs],
    c_args: libapoi_cargs,
    link_with: [vlc_libcompat],
    dependencies: [libapoicore_dep, m_lib, threads_dep],
    install: true
)
