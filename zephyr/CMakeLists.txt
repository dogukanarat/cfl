if(CONFIG_CFL)
    zephyr_library()

    zephyr_library_sources(
        ../src/cfl.c
    )

    zephyr_include_directories(
        ../include
        ../src
    )
endif()
