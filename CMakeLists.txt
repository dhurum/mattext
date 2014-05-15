cmake_minimum_required(VERSION 2.8)

set(TARGET mattext)
project(${TARGET})

find_package(Curses)

if(CURSES_NCURSES_LIBRARY MATCHES NOTFOUND)
  message(FATAL_ERROR "Ncurses library not found!")
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


set(TARGET_LIBS ${TARGET_LIBS} ${CURSES_NCURSES_LIBRARY})
include_directories(${CURSES_INCLUDE_PATH})

set(CMAKE_CXX_FLAGS
  -Werror
  -Wall
  -Wuninitialized
  -Winit-self
  -Wtype-limits
  -Wempty-body
  -O2
)
#add additional warning option for gcc
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wclobbered)
endif()
#transform list to string
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(${TARGET}_SRCS
  src/cmdline.cpp
  src/input.cpp
  src/screen.cpp
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

#check if we need to include some dir to get curses with wchar_t support
include(CheckCXXSymbolExists)
set(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE_EXTENDED)
set(CMAKE_REQUIRED_INCLUDES ${CURSES_INCLUDE_PATH})
set(CMAKE_REQUIRED_LIBRARIES ${TARGET_LIBS})
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
