cmake_minimum_required (VERSION 2.8)
project (libobject)

#find_package(SDL2 SDL2_ttf REQUIRED)
#option (USE_MYMATH
        #"Use tutorial provided math implementation" ON)

# The version number.
set (LIBOBJECT_VERSION_MAJOR 1)
set (LIBOBJECT_VERSION_MINOR 0)

set(Source_Files)
# Set the output folder where your program will be created
set (CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
# Debug/Release settings
set (CMAKE_BUILD_TYPE "Debug")
set (CMAKE_C_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -Wno-return-type -Wno-unused-variable -Wno-unused-function -Wno-sometimes-uninitialized -g -Wno-char-subscripts -Wno-sometimes-uninitialized -Wno-unused-label -Wno-uninitialized -Wno-int-conversion -Wno-implicit-function-declaration -Wno-uninitialized -ggdb")
set (CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

include_directories(src/include)

add_subdirectory(src)
#message("Source_Files:${Source_Files}")
add_executable(test ${Source_Files})
target_link_libraries(test SDL2)
target_link_libraries(test SDL2_ttf)

ADD_LIBRARY (object SHARED ${Source_Files})
SET_TARGET_PROPERTIES (object PROPERTIES VERSION 1.2 SOVERSION 1)
target_link_libraries(object SDL2)
target_link_libraries(object SDL2_ttf)

install (TARGETS test DESTINATION bin)
