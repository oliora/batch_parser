# Created by Andrey Upadyshev aka Oliora.
# This file was released to Public domain.

cmake_minimum_required(VERSION 2.8)

project(batch_parser)

if(MSVC11)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=0)
    add_definitions(-D_VARIADIC_MAX=10)
endif()

if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

# enable unit testing
enable_testing()

# Find Boost
add_definitions(-DBOOST_ALL_NO_LIB) # Turn off Boost's auto-linking
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.41 REQUIRED COMPONENTS)

# Find GTest + GMock
#find_package(GMock 1.6)
#if(NOT GMOCK_FOUND)
#    message(WARNING "Can't find GMock 1.6. Unit tests project will NOT be generated")
#endif()

add_subdirectory(src)

#if(GMOCK_FOUND)
#    add_subdirectory(tests)
#endif()
