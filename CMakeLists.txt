cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_COLOR_DIAGNOSTICS true)

project(app)

# Add a custom target for building Debug
add_custom_target(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug -B ${CMAKE_BINARY_DIR}/debug -G "MinGW Makefiles"
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/debug
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Add a custom target for building Release
add_custom_target(release
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release -B ${CMAKE_BINARY_DIR}/release -G "MinGW Makefiles"
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Custom command for cleaning debug
add_custom_target(clean-debug
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/debug
)

# Custom command for cleaning release
add_custom_target(clean-release
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/release
)

# Add ImGui Files to source
file(GLOB IMGUISOURCES
    ${CMAKE_SOURCE_DIR}/include/rlimgui/*.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Building for debug")
else()
    message("Building for release")
endif()

# Set up executable
if(WIN32)
    message("Setting up Windows executable...")
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libstdc++ -static-libgcc")
    add_definitions(-D_THREAD_SAFE)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_executable(app WIN32 src/main.cpp ${IMGUISOURCES})
    else()
        add_executable(app src/main.cpp ${IMGUISOURCES})
    endif()
    
else()
    add_executable(app src/main.cpp ${IMGUISOURCES})
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/include/rlimgui)

# Copy assests to binary directory
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Copying game files")
    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY maps DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY packs DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Link with MacOS libraries
if(APPLE)
    message("Building for MacOS")
    
    target_link_libraries(app "-framework CoreVideo")
    target_link_libraries(app "-framework IOKit")
    target_link_libraries(app "-framework Cocoa")
    target_link_libraries(app "-framework GLUT")
    target_link_libraries(app "-framework OpenGL")

    target_link_libraries(app ${CMAKE_SOURCE_DIR}/lib/macos/libraylib.a)
endif()

# Link with Windows libraries
if(WIN32)
    message("Building for Windows...")
    target_link_libraries(app ${CMAKE_SOURCE_DIR}/lib/win/libraylib.a winmm gdi32)
endif()