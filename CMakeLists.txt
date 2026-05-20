cmake_minimum_required(VERSION 3.30)

project(reflect LANGUAGES CXX)

option(BUILD_EXAMPLES "Build all example executables" ON)
option(BUILD_TESTS "Build the test executable" ON)

include(FetchContent)
if(BUILD_TESTS)
    include(CTest)
    enable_testing()
endif()

# ------------------------
# Dependencies
# ------------------------
FetchContent_Declare(taocpp-taopq GIT_REPOSITORY https://github.com/taocpp/taopq.git GIT_TAG main GIT_SHALLOW TRUE EXCLUDE_FROM_ALL SYSTEM)
FetchContent_MakeAvailable(taocpp-taopq)

FetchContent_Declare(sl3 GIT_REPOSITORY https://github.com/a4z/libsl3.git GIT_TAG main OVERRIDE_FIND_PACKAGE)
set(sl3_USE_COMMON_COMPILER_WARNINGS OFF CACHE BOOL "Use common compiler warning preset target" FORCE)
FetchContent_MakeAvailable(sl3)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.x GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.14.0 GIT_SHALLOW TRUE)
if(BUILD_TESTS)
    FetchContent_MakeAvailable(Catch2)
endif()

# ------------------------
# Library
# ------------------------
file(GLOB_RECURSE cpp_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} STATIC)
add_library(reflect::reflect ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${cpp_sources})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_26)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_link_libraries(${PROJECT_NAME} PRIVATE taocpp::taopq sl3)
target_compile_options(${PROJECT_NAME} PUBLIC -freflection PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter)

# ------------------------
# Examples
# ------------------------
if(BUILD_EXAMPLES)
    file(GLOB example_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")

    foreach(example_source IN LISTS example_sources)
        get_filename_component(example_name "${example_source}" NAME_WE)
        set(example_target "${PROJECT_NAME}_${example_name}")

        add_executable(${example_target} "${example_source}")
        set_target_properties(${example_target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/runtime")
        target_link_libraries(${example_target} PRIVATE reflect::reflect spdlog::spdlog)
        target_link_options(${example_target} PRIVATE -static -static-libgcc -static-libstdc++)
        target_compile_options(${example_target} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter)
        target_compile_options(${PROJECT_NAME} PRIVATE -O1)
    endforeach()
endif()

# ------------------------
# Tests
# ------------------------
if(BUILD_TESTS)
    file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

    if(test_sources)
        add_executable(${PROJECT_NAME}_tests ${test_sources})
        set_target_properties(${PROJECT_NAME}_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/runtime")
        target_link_libraries(${PROJECT_NAME}_tests PRIVATE reflect::reflect Catch2::Catch2WithMain)
        target_compile_options(${PROJECT_NAME}_tests PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter)
        include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
        catch_discover_tests(${PROJECT_NAME}_tests)
    endif()
endif()
