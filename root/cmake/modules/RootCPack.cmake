#---------------------------------------------------------------------------------------------------
#  RootCPack.cmake
#   - basic setup for packaging ROOT using CTest
#---------------------------------------------------------------------------------------------------

#---------------------------------------------------------------------------------------------------
# Package up needed system libraries - only for WIN32?
#
include(InstallRequiredSystemLibraries)

#----------------------------------------------------------------------------------------------------
# General packaging setup - variable relavant to all package formats
#
set(CPACK_PACKAGE_DESCRIPTION "ROOT project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ROOT project")
set(CPACK_PACKAGE_VENDOR "HEPSoft")
set(CPACK_PACKAGE_VERSION ${ROOT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${ROOT_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${ROOT_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${ROOT_PATCH_VERSION})

string(REGEX REPLACE "^([0-9]+).*$" "\\1" CXX_MAJOR ${CMAKE_CXX_COMPILER_VERSION})
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\2" CXX_MINOR ${CMAKE_CXX_COMPILER_VERSION})

#---Resource Files-----------------------------------------------------------------------------------
configure_file(README/README README.txt COPYONLY)
configure_file(LICENSE LICENSE.txt COPYONLY)
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_BINARY_DIR}/README.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_BINARY_DIR}/README.txt")

#---Source package settings--------------------------------------------------------------------------
set(CPACK_SOURCE_IGNORE_FILES 
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/tests
    "~$"
    "/CVS/"
    "/.svn/"
    "/\\\\\\\\.svn/"
    "/.git/"
    "/\\\\\\\\.git/"
    "\\\\\\\\.swp$"
    "\\\\\\\\.swp$"
    "\\\\.swp"
    "\\\\\\\\.#"
    "/#"
)
set(CPACK_SOURCE_STRIP_FILES "")

#---Binary package setup-----------------------------------------------------------------------------
if(MSVC)
  math(EXPR VS_VERSION "${VC_MAJOR} - 6")
  set(COMPILER_NAME_VERSION ".vc${VS_VERSION}")
else()
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(COMPILER_NAME_VERSION "-gcc${CXX_MAJOR}.${CXX_MINOR}")
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(COMPILER_NAME_VERSION "-clang${CXX_MAJOR}${CXX_MINOR}")
  endif()
endif()

set(CPACK_PACKAGE_RELOCATABLE True)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "root_v${ROOT_VERSION}")
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set(CPACK_PACKAGE_FILE_NAME "root_v${ROOT_VERSION}.${ROOT_ARCHITECTURE}${COMPILER_NAME_VERSION}")
else()
  set(CPACK_PACKAGE_FILE_NAME "root_v${ROOT_VERSION}.${ROOT_ARCHITECTURE}${COMPILER_NAME_VERSION}.${CMAKE_BUILD_TYPE}")
endif()
set(CPACK_PACKAGE_EXECUTABLES "root" "ROOT")

if(WIN32)
  set(CPACK_GENERATOR "ZIP;NSIS;TGZ")
  set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
elseif(APPLE)
  set(CPACK_GENERATOR "TGZ;PackageMaker")
  set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
else()
  set(CPACK_GENERATOR "STGZ;TGZ")
  set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2;ZIP")
endif()

#----------------------------------------------------------------------------------------------------
# Finally, generate the CPack per-generator options file and include the
# base CPack configuration.
#
configure_file(cmake/modules/CMakeCPackOptions.cmake.in CMakeCPackOptions.cmake @ONLY)
set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_BINARY_DIR}/CMakeCPackOptions.cmake)
include(CPack)

#----------------------------------------------------------------------------------------------------
# Define components and installation types (after CPack included!)
#
cpack_add_install_type(full      DISPLAY_NAME "Full Installation")
cpack_add_install_type(minimal   DISPLAY_NAME "Minimal Installation")
cpack_add_install_type(developer DISPLAY_NAME "Developer Installation")

cpack_add_component(applications 
    DISPLAY_NAME "ROOT Applications" 
    DESCRIPTION "ROOT executables such as root.exe"
     INSTALL_TYPES full minimal developer)

cpack_add_component(libraries 
    DISPLAY_NAME "ROOT Libraries" 
    DESCRIPTION "All ROOT libraries and dictionaries"
     INSTALL_TYPES full minimal developer)

cpack_add_component(headers 
    DISPLAY_NAME "C++ Headers" 
    DESCRIPTION "These are needed to do any development"
     INSTALL_TYPES full developer)
     
cpack_add_component(tests 
    DISPLAY_NAME "ROOT Tests and Tutorials" 
    DESCRIPTION "These are needed to do any test and tutorial"
     INSTALL_TYPES full developer)


