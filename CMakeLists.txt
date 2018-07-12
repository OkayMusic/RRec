# minimum version for cmake
cmake_minimum_required(VERSION 2.8)

# project's name
project(MolecularDynamics)

# define some constant we'll use as compiler flags
SET(COMPILER_FLAGS "-march=native -Ofast")
add_definitions(${COMPILER_FLAGS})

# define location for produced binaries
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# NOTE: running cmake generates a makefile with explicitly named .x files, so
# every time a new .c, .cpp or .h file is added cmake should be run in order to
# generate a makefile which includes the new file.
file(GLOB SRC_FILES
  "src/*.h"
  "src/*.cpp"
  "src/*.c"
)
find_package( OpenCV REQUIRED )

set(CMAKE_CXX_STANDARD 14)


add_executable(a.out ${SRC_FILES})

target_link_libraries(a.out m boost_thread boost_system ${OpenCV_LIBS})