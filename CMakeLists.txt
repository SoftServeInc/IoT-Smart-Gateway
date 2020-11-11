cmake_minimum_required(VERSION 3.1)
project( IoT-Smart-Gateway C CXX)
set( CMAKE_CXX_STANDARD 14 )

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()

set( Boost_USE_STATIC_LIBS OFF)
set( Boost_USE_MULTITHREADED ON)
set( Boost_USE_STATIC_RUNTIME OFF)
set( THREADS_PREFER_PTHREAD_FLAG ON )

find_package( libcoap REQUIRED )
find_package( Threads REQUIRED )
find_package( Boost REQUIRED COMPONENTS
        filesystem
        container
        thread
        regex
        )

include_directories( ${Boost_INCLUDE_DIR} )
link_directories( ${Boost_LIBRARY_DIR} )

add_definitions( -DBOOST_ALL_NO_LIB )
add_definitions( -DBOOST_ALL_DYN_LINK )
add_subdirectory( "src/http" "http" )
include_directories( "src" )

add_executable( ${PROJECT_NAME}
        src/source.cpp
        src/coap-server.cpp
        src/coap-client.cpp
        src/http-server.cpp
        src/coap-server.hpp
        src/http-server.hpp
        src/clib/global_timer.hpp
        src/clib/simple_signal.hpp
        src/clib/session_manager.hpp
        src/clib/observation_module.hpp
        )
target_link_libraries( ${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES} OpenSSL::Crypto OpenSSL::SSL ${Boost_LIBRARIES} Threads::Threads libcoap::coap-2 )
target_compile_features(${PROJECT_NAME} PRIVATE
        cxx_lambdas
        cxx_inline_namespaces
        cxx_noexcept
        cxx_nullptr
        cxx_range_for
        cxx_return_type_deduction
        )
