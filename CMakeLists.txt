CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(libtq CXX)
INCLUDE_DIRECTORIES(include src)

ADD_DEFINITIONS(-g -Wall -Werror)

ADD_LIBRARY(tq
  STATIC
  src/task_queue.cc
  src/task.cc)

ADD_SUBDIRECTORY(tests/tq-tester)
