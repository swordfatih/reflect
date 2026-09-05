cmake_minimum_required(VERSION 3.30)

project(reflect LANGUAGES CXX)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(REFLECT_BUILD_EXAMPLES "Build all example executables" OFF)
option(REFLECT_BUILD_TESTS "Build the test executable" OFF)
option(REFLECT_ENABLE_SQLITE "Build the SQLite backend" ON)
option(REFLECT_ENABLE_POSTGRESQL "Build the PostgreSQL backend" OFF)

include(FetchContent)
if(REFLECT_BUILD_TESTS)
    include(CTest)
    enable_testing()
endif()

function(reflect_link_static_runtime target_name)
    if(MINGW)
        target_link_options(${target_name} PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
endfunction()

# ------------------------
# Dependencies
# ------------------------
if(REFLECT_ENABLE_POSTGRESQL)
    FetchContent_Declare(taocpp-taopq GIT_REPOSITORY https://github.com/taocpp/taopq.git GIT_TAG main GIT_SHALLOW TRUE EXCLUDE_FROM_ALL SYSTEM)
    FetchContent_MakeAvailable(taocpp-taopq)
endif()

if(REFLECT_ENABLE_SQLITE)
    FetchContent_Declare(sl3 GIT_REPOSITORY https://github.com/a4z/libsl3.git GIT_TAG 2bd80cd259584d3afa549f3a17ac4d7d35382611 OVERRIDE_FIND_PACKAGE)
    set(sl3_USE_INTERNAL_SQLITE3 ON CACHE BOOL "Use bundled SQLite" FORCE)
    set(sl3_USE_COMMON_COMPILER_WARNINGS OFF CACHE BOOL "Use common compiler warning preset target" FORCE)
    FetchContent_MakeAvailable(sl3)
endif()

FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.14.0 GIT_SHALLOW TRUE)
if(REFLECT_BUILD_TESTS)
    FetchContent_MakeAvailable(Catch2)
endif()

# ------------------------
# Library
# ------------------------
set(cpp_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/reflect.cpp")

if(REFLECT_ENABLE_SQLITE)
    list(APPEND cpp_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite_backend.cpp")
endif()

if(REFLECT_ENABLE_POSTGRESQL)
    list(APPEND cpp_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/postgresql_backend.cpp")
endif()

add_library(${PROJECT_NAME} STATIC)
add_library(reflect::reflect ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${cpp_sources})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_26)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
if(REFLECT_ENABLE_SQLITE)
    target_link_libraries(${PROJECT_NAME} PRIVATE sl3)
    target_compile_definitions(${PROJECT_NAME} PRIVATE REFLECT_ENABLE_SQLITE)
endif()
if(REFLECT_ENABLE_POSTGRESQL)
    target_link_libraries(${PROJECT_NAME} PRIVATE taocpp::taopq)
    target_compile_definitions(${PROJECT_NAME} PRIVATE REFLECT_ENABLE_POSTGRESQL)
endif()
target_compile_options(${PROJECT_NAME}
    PUBLIC
        -freflection
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-but-set-variable
        -Wno-unused-parameter
)

# ------------------------
# Examples
# ------------------------
if(REFLECT_BUILD_EXAMPLES)
    if(NOT REFLECT_ENABLE_SQLITE)
        message(FATAL_ERROR "REFLECT_BUILD_EXAMPLES requires REFLECT_ENABLE_SQLITE because the current examples use SQLite.")
    endif()

    FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.x GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(spdlog)

    file(GLOB example_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")

    foreach(example_source IN LISTS example_sources)
        get_filename_component(example_name "${example_source}" NAME_WE)
        set(example_target "${PROJECT_NAME}_${example_name}")

        add_executable(${example_target} "${example_source}")
        set_target_properties(${example_target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/runtime")
        target_link_libraries(${example_target} PRIVATE reflect::reflect spdlog::spdlog)
        reflect_link_static_runtime(${example_target})
        target_compile_options(${example_target} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter)
    endforeach()
endif()

# ------------------------
# Tests
# ------------------------
if(REFLECT_BUILD_TESTS)
    file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

    if(NOT REFLECT_ENABLE_SQLITE)
        list(REMOVE_ITEM test_sources "${CMAKE_CURRENT_SOURCE_DIR}/tests/SqliteBackend.test.cpp")
    endif()
    if(NOT REFLECT_ENABLE_POSTGRESQL)
        list(REMOVE_ITEM test_sources "${CMAKE_CURRENT_SOURCE_DIR}/tests/PostgresqlBackend.test.cpp")
    endif()

    if(test_sources)
        add_executable(${PROJECT_NAME}_tests ${test_sources})
        set_target_properties(${PROJECT_NAME}_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/runtime")
        target_link_libraries(${PROJECT_NAME}_tests PRIVATE reflect::reflect Catch2::Catch2WithMain)
        reflect_link_static_runtime(${PROJECT_NAME}_tests)
        target_compile_options(${PROJECT_NAME}_tests PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter)
        include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
        catch_discover_tests(${PROJECT_NAME}_tests)
    endif()
endif()
