cmake_minimum_required(VERSION 3.0)
project(EffectiveModernCpp)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

### Catch Framework settings ###
set(YOUR_CATCH_ROOT_DIR "/Users/yoonsung-mac/OpenSources/Catch")

set(CATCH "${YOUR_CATCH_ROOT_DIR}/include")
include_directories(${CATCH})

#add_library(Catch INTERFACE)
#target_link_libraries(Catch INTERFACE ${CATCH}) #library 뽑는게 아니라 executable 뽑을 때는 target_link_libraries 가 필요 없나?

### GoogleTest Framework settings ###
set(GTEST_DIR "/Users/yoonsung-mac/OpenSources/googletest/googletest")
include_directories(${GTEST_DIR}/include)
include_directories(${GTEST_DIR}/mybuild)

option(BUILD_TEST "Build the unit tests" ON)

#add_test(NAME ADDTest_BrainShockSTL COMMAND Test_BrainShockSTL arg1="-r junit")
#add_test(ADDTest_BrainShockSTL Test_BrainShockSTL -r junit)
add_subdirectory(src)
add_subdirectory(test)

enable_testing()
