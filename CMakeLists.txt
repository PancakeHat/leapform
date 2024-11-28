cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_COLOR_DIAGNOSTICS true)

project(app)

# Add ImGui Files to source
file(GLOB IMGUISOURCES
    ${CMAKE_SOURCE_DIR}/include/rlimgui/*.cpp
)

# if(WIN32)
#     add_executable(app WIN32 src/main.cpp ${IMGUISOURCES})
# else()
#     add_executable(app src/main.cpp ${IMGUISOURCES})
# endif()

add_executable(app src/main.cpp ${IMGUISOURCES})

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/include/rlimgui)

# Copy assests to binary directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY maps DESTINATION ${CMAKE_BINARY_DIR})

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
    message("Building for Windows")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    target_link_libraries(app ${CMAKE_SOURCE_DIR}/lib/win/libraylib.a winmm gdi32)
endif()