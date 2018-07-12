cmake_minimum_required(VERSION 3.8.2)

project(oemros)

option(BUILD_DOC "Build documentation" ON)

# C++17 std::shared_mutex
# C++17 [[ maybe_unused ]]
# C++14 std::shared_timed_mutex
set(CMAKE_CXX_STANDARD 17)
add_definitions(-Werror -Wall -Wextra)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_definitions(-Wunused -Wsuggest-override)
endif()

find_library(HAMLIB_LIBRARY NAMES hamlib)
find_library(UV_LIBRARY NAMES uv)

find_package(Threads)
find_package(Doxygen)

if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(docs 
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )

else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

find_package(FLEX)
flex_target(scanner turboini.flexpp ${CMAKE_CURRENT_BINARY_DIR}/turboini.cpp)

add_executable(oemros
    ${FLEX_scanner_OUTPUTS}
    logging.cpp
    main.cpp
    module.cpp
    module.test.cpp
    radio.cpp
    runloop.cpp
    system.cpp
    thread.cpp)

target_link_libraries(oemros ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(oemros ${HAMLIB_LIBRARY})
target_link_libraries(oemros ${UV_LIBRARY})

set_source_files_properties(${FLEX_scanner_OUTPUTS} PROPERTIES COMPILE_FLAGS "-Wno-sign-compare -Wno-deprecated-register")

