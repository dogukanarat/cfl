if(CONFIG_CFL)
    add_library(cfl_lib INTERFACE)

    target_include_directories(cfl_lib INTERFACE ../include)

    target_link_libraries(zephyr_interface INTERFACE cfl_lib)
endif()
