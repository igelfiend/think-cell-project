cmake_minimum_required(VERSION 3.5)

project(ThinkCell-project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(ThinkCell-project main.cpp)

target_link_libraries(
  ThinkCell-project
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ThinkCell-project)

install(
    TARGETS ThinkCell-project
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
