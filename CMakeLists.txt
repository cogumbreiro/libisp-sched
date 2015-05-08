cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(libisp-sched)


###
# CPM dependencies
###
set(CPM_NO_UPDATE TRUE)
include(cpm/CPM.cmake)
CPM_AddModule("catch" GIT_REPOSITORY "https://github.com/bcachet/cpm-catch")
# find_package( Boost REQUIRED COMPONENTS iterator )
CPM_AddModule("boost"
    GIT_REPOSITORY "https://github.com/cogumbreiro/cpm-boost"
    GIT_TAG "1.57.0")
CPM_Finish()

##
# Project details:
##

add_definitions (
    -Wall -pedantic -g
    -std=c++1y # Or -std=c++0x
)

enable_testing()

SET(SRC
    #src/CB.hpp
    src/name2id.cpp #src/name2id.hpp
    src/Envelope.cpp #src/Envelope.hpp
    #src/Transition.hpp
    #src/Trace.cpp src/Trace.hpp
    #src/State.hpp
    #src/EnabledTransitions.cpp src/EnabledTransitions.hpp
    #src/AmpleSet.cpp src/AmpleSet.hpp
    #src/Node.cpp src/Node.hpp
    #src/Call.hpp src/Call.cpp
    #src/InterleavingTree.cpp src/InterleavingTree.hpp
    src/Call.cpp
    src/Checker.cpp
    src/MatchSet.cpp
    src/Generator.cpp
    src/TestCall.cpp
)

add_library(isp-sched STATIC ${SRC})

add_executable(test-call src/TestCall.cpp)
target_link_libraries(test-call isp-sched)
target_link_libraries(test-call ${CPM_LIBRARIES})

add_executable(test-ex src/TestExamples.cpp)
target_link_libraries(test-ex isp-sched)
target_link_libraries(test-ex ${CPM_LIBRARIES})

add_test(call test-call)
add_test(examples test-ex)
