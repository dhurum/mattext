cmake_minimum_required(VERSION 2.8)

set(TARGET mattext)
project(${TARGET})

find_package(Curses)

set(${TARGET}_LIBS ncursesw)

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
  ${${TARGET}_LIBS}
)

#compress manpage
set(MANPAGE_GZ ${CMAKE_BINARY_DIR}/mattext.1.gz)
add_custom_command(OUTPUT ${MANPAGE_GZ} 
  COMMAND gzip -c ${CMAKE_SOURCE_DIR}/mattext.1 > ${MANPAGE_GZ})

add_custom_target(MANPAGE ALL DEPENDS ${MANPAGE_GZ})

install(TARGETS ${TARGET} DESTINATION bin)
install(FILES ${MANPAGE_GZ} DESTINATION share/man/man1)

#check if we need to include some dir to get curses with wchar_t support
include(CheckCXXSymbolExists)
set(CMAKE_REQUIRED_LIBRARIES ${${TARGET}_LIBS})
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
        include_directories(${CURSES_SUBDIR})
        break()
      endif()
    endif()
  endforeach()
endif()

if(NOT CURSES_HAS_WCHAR)
  message(FATAL_ERROR "Ncurses does not support wide characters!")
endif()
