cmake_minimum_required (VERSION 3.14.4)
project (PBR_DEMO)

# Set C++ language version to C++17
set (CMAKE_CXX_STANDARD 17)

# Set debug mode
set(CMAKE_BUILD_TYPE debug)

# CONFIG VARIABLES

# Paths
set(RESOURCE_PATH "\"${PROJECT_BINARY_DIR}/res/\"")

# Memory allocation
set (DEFAULT_CONTAINER_ALLOCATOR_SIZE_BYTES 256*1024*1024)
set (DEFAULT_CONTAINER_ALLOCATOR_BLOCK_SIZE_BYTES 256)
set (DEFAULT_CONTAINER_ALLOCATOR_ALIGNMENT_BYTES 4)

# Graphics
set (NUMBER_SUPPORTED_TEXTURES 64)
set (NUMBER_SUPPORTED_MODEL_MATRICES 10)
set (NUMBER_SUPPORTED_POINTLIGHTS 4)
set (NUMBER_SUPPORTED_BOXLIGHTS 20)
set (NUMBER_SHADOWMAP_CASCADES 5)
set (NUMBER_MAX_BONES 100)

# Game
set (FRAME_RATE 60)

# END CONFIG VARIABLES

# Write to shaders
file(GLOB_RECURSE SHADERS_IN
"res/shaders/*.in"
    )
    
foreach(SHADER_IN ${SHADERS_IN})
  file(READ "${SHADER_IN}" filedata)
  string(REGEX REPLACE "@NUMBER_SUPPORTED_TEXTURES@"        "${NUMBER_SUPPORTED_TEXTURES}"      filedata "${filedata}")
  string(REGEX REPLACE "@NUMBER_SUPPORTED_MODEL_MATRICES@" "${NUMBER_SUPPORTED_MODEL_MATRICES}" filedata "${filedata}")
  string(REGEX REPLACE "@NUMBER_SUPPORTED_POINTLIGHTS@"    "${NUMBER_SUPPORTED_POINTLIGHTS}"    filedata "${filedata}")
  string(REGEX REPLACE "@NUMBER_SUPPORTED_BOXLIGHTS@"    "${NUMBER_SUPPORTED_BOXLIGHTS}"    filedata "${filedata}")
  string(REGEX REPLACE "@NUMBER_SHADOWMAP_CASCADES@"    "${NUMBER_SHADOWMAP_CASCADES}"    filedata "${filedata}")
  string(REGEX REPLACE "@NUMBER_MAX_BONES@"    "${NUMBER_MAX_BONES}"    filedata "${filedata}")
  string(REPLACE ".in" "" SHADER_OUT "${SHADER_IN}")
  file(WRITE  "${SHADER_OUT}" "${filedata}")
endforeach(SHADER_IN)

# Paths
set(RESOURCE_PATH "\"${PROJECT_BINARY_DIR}/res/\"")

# Write to shaders
file(GLOB_RECURSE SHADERS_IN
"res/shaders/*.in"
    )

# Set paths
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# Configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/config/config.h.in"
  "${PROJECT_BINARY_DIR}/src/config/config.h"
  )

# Copy resource folder to build
file(COPY res DESTINATION ${CMAKE_BINARY_DIR})
 
# add the binary tree to the search path for include files
# so that we will find config.h
include_directories(
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_SOURCE_DIR}/test"
  "${PROJECT_SOURCE_DIR}/external"
  "${PROJECT_SOURCE_DIR}/external/imgui"
  "${PROJECT_SOURCE_DIR}/external/imgui-filebrowser"
)

file(GLOB SOURCES
    "src/main/*.cpp"
    "src/graphics/*.cpp"
    "src/graphics/geometry/*.cpp"
    "src/memory/*.cpp"
    "src/container/*.cpp"
    "src/util/*.cpp"
    "src/input/*.cpp"
    # external
    "external/*/*.cpp"
    "external/imgui/backends/imgui_impl_glfw.cpp"
)

# Add libraries
# vulkan
project(vulkaninfo)
find_package(Vulkan REQUIRED)
# glfw
find_package(glfw3 3.3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
# glm
find_package(glm REQUIRED)
# assimp
find_package(ASSIMP REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIR})

# Compile shaders
#if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL <<TARGET PLATFORM>>)
set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}bin/glslangValidator")
#elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "<<ANOTHER TARGET PLATFORM>>)
#set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/PROPER/PATH/TO/GLSLANG_VALIDATOR")
#endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
"res/shaders/*.frag"
"res/shaders/*.vert"
    )
    
foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/res/shaders/${FILE_NAME}.spv")
  add_custom_command(
  OUTPUT ${SPIRV}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/res/shaders/"
  COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
  DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)
add_custom_target(
    Shaders 
DEPENDS ${SPIRV_BINARY_FILES}
    )

# Build PBR Demo
# Add the executable
add_executable(pbr_demo ${SOURCES})
# Set compiler flags
target_compile_options(pbr_demo PUBLIC -Wall -Wextra -Werror -g -fsanitize=address)
target_link_options(pbr_demo PUBLIC -Wall -Wextra -Werror -g -fsanitize=address)
# Link libraries
target_link_libraries(pbr_demo Vulkan::Vulkan)
target_link_libraries(pbr_demo glfw)
target_link_libraries(pbr_demo glm)
target_link_libraries(pbr_demo assimp::assimp)

# Add shaders to all projects
add_dependencies(pbr_demo Shaders)
