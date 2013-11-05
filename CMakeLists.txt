cmake_minimum_required(VERSION 2.8)

set(TARGET mattext)
project(${TARGET})

find_package(Curses)

set(CMAKE_CXX_FLAGS 
  " -Werror" 
  " -Wall"
  " -Wuninitialized"
  " -Winit-self"
  " -Wtype-limits"
  " -Wclobbered"
  " -Wempty-body"
)

#transform list to string
string(REPLACE ";" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

set(${TARGET}_SRCS 
  mattext.cpp
)

add_executable(${TARGET} ${${TARGET}_SRCS})

target_link_libraries(${TARGET}
  cursesw
)

install(TARGETS ${TARGET} DESTINATION bin)
