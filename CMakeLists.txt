cmake_minimum_required(VERSION 3.8.2)

project(oemros)

option(BUILD_DOC "Build documentation" ON)

# C++17 std::shared_mutex
# C++17 [[ maybe_unused ]]
# C++14 std::shared_timed_mutex
set(CMAKE_CXX_STANDARD 17)

# g++ (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609
# does not understand
# -Wtautological-compare -Wmisleading-indentation

add_definitions(-Wfatal-errors -Werror -Wall -Wextra)
add_definitions(-Wuninitialized -Winit-self)
add_definitions(-Wfloat-equal -Wpointer-arith -Wcast-qual -Wcast-align)
add_definitions(-Wundef -Wshadow)
# security related warnings - maybe overkill?
add_definitions(-Wformat-nonliteral -Wformat-security)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_definitions(-Wunused -Wsuggest-override)
endif()

find_library(HAMLIB_LIBRARY NAMES hamlib)
find_library(UV_LIBRARY NAMES uv)

find_package(Threads)
find_package(Doxygen)

if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/Doxyfile.in)
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

add_executable(
    oemros
    
    src/logjam.cxx
    src/system.cxx
    src/system.unix.cxx
    src/thread.cxx
    src/logging.cxx
    src/runloop.cxx
    src/hamlib.cxx
    src/radio.cxx
    src/main.cxx
)

target_link_libraries(oemros ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(oemros boost_system)
target_link_libraries(oemros boost_thread)
target_link_libraries(oemros hamlib)
