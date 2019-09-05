function (test arg)
    message(${arg})
endfunction()

function (find_source_files ret)
    file(GLOB_RECURSE files ${PROJECT_SOURCE_DIR}/src/*.c)
    set(${ret} ${files} PARENT_SCOPE)
endfunction()

function (find_main_file ret)
    file(GLOB_RECURSE file ${PROJECT_SOURCE_DIR}/src main.c)
    set(${ret} ${file} PARENT_SCOPE)
endfunction()

function (find_module_files ret module_name)
    file(GLOB_RECURSE files ${PROJECT_SOURCE_DIR}/src/${module_name}/*.c)
    set(${ret} ${files} PARENT_SCOPE)
endfunction()

function (exclude_files ret source filted_files)
  #message("filted_files: ${filted_files}")
  #message("pre: ${source}")
  list( REMOVE_ITEM source ${filted_files} )
  #message("abc: ${source}")
  set(${ret} ${source} PARENT_SCOPE)
endfunction()

macro (init_platform)
    if (DEFINED ANDROID_PLATFORM)
        set (PLATFORM android) 
        message("set platform to : ${PLATFORM}")
    endif()

    message("platform: ${PLATFORM}")

    if ("${PLATFORM}" STREQUAL "mac")
        include (mk/mac.cmake)
        add_definitions(-DMAC_USER_MODE)
        message("mac Platform")
    elseif ("${PLATFORM}" STREQUAL "android")
        include (mk/android.cmake)
        set_android_environment_variable()
        add_definitions(-DANDROID_USER_MODE)
        message("android Platform")
    elseif ("${PLATFORM}" STREQUAL "ios")
        include (mk/ios.cmake)
        add_definitions(-DIOS_USER_MODE)
        message("ios Platform")
    elseif ("${PLATFORM}" STREQUAL "linux")
        include (mk/linux.cmake)
        add_definitions(-DLINUX_USER_MODE)
        message("linux Platform")
    else ()
        include (mk/mac.cmake)
        add_definitions(-DMAC_USER_MODE)
        message("default Platform")
    endif()
endmacro()

macro (config_platform)
    if ("${PLATFORM}" STREQUAL "mac")
        set_cmake_evironment_variable()
    elseif ("${PLATFORM}" STREQUAL "android")
        set_cmake_evironment_variable()
    elseif ("${PLATFORM}" STREQUAL "ios")
        set_cmake_evironment_variable()
    elseif ("${PLATFORM}" STREQUAL "linux")
        set_cmake_evironment_variable()
    else ()
        set_cmake_evironment_variable()
    endif()
endmacro()

macro (display_configs)
    if ("${PLATFORM}" STREQUAL "mac")
        display_mac_platform_configs()
    elseif ("${PLATFORM}" STREQUAL "android")
        display_android_platform_configs()
    elseif ("${PLATFORM}" STREQUAL "ios")
        display_ios_platform_configs()
    elseif ("${PLATFORM}" STREQUAL "linux")
        display_linux_platform_configs()
    else ()
        display_mac_platform_configs()
    endif()
endmacro()
