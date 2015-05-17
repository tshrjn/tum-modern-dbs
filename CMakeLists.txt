cmake_minimum_required(VERSION 2.8.4)
project(tum-modern-dbs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_custom_target(slottedTest COMMAND make -C ${tum-modern-dbs_SOURCE_DIR}
                         CLION_EXE_DIR=${PROJECT_BINARY_DIR})