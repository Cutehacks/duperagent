TEMPLATE = app
TARGET = tst_duperagent
CONFIG += warn_on qmltestcase
SOURCES += tst_duperagent.cpp
OTHER_FILES += *.qml

include($$PWD/../com_cutehacks_duperagent.pri)
