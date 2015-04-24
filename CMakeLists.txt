cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(libisp-sched)


###
# CPM dependencies
###
set(CPM_NO_UPDATE TRUE)
include(cpm/CPM.cmake)
CPM_AddModule("catch" GIT_REPOSITORY "https://github.com/bcachet/cpm-catch")    
CPM_Finish()

##
# Project details:
##

ADD_DEFINITIONS(
    -std=c++11 # Or -std=c++0x
)

enable_testing()

add_executable(test-cb src/CB.hpp src/TestCB.cpp)
add_test(test-cb test-cb)
target_link_libraries(test-cb ${CPM_LIBRARIES})


add_executable(test-tr src/CB.hpp src/Transition.cpp src/Transition.hpp src/TestTransition.cpp)
add_test(test-tr test-tr)
target_link_libraries(test-tr ${CPM_LIBRARIES})

add_executable(test-tl src/CB.hpp src/Transition.cpp src/Transition.hpp src/TestTransition.cpp src/TransitionList.cpp src/TransitionList.hpp)
add_test(test-tl test-tl)
target_link_libraries(test-tl ${CPM_LIBRARIES})

#set(Sources
#    src/TestTransition.cpp
#    src/name2id.cpp src/name2id.hpp
#    src/Envelope.cpp src/Envelope.hpp
#    src/Transition.hpp src/Transition.cpp
#    src/TransitionList.hpp src/TransitionList.cpp)

#-----------------------------------------------------------------------
# Setup executable
#-----------------------------------------------------------------------
#set(EXE_NAME cpm-test)
#add_executable(${EXE_NAME} ${Sources})
#target_link_libraries(${EXE_NAME} ${CPM_LIBRARIES})
