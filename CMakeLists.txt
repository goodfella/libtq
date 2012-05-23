CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(libtq CXX)
INCLUDE_DIRECTORIES(include src)

ADD_DEFINITIONS(-g -Wall -Werror)

ADD_LIBRARY(tq
  STATIC
  src/task_queue.cc
  src/itask_queue.cc
  src/task_cleanup.cc
  src/task_handle.cc
  src/task_allocator.cc
  src/task.cc
  src/mutex_lock.cc
  src/rwlock_rdlock.cc
  src/rwlock_wrlock.cc)

ADD_SUBDIRECTORY(tests)
