cmake_minimum_required(VERSION 3.10)
project(cs6420_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


include_directories(include)
add_executable(main src/MemoryBounds.cpp ${ALG_CPP})
target_link_libraries(main PRIVATE Threads::Threads)