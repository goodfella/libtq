CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(tq-tester CXX)
ADD_DEFINITIONS(-g -Wall -Werror)

ADD_EXECUTABLE(tq-tester
  tq_tester.cc
  task_manager.cc
  task_thread_manager.cc
  task.cc
  queue_thread_manager.cc
  bool_flag.cc)

TARGET_LINK_LIBRARIES(tq-tester tq pthread)
