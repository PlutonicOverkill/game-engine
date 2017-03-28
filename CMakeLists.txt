# adapted from https://github.com/polytonic/glitter - bullet removed for now
# cmake_minimum_required(VERSION 3.0)
project(glare)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(src/lib/glfw)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(src/lib/assimp)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

set(GLARE_UNIT_TEST Glare_unit_test)
set(GLARE_TEST_SOURCES
	src/tests/test.cpp
	src/tests/test_slot_map.cpp
)

add_subdirectory(src/lib/gtest)
# enable_testing()
include_directories(src/lib/gtest/googletest/include)
add_executable(${GLARE_UNIT_TEST} ${GLARE_TEST_SOURCES})
target_link_libraries(${GLARE_UNIT_TEST} gtest)
# add_test(NAME ${GLARE_UNIT_TEST} COMMAND ${GLARE_UNIT_TEST})

# option(BUILD_BULLET2_DEMOS OFF)
# option(BUILD_CPU_DEMOS OFF)
# option(BUILD_EXTRAS OFF)
# option(BUILD_OPENGL3_DEMOS OFF)
# option(BUILD_UNIT_TESTS OFF)
# add_subdirectory(src/lib/bullet)

if(MSVC)
	# disable stupid size_t conversion warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /wd4267 /wd4100")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++17 ")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(
	src/main/
	src/lib/assimp/include/
	# src/lib/bullet/src/
	src/lib/glad/include/
	src/lib/glfw/include/
	src/lib/glm/
	src/lib/stb/
)
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/src/lib/glad/src/glad.c)
execute_process(
	COMMAND python -m glad
		--profile=core
		--out-path=./
		--generator=c-debug
		--extensions=../../../GL_EXTENSIONS.txt
		--omit-khrplatform
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/lib/glad
)
endif()

set(VENDORS_SOURCES
	src/lib/glad/src/glad.c
)

set(PROJECT_HEADERS
	src/glare/ecs.hpp
	src/glare/error.hpp
	src/glare/glare.hpp
	src/glare/slot_map.hpp
	src/glare/utility.hpp
	src/glare/video.hpp
)

set(PROJECT_SOURCES
	src/game/main.cpp
)

set(PROJECT_SHADERS
	src/shaders/main.frag
	src/shaders/main.vert
)

set(PROJECT_CONFIGS
	CMakeLists.txt
	Readme.md
	.gitattributes
	.gitignore
	.gitmodules
)

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Vendors" FILES ${VENDORS_SOURCES})
source_group("Shaders" FILES ${PROJECT_SHADERS})
source_group("Configs" FILES ${PROJECT_CONFIGS})

add_definitions(
	-DGLFW_INCLUDE_NONE
	-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"
)

add_executable(
	${PROJECT_NAME}
	${PROJECT_SOURCES}
	${PROJECT_HEADERS}
	${PROJECT_CONFIGS}
	${VENDORS_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
	assimp
	glfw
	${GLFW_LIBRARIES}
	${GLAD_LIBRARIES}
	# BulletDynamics
	#BulletCollision
	#LinearMath
)

set(GLARE_INSTALL_DIR ${CMAKE_BINARY_DIR}/bin)

set_target_properties(${PROJECT_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${GLARE_INSTALL_DIR}/${PROJECT_NAME}
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GLARE_INSTALL_DIR}/${PROJECT_NAME}
)
set_target_properties(${GLARE_UNIT_TEST} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${GLARE_INSTALL_DIR}/test
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GLARE_INSTALL_DIR}/test
)

ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory ${GLARE_INSTALL_DIR}/${PROJECT_NAME}/shaders
          # COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SHADERS} ${GLARE_INSTALL_DIR}/${PROJECT_NAME}/shaders
		  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/src/shaders ${GLARE_INSTALL_DIR}/${PROJECT_NAME}/shaders
)