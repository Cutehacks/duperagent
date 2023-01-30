project(${CMAKE_PROJECT_NAME}_Tests)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(QML_IMPORT_PATH ${QML_IMPORT_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_BINARY_DIR}/ ${CMAKE_BINARY_DIR}/imports CACHE STRING "" FORCE)

message(STATUS "QML_IMPORT_PATH: ${QML_IMPORT_PATH}")

find_package(
  QT NAMES Qt6 Qt5
  COMPONENTS QuickTest
  REQUIRED)

find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS QuickTest
  REQUIRED)

enable_testing()

add_definitions(-DQUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(TEST_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_duperagent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_duperagent.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_imageutils.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_promise.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/data.qrc)

add_executable(${PROJECT_NAME} ${TEST_SRC_FILES})

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::QuickTest
                      DuperAgent)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
