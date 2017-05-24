cmake_minimum_required(VERSION 2.6)
project(ecrire)
set(VMAJ 0)
set(VMIN 2)
set(VMIC 6)
set(PACKAGE_VERSION ${VMAJ}.${VMIN}.${VMIC})

set(PACKAGE ${CMAKE_PROJECT_NAME})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include_directories ("${PROJECT_BINARY_DIR}")

# FIXME: add version checking!
find_package(Eo REQUIRED)
find_package(Eina REQUIRED)
find_package(Eet REQUIRED)
find_package(Evas REQUIRED)
find_package(Ecore REQUIRED)
find_package(Ecore COMPONENTS INPUT)
find_package(Edje REQUIRED)
find_package(Efreet REQUIRED)
find_package(Efreet COMPONENTS mime)
find_package(Elementary REQUIRED)

IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    find_package(intl REQUIRED)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

# likely put this into an FindCompilerAttribute.cmake:
INCLUDE(CheckCSourceCompiles)
SET(HAVE___ATTRIBUTE__)
CHECK_C_SOURCE_COMPILES(
   "void  foo (int bar __attribute__((unused)) ) { }
   static void baz (void) __attribute__((unused));
   static void baz (void) { }
   int main(){}
   " HAVE___ATTRIBUTE__
)

ADD_DEFINITIONS(-DPACKAGE_BIN_DIR="${CMAKE_INSTALL_PREFIX}/bin"
   -DPACKAGE_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}"
   -DPACKAGE_LIB_DIR="${CMAKE_INSTALL_PREFIX}/lib")

set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale")

ADD_DEFINITIONS(
   -DLOCALE_DIR="${LOCALEDIR}"
)

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DEFL_BETA_API_SUPPORT)

find_package(Gettext)
if (GETTEXT_FOUND)
   set(GettextTranslate_ALL true)
   set(GettextTranslate_GMO_BINARY true)
   include(GettextTranslate)
   set(ENABLE_NLS 1)
endif (GETTEXT_FOUND)

configure_file (
   "${PROJECT_SOURCE_DIR}/cmakeconfig.h.in"
   "${PROJECT_BINARY_DIR}/config.h"
)

add_subdirectory(src/bin)
add_subdirectory(data)
add_subdirectory(po)

# Package Sources
set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
set(CPACK_SOURCE_IGNORE_FILES
"${PROJECT_SOURCE_DIR}/.git/"
"${PROJECT_SOURCE_DIR}/.gitignore"
"${PROJECT_SOURCE_DIR}/CMakeCache.txt"
"${PROJECT_SOURCE_DIR}/CMakeFiles"
"${PROJECT_SOURCE_DIR}/_CPack_Packages/"
"${PROJECT_SOURCE_DIR}/CPackConfig.cmake"
"${PROJECT_SOURCE_DIR}/CPackSourceConfig.cmake"
"${PROJECT_SOURCE_DIR}/Makefile"
"${PROJECT_SOURCE_DIR}/cmake_install.cmake"
"${PROJECT_SOURCE_DIR}/nbproject/"
"${PROJECT_SOURCE_DIR}/dist/"
"${PROJECT_SOURCE_DIR}/docs/"
"${PROJECT_SOURCE_DIR}/install_manifest.txt"
"${PROJECT_SOURCE_DIR}/src/bin/CMakeFiles"
"${PROJECT_SOURCE_DIR}/src/bin/Makefile"
"${PROJECT_SOURCE_DIR}/src/bin/cmake_install.cmake"

)

# Make dist
set(CPACK_PACKAGE_NAME "ecrire")
set(CPACK_PACKAGE_CONTACT "William L. Thomson Jr.")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "William L. Thomson Jr.")
set(CPACK_PACKAGE_VERSION_MAJOR ${VMAJ})
set(CPACK_PACKAGE_VERSION_MINOR ${VMIN})
set(CPACK_PACKAGE_VERSION_PATCH ${VMIC})
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_OUTPUT_FILE_PREFIX dist)
set(CPACK_PACKAGE_FILE_NAME
   "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source package )
include(CPack)

include(MakeDistcheck)
add_distcheck()
