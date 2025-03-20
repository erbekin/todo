cmake_minimum_required(VERSION 3.10.0)
project(todo VERSION 0.1.0 LANGUAGES C)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG)
endif()

add_executable(todo main.c todo.c)

find_package(OpenSSL REQUIRED)
target_link_libraries(todo PRIVATE OpenSSL::SSL OpenSSL::Crypto)