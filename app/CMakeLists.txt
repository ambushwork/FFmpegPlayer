# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
file(GLOB SOURCE src/main/cpp/*.cpp)
#aux_source_directory()

add_subdirectory(src/main/cpp/librtmp)

add_library(
             ffmpegplayer
             SHARED
             ${SOURCE})
include_directories(src/main/cpp/include
        librtmp)

set(my_lib_path ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${my_lib_path}")

find_library(
              log-lib
              log )
target_link_libraries( # Specifies the target library.
                ffmpegplayer
                rtmp
                -Wl,--start-group
                avformat avcodec avutil swresample swscale avfilter
                -Wl,--end-group
        # Links the target library to the log library
               # included in the NDK.
               ${log-lib}
                z
                OpenSLES
                 android)