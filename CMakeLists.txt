cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

#------------------------------------------------------------------------------
# Required CPM Setup - no need to modify - See: https://github.com/iauns/cpm
#------------------------------------------------------------------------------
set(CPM_NO_UPDATE TRUE)
include(cpm/CPM.cmake)
CPM_AddModule("cpm-catch" GIT_REPOSITORY "https://github.com/bcachet/cpm-catch")
    
CPM_Finish()


set(Sources
    src/TestTransition.cpp
    src/name2id.cpp src/name2id.hpp
    src/Envelope.cpp src/Envelope.hpp
    src/Transition.hpp src/Transition.cpp
    src/TransitionList.hpp src/TransitionList.cpp)

#-----------------------------------------------------------------------
# Setup executable
#-----------------------------------------------------------------------
set(EXE_NAME cpm-test)
add_executable(${EXE_NAME} ${Sources})
target_link_libraries(${EXE_NAME} ${CPM_LIBRARIES})
