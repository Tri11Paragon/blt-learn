cmake_minimum_required(VERSION 3.25)

macro(sanitizers target_name)
    if (${ENABLE_ADDRSAN} MATCHES ON)
        target_compile_options(${target_name} PRIVATE -fsanitize=address)
        target_link_options(${target_name} PRIVATE -fsanitize=address)
    endif ()

    if (${ENABLE_UBSAN} MATCHES ON)
        target_compile_options(${target_name} PRIVATE -fsanitize=undefined)
        target_link_options(${target_name} PRIVATE -fsanitize=undefined)
    endif ()

    if (${ENABLE_TSAN} MATCHES ON)
        target_compile_options(${target_name} PRIVATE -fsanitize=thread)
        target_link_options(${target_name} PRIVATE -fsanitize=thread)
    endif ()
endmacro()

macro(compile_options target_name)
    if (NOT ${MOLD} STREQUAL MOLD-NOTFOUND)
        target_compile_options(${target_name} PUBLIC -fuse-ld=mold)
    endif ()

    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)
    target_link_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)
    sanitizers(${target_name})
endmacro()

macro(blt_add_project name source type)

    project(${name}-${type})

    add_executable(${name}-${type} ${source})

    target_link_libraries(${name}-${type} PRIVATE BLT blt-gp Threads::Threads)

    compile_options(${name}-${type})
    target_compile_definitions(${name}-${type} PRIVATE BLT_DEBUG_LEVEL=${DEBUG_LEVEL})

    if (${TRACK_ALLOCATIONS})
        target_compile_definitions(${name}-${type} PRIVATE BLT_TRACK_ALLOCATIONS=1)
    endif ()

    add_test(NAME ${name} COMMAND ${name}-${type})

    set_property(TEST ${name} PROPERTY FAIL_REGULAR_EXPRESSION "FAIL;ERROR;FATAL;exception")

    project(blt-learn)
endmacro()

function(has_main INPUT_FILE OUT_VAR)
    if(NOT EXISTS "${INPUT_FILE}")
        message(FATAL_ERROR "File not found: ${INPUT_FILE}")
    endif()

    file(READ "${INPUT_FILE}" _contents)

    set(_main_regex
            "main\(.*\)"
    )
    string(REGEX MATCH ${_main_regex} svr ${_contents})
    if(svr)
        set(${OUT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${OUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

project(blt-learn VERSION 0.0.4)

message(STATUS "Building runtime with C++ compiler: ${CMAKE_CXX_COMPILER}")

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)
option(BUILD_BLT_LEARN_EXAMPLES "Build example programs. This will build with CTest" OFF)
option(BUILD_BLT_LEARN_TESTS "Build test programs. This will build with CTest" OFF)

add_subdirectory(lib/blt-with-graphics)

include_directories(include/)
file(GLOB_RECURSE PROJECT_BUILD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/lib/*.cpp")
file(GLOB_RECURSE RUNTIME_FILES "${CMAKE_CURRENT_SOURCE_DIR}/runtime/*.cpp")

set(CMAKE_CXX_STANDARD 17)

add_library(olds ${PROJECT_BUILD_FILES})

add_executable(rerun_last_runtime src/runtime.cpp)
add_executable(run_runtime src/runtime.cpp)

compile_options(olds)
compile_options(rerun_last_runtime)
compile_options(run_runtime)

target_link_libraries(olds PRIVATE BLT_WITH_GRAPHICS)
target_link_libraries(rerun_last_runtime PRIVATE BLT_WITH_GRAPHICS)
target_link_libraries(run_runtime PRIVATE BLT_WITH_GRAPHICS)

target_compile_definitions(rerun_last_runtime PRIVATE BLT_AUTORELOAD_RUNTIME=true)

foreach(file ${RUNTIME_FILES})
    file(RELATIVE_PATH rp "${CMAKE_CURRENT_SOURCE_DIR}/runtime" ${file})
    string(MAKE_C_IDENTIFIER "${rp}" rp)
    string(REPLACE "_cpp" "" rp ${rp})
    has_main("${file}" FILE_HAS_MAIN)
    if(FILE_HAS_MAIN)
        message(STATUS "${rp} defines main()")
        string(APPEND runtimes "\"${CMAKE_BINARY_DIR}/${rp}\",\n")
        string(APPEND sources "\"${file}\",\n")

        add_executable(${rp} ${file})
        add_dependencies(rerun_last_runtime ${rp})
        add_dependencies(run_runtime ${rp})

        compile_options(${rp})

        target_link_libraries(${rp} PRIVATE BLT_WITH_GRAPHICS olds)
    else()
        message(STATUS "${rp} does not define main()")
    endif()
endforeach ()

string(APPEND runtimes " ")
string(APPEND sources " ")
target_compile_definitions(rerun_last_runtime PRIVATE RUNTIMES=${runtimes} SOURCES=${sources})
target_compile_definitions(run_runtime PRIVATE RUNTIMES=${runtimes} SOURCES=${sources})

if (${BUILD_BLT_LEARN_EXAMPLES})

endif()

if (BUILD_BLT_LEARN_TESTS)

endif()
