cmake_minimum_required(VERSION 3.2)
project(tcp-relay)

set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

list(APPEND SOURCES
     src/server.cc
     src/parse_args.cc
     src/main.cc
    )

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-missing-braces")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

    find_library(BOOSTSYS NAMES boost_system boost_system-mt)
    find_library(GLOG     glog)
    find_library(GFLAGS   gflags)
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2")

    find_library(BOOSTSYS NAMES boost_system boost_system-vc140-mt boost_system-vc140-mt-gd)

    find_package(glog REQUIRED)
    find_package(gflags REQUIRED)
    set(GLOG glog::glog)
    set(GFLAGS gflags)
endif()

list(APPEND DEPS
     ${BOOSTSYS}
     ${GLOG}
     ${GFLAGS}
    )

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${DEPS})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

