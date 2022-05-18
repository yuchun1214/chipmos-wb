cmake_minimum_required(VERSION 3.8)
project(ChipmosWb LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
include_directories(${CMAKE_SOURCE_DIR})
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(
    main 
    main.cpp)

add_executable(
	indic
	indicator.cpp)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tests)

if(WIN32)
    target_link_libraries(main src pgalib -static-libgcc -static-libstdc++)
    target_link_libraries(indic src)
else()
    target_link_libraries(main src pgalib pthread)
    target_link_libraries(indic src)
endif()

# if(WIN32)
#     target_link_libraries(main src pgalib -static-libstdc++ m)
# else()
#     target_link_libraries(main src pgalib stdc++ m)
# endif()
