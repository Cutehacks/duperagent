cmake_minimum_required(VERSION 3.14)

set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 1)
set(${PROJECT_NAME}_PATCH_VERSION 0)
set(${PROJECT_NAME}_VERSION
    ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION}
)

project(
  DuperAgent
  LANGUAGES CXX
  VERSION ${${PROJECT_NAME}_VERSION})

set(CMAKE_AUTOMOC ON)

# compiler options
include(cmake/StandardProjectSettingsConfig.cmake)

# standard compiler warnings
include(cmake/CompilerOptionsAndWarningsConfig.cmake)
set(project_options_and_warnings ${PROJECT_NAME}_project_options_and_warnings)
add_library(${project_options_and_warnings} INTERFACE)
set_project_options_and_warnings(${project_options_and_warnings})

# enable doxygen
include(cmake/DoxygenConfig.cmake)

# project satinizers
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options_and_warnings)

# static analyzers
include(cmake/CppCheckConfig.cmake)
include(cmake/ClangTidyConfig.cmake)

find_package(
  QT NAMES Qt6 Qt5
  COMPONENTS Core Qml Network
  REQUIRED)

find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS Core Qml Network
  REQUIRED)

# find_package(OpenSSL)

set(SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cookiejar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/duperagent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imageutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/multipartsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/networkactivityindicator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/promise.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/promisemodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/request.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/response.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/serialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl.cpp)

set(INCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cookiejar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/duperagent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imageutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jsvalueiterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/multipartsource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/networkactivityindicator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/promise.h
    ${CMAKE_CURRENT_SOURCE_DIR}/promisemodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qpm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/request.h
    ${CMAKE_CURRENT_SOURCE_DIR}/response.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl.h)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES} ${INCLUDE_FILES})

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

target_link_libraries(${PROJECT_NAME} PRIVATE ${project_options_and_warnings})

target_link_libraries(
  ${PROJECT_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Qml
                         Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX d)

# Install script
option(${PROJECT_NAME}_INSTALL "Enable Installing as Library" OFF)

if(${PROJECT_NAME}_INSTALL)
  # Offer the user the choice of overriding the installation directories
  install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    EXPORT ${PROJECT_NAME}Targets)

  install(FILES ${INCLUDE_FILES} DESTINATION include/${PROJECT_NAME})

  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}Version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

  # installation - build tree specific package config files
  export(EXPORT ${PROJECT_NAME}Targets
         FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
  # installation - relocatable package config files
  configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION cmake)

  set(CONFIG_PACKAGE_LOCATION ${CMAKE_INSTALL_LIBDIR}/cmake/QSyncable)

  install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CONFIG_PACKAGE_LOCATION})

  install(FILES ${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
                ${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}Version.cmake
          DESTINATION ${CONFIG_PACKAGE_LOCATION})
endif()

option(ENABLE_TESTING "Enable Test Builds" ON)

if(ENABLE_TESTING)
  message(STATUS "Building Tests")
  include(CTest)
  add_subdirectory(tests)
endif()
