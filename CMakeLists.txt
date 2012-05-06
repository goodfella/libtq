CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(libtq CXX)
INCLUDE_DIRECTORIES(include src)

ADD_DEFINITIONS(-g -Wall -Werror)

ADD_LIBRARY(tq
  STATIC
  src/task_queue.cc
  src/task_desc.cc)

ADD_EXECUTABLE(tq-tester tests/tq_tester.cc)

TARGET_LINK_LIBRARIES(tq)
TARGET_LINK_LIBRARIES(tq-tester tq pthread)
