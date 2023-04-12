
include(FindPackageHandleStandardArgs)

FIND_PATH(screen_INCLUDE_DIR
  screen/screen.h
)
FIND_LIBRARY(screen_LIBRARY
  NAMES screen
)


find_package_handle_standard_args(screen DEFAULT_MSG
                                  screen_LIBRARY
                                  screen_INCLUDE_DIR)

mark_as_advanced(screen_INCLUDE_DIR screen_LIBRARY )

set(screen_LIBRARIES ${screen_LIBRARY} )
set(screen_INCLUDE_DIRS ${screen_INCLUDE_DIR} )


if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
    # Target approach
    if(NOT TARGET screen::screen)
        add_library(screen::screen INTERFACE IMPORTED)
    if(screen_INCLUDE_DIRS)
      set_target_properties(screen::screen PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
        "${screen_INCLUDE_DIRS}")
    endif()
    set_property(TARGET screen::screen PROPERTY INTERFACE_LINK_LIBRARIES
      "${screen_LIBRARIES}")
    endif()
endif()
