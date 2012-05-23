CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(tq-tester CXX)
ADD_DEFINITIONS(-g -Wall -Werror)

ADD_EXECUTABLE(tq-tester
  tq_tester.cc
  task_manager.cc
  task_thread_manager.cc
  test_task.cc
  queue_thread_manager.cc)


TARGET_LINK_LIBRARIES(tq-tester tq-test tq pthread)
