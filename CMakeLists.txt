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

ADD_DEFINITIONS(
    -std=c++1y # Or -std=c++0x
)

enable_testing()

add_library(isp-sched
        src/CB.hpp
        src/name2id.cpp src/name2id.hpp
        src/Envelope.cpp src/Envelope.hpp
        src/Transition.hpp
        src/Trace.cpp src/Trace.hpp
        src/Matcher.hpp
        src/State.hpp
        src/EnabledTransitions.cpp src/EnabledTransitions.hpp
        src/AmpleSet.cpp src/AmpleSet.hpp
        src/Node.cpp src/Node.hpp
        src/InterleavingTree.cpp src/InterleavingTree.hpp
        )

target_include_directories(isp-sched PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test-cb src/TestCB.cpp)
add_test(test-cb test-cb)
#target_link_libraries(test-cb ${CPM_LIBRARIES})

add_executable(test-tr src/TestTransition.cpp)
add_test(test-tr test-tr)
