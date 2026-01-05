if(CONFIG_CFL)
    zephyr_library()

    zephyr_library_sources(
        ../src/cfl.c
        ../src/services/cfl_service_danp.c
    )

    zephyr_library_sources_ifdef(CONFIG_SHELL
        ../src/arch/zephyr/cfl_shell.c
    )

    zephyr_include_directories(
        ../include
        ../src
    )
endif()
