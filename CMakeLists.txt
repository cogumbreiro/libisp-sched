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

function(add_executable_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} isp-sched)
    target_link_libraries(${target} ${CPM_LIBRARIES})
    add_test(${target} ${target})
endfunction()

##
# Project details:
##

add_definitions (
    -Wall -pedantic -g
    -std=c++1y # Or -std=c++0x
)

enable_testing()

set(SRC
    src/name2id.cpp
    src/Call.cpp
    src/Checker.cpp
    src/MatchSet.cpp
    src/CallDB.cpp
    src/Generator.cpp
)

add_library(isp-sched STATIC ${SRC})

add_executable_test(test-call src/TestCall.cpp)
add_executable_test(test-check src/TestChecker.cpp)
add_executable_test(test-call-db src/TestCallDB.cpp)
add_executable_test(test-gen src/TestGenerator.cpp)
add_executable_test(test-ex src/TestExamples.cpp)
