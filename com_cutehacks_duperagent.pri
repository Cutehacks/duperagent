
HEADERS += $$PWD/duperagent.h \
    $$PWD/qpm.h \
    $$PWD/request.h \
    $$PWD/response.h \
    $$PWD/serialization.h \
    $$PWD/jsvalueiterator.h \
    $$PWD/config.h \
    $$PWD/cookiejar.h \
    $$PWD/promise.h \
    $$PWD/promisemodule.h

SOURCES += $$PWD/duperagent.cpp \
    $$PWD/request.cpp \
    $$PWD/response.cpp \
    $$PWD/serialization.cpp \
    $$PWD/config.cpp \
    $$PWD/cookiejar.cpp \
    $$PWD/promise.cpp \
    $$PWD/promisemodule.cpp

contains(QT_CONFIG, ssl) | contains(QT_CONFIG, openssl) | contains(QT_CONFIG, openssl-linked) {
    HEADERS += $$PWD/ssl.h
    SOURCES += $$PWD/ssl.cpp
}
