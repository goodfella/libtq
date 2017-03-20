CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(libtq CXX)
INCLUDE_DIRECTORIES(include src)

ADD_DEFINITIONS(-g -Wall -Werror -std=c++14)

ADD_LIBRARY(tq
  STATIC
  src/task_queue.cc
  src/itask_queue.cc
  src/task_runner.cc
  src/shutdown_task.cc
  src/runner_canceled.cc
  src/wait_task.cc
  src/itask.cc)

ADD_SUBDIRECTORY(tests)
