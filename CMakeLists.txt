cmake_minimum_required(VERSION 3.2)

project(configtree-parser C CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(Eigen3)

add_definitions(-DHAVE_EIGEN=${EIGEN3_FOUND})

function(add_eigen3_flags)
  if(EIGEN3_FOUND)
    cmake_parse_arguments(ADD_EIGEN "SOURCE_ONLY;OBJECT" "" "" ${ARGN})
    if(ADD_EIGEN_SOURCE_ONLY)
      set(_prefix SOURCE)
    else()
      set(_prefix TARGET)
    endif()

    include_directories(${EIGEN3_INCLUDE_DIR})
    set_property(${_prefix} ${ADD_EIGEN_UNPARSED_ARGUMENTS}
      APPEND PROPERTY
      COMPILE_DEFINITIONS "ENABLE_EIGEN=1")
  endif(EIGEN3_FOUND)
endfunction(add_eigen3_flags)

add_compile_options(-Wall -O2)

enable_testing()

add_executable(configtreetest configtreetest.cc)
add_eigen3_flags(configtreetest)
add_test(configtreetest configtreetest)
