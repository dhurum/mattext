cmake_minimum_required(VERSION 2.8)

set(TARGET mattext)
project(${TARGET})

find_package(Curses)

if(CURSES_NCURSES_LIBRARY MATCHES NOTFOUND)
  message(FATAL_ERROR "Ncurses library not found!")
endif()


include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(--std=c++1y HAS_CXX14)
if(NOT HAS_CXX14)
  message(FATAL_ERROR "Compiler does not support c++14!")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  #add homebrew paths and argp library for OSX
  if(NOT HOMEBREW_PATH)
    set(HOMEBREW_PATH /usr/local)
  endif()

  include_directories(${HOMEBREW_PATH}/include)
  link_directories(${HOMEBREW_PATH}/lib)

  set(TARGET_LIBS argp)
else()
  #in Linux use widechar ncurses library
  string(REPLACE "s." "sw." CURSES_NCURSES_LIBRARY "${CURSES_NCURSES_LIBRARY}")
endif()

#check if we need to include some dir to get curses with wchar_t support
include(CheckCXXSymbolExists)
set(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE_EXTENDED)
set(CMAKE_REQUIRED_INCLUDES ${CURSES_INCLUDE_PATH})
set(CMAKE_REQUIRED_LIBRARIES ${CURSES_NCURSES_LIBRARY})
CHECK_CXX_SYMBOL_EXISTS(setcchar "${CURSES_INCLUDE_PATH}/curses.h"
  CURSES_HAS_WCHAR)

if(NOT CURSES_HAS_WCHAR)
  file(GLOB CURSES_SUBDIRS "${CURSES_INCLUDE_PATH}/*curses*")
  foreach(CURSES_SUBDIR ${CURSES_SUBDIRS})
    if(IS_DIRECTORY ${CURSES_SUBDIR})
      unset(CURSES_HAS_WCHAR CACHE)
      CHECK_CXX_SYMBOL_EXISTS(setcchar "${CURSES_SUBDIR}/curses.h"
        CURSES_HAS_WCHAR)
      if(${CURSES_HAS_WCHAR})
        include_directories(BEFORE ${CURSES_SUBDIR})
        break()
      endif()
    endif()
  endforeach()
endif()

if(NOT CURSES_HAS_WCHAR)
  message(FATAL_ERROR "Ncurses does not support wide characters!")
endif()

find_library(LIBEV_LIBRARY NAMES ev)
if(NOT LIBEV_LIBRARY)
  message(FATAL_ERROR "Libev library not found!")
endif()

set(TARGET_LIBS
  ${TARGET_LIBS} 
  ${CURSES_NCURSES_LIBRARY}
  ${LIBEV_LIBRARY}
)
include_directories(${CURSES_INCLUDE_PATH})

set(CMAKE_CXX_FLAGS
  ${CMAKE_CXX_FLAGS}
  -Werror
  -Wall
  -Wuninitialized
  -Winit-self
  -Wtype-limits
  -Wempty-body
  --std=c++1y
)
#add additional warning option for gcc
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wclobbered)
endif()

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -O2)
endif()

set (CMAKE_CXX_FLAGS_DEBUG -g3)

#transform list to string
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE ";" " " CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set(${TARGET}_SRCS
  src/config.cpp
  src/file_io.cpp
  src/file_stream.cpp
  src/file_reader.cpp
  src/file_forward_reader.cpp
  src/file_backward_reader.cpp
  src/terminal.cpp
  src/animation.cpp
  src/animation_matrix.cpp
  src/animation_reverse_matrix.cpp
  src/manager_interactive.cpp
  src/manager_plain.cpp
  src/mattext.cpp
)

add_executable(${TARGET} ${${TARGET}_SRCS})

target_link_libraries(${TARGET}
  ${TARGET_LIBS}
)

#compress manpage
set(MANPAGE_GZ ${CMAKE_BINARY_DIR}/mattext.1.gz)
set(MANPAGE_SRC ${CMAKE_SOURCE_DIR}/mattext.1)
add_custom_command(OUTPUT ${MANPAGE_GZ}
  COMMAND gzip -c ${MANPAGE_SRC} > ${MANPAGE_GZ}
  DEPENDS ${MANPAGE_SRC})

add_custom_target(MANPAGE ALL DEPENDS ${MANPAGE_GZ})

install(TARGETS ${TARGET} DESTINATION bin)
install(FILES ${MANPAGE_GZ} DESTINATION share/man/man1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
