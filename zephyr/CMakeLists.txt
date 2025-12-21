if(CONFIG_CTRL_FREAK_LITE)
    zephyr_library()

    zephyr_library_sources(
        ../src/cfl.c
        ../src/cfl_service.c
    )

    zephyr_library_sources_ifdef(CONFIG_SHELL
        ../src/cfl_shell.c
    )

    zephyr_include_directories(
        ../include
        ../src
    )
endif()
