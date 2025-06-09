project(noslink C)
cmake_minimum_required(VERSION 2.6.4)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZeroMQ REQUIRED libzmq)

# Find Simulith headers and library
set(SIMULITH_DIR "../../../../sims/simulith" CACHE PATH "Path to Simulith directory")
if(NOT EXISTS "${SIMULITH_DIR}/include/simulith_uart.h")
    message(FATAL_ERROR "Could not find simulith_uart.h in ${SIMULITH_DIR}/include. Please set SIMULITH_DIR to the correct path.")
endif()

# Build Simulith if not already built
add_subdirectory(${SIMULITH_DIR} ${CMAKE_BINARY_DIR}/simulith)

include_directories(inc
                    ${NOSENGINE_INCLUDE_DIRS}
                    ${MISSION_SOURCE_DIR}/components/hwlib/fsw/public_inc
                    ${MISSION_SOURCE_DIR}/osal/src/os/inc
                    ${MISSION_SOURCE_DIR}/psp/fsw/inc
                    ${SIMULITH_DIR}/include
                    ${ZeroMQ_INCLUDE_DIRS}
                    )

aux_source_directory(src NOSLINK_SRC)

add_library(noslink STATIC ${NOSLINK_SRC})
target_link_libraries(noslink 
    ${NOSENGINE_LIBRARIES} 
    simulith
    ${ZeroMQ_LIBRARIES}
    gcov
)

