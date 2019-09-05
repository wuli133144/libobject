cmake_minimum_required (VERSION 2.8)
include (mk/util.cmake)
init_platform()
project (libobject)
config_platform()

# The version number.
set (LIBOBJECT_VERSION_MAJOR 1)
set (LIBOBJECT_VERSION_MINOR 0)
# Set the output folder where your program will be created
set (CMAKE_MACOSX_RPATH 1)
# Debug/Release settings
set (CMAKE_BUILD_TYPE "Debug")

display_configs("${PLATFORM}")

#find_package(PkgConfig)
#find_package(opengl)
#find_package(CURL)

option (MODULE_UI "ui module" OFF)
option (MODULE_BUS "bus module" OFF)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    if(CMAKE_COMPILER_IS_GNUCC)
        # add compile paramater 
        add_compile_options( 
        -std=gnu99 -g -fPIC -Wall -Wextra
        -Wno-unused-but-set-variable 
        -Wno-unused-variable
        -Wno-unused-function
        -Wno-char-subscripts
        -Wno-unused-label
        -Wno-return-type
        -Wno-uninitialized
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -Wno-implicit-function-declaration)
    else()
    message(ERROR_FATAL,"compiler_c not support exit")
    endif(CMAKE_COMPILER_GNUCC)
endif()

#add_subdirectory(src)
find_source_files(Source_Files)
find_main_file(Main_File)

if (Main_File)
    list(REMOVE_ITEM Source_Files ${Main_File} )
    message("-- Main file path: ${Main_File}")
endif ()

if (MODULE_UI)
    message("-- Turn ui module on")
    #find_package(SDL2 REQUIRED)
    SET(ExternalLibs ${ExternalLibs} SDL2 SDL2_ttf)
else ()
    message(WARNING "-- Turn ui module off-----------")
    find_module_files(Exclude_UI_Module_Files ui)
    #message(ERROR_FATAL "${Exclude_UI_Module_Files}")
    #message(ERROR_FATAL "${Source_Files}")
    exclude_files(Source_Files "${Source_Files}" "${Exclude_UI_Module_Files}")
    #message("sources exclude ui module: ${Source_Files}")
endif ()

if (MODULE_BUS)
    message("-- Turn bus module on")
    find_package(SDL2 REQUIRED)
else ()
    message("-- Turn bus module off")
    find_module_files(Exclude_BUS_Module_Files bus)
    exclude_files(Source_Files "${Source_Files}" "${Exclude_BUS_Module_Files}")
endif ()

ADD_LIBRARY(object SHARED ${Source_Files})

if(EXISTS "${ExternalLibs}")
    message(STATUS "depend_lib:${ExternalLibs}")
    target_link_libraries(object ${ExternalLibs})
else()
    message(STATUS "not found depend_lib ExternalLibs")
    SET(ExternalLibs ${ExternalLibs} pthread m json)
    target_link_libraries(object ${ExternalLibs})
endif(EXISTS "${ExternalLibs}")

add_executable(test ${Main_File})
target_link_libraries(test object)

#add_executable(test ${Main_File} ${Source_Files})
#target_link_libraries(test ${ExternalLibs})

#set_target_properties(object PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(object_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES (object PROPERTIES VERSION 1.2 SOVERSION 1)

install (TARGETS test DESTINATION bin)
install (DIRECTORY ${LIBRARY_DIR_PATH} DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)