#
# Select QT version preferred
#
set(QT5_VERSION_MATCH 5.15)
set(QT6_VERSION_MATCH 6)

if (QT_VERSION_PREFERRED AND QT_VERSION_PREFERRED EQUAL 5)
  message(STATUS "Try loading Qt5 (explicitly preferred)...")
  set(QT5_TRIED 1)
  set(QT_DEFAULT_MAJOR_VERSION 5)
  find_package(Qt5 ${QT5_VERSION_MATCH} COMPONENTS Core QUIET)
  if(Qt5_FOUND)
    message(STATUS "Choosing Qt5, since explicitly preferred")
    set(QT_FOUND 1)
    set(QT_VERSION_MAJOR 5)
    set(QT_VERSION ${QT5_VERSION_MATCH})
  else ()
    message(STATUS "Qt5 NOT found")
  endif()
endif ()

if (QT_VERSION_PREFERRED AND QT_VERSION_PREFERRED EQUAL 6)
  message(STATUS "Try loading preferred Qt6 (explicitly preferred)...")
  set(QT6_TRIED 1)
  set(QT_DEFAULT_MAJOR_VERSION 6)
  find_package(Qt6 ${QT6_VERSION_MATCH} COMPONENTS Core QUIET)
  if(Qt6_FOUND)
    message(STATUS "Choosing Qt6, since explicitly preferred")
    set(QT_FOUND 1)
    set(QT_VERSION_MAJOR 6)
    set(QT_VERSION ${QT6_VERSION_MATCH})
  else ()
    message(STATUS "Qt6 NOT found")
  endif()
endif()

if (NOT QT_FOUND AND NOT QT5_TRIED)
  message(STATUS "Try loading Qt5 (implicitly preferred version)...")
  set(QT_DEFAULT_MAJOR_VERSION 5)
  find_package(Qt5 ${QT5_VERSION_MATCH} COMPONENTS Core QUIET)
endif ()

if(NOT QT_FOUND)
  if (Qt5_FOUND)
    message(STATUS "Choosing Qt5, since implicitly preferred")
    set(QT_FOUND 1)
    set(QT_VERSION_MAJOR 5)
    set(QT_VERSION ${QT5_VERSION_MATCH})
  else ()
    message(STATUS "Qt5 NOT found")
  endif ()
endif ()

if (NOT QT_FOUND AND NOT QT6_TRIED)
  message(STATUS "Try loading Qt6 (implicitly preferred version)...")
  set(QT_DEFAULT_MAJOR_VERSION 6)
  find_package(Qt6 ${QT6_VERSION_MATCH} COMPONENTS Core QUIET)
endif()

if (NOT QT_FOUND)
  if (Qt6_FOUND)
    message(STATUS "Choosing Qt6, since implicitly preferred")
    set(QT_FOUND 1)
    set(QT_VERSION_MAJOR 6)
    set(QT_VERSION ${QT6_VERSION_MATCH})
  else ()
    message(STATUS "Qt6 NOT found")
  endif()
endif()

if(QT_FOUND)
  message(STATUS "Qt version used: ${QT_VERSION}")
  option(QT_QML_DEBUG "Build with QML debugger support" OFF)
  mark_as_advanced(QT_QML_DEBUG)
endif ()

