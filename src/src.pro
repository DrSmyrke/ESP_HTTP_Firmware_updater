QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = esp_http_updater
CONFIG += c++11

CONFIG(debug, debug|release):CONFIGURATION=debug
win32|win64{
    CONFIG(release, debug|release){
        CONFIGURATION=release
        #from static build
        QMAKE_LFLAGS_RELEASE += -static -static-libgcc
    }
}

build_pass:CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
}


DEFINES += QT_DEPRECATED_WARNINGS

OBJECTS_DIR         = ../build/obj/$${CONFIGURATION}
MOC_DIR             = ../build/$${CONFIGURATION}
RCC_DIR             = ../build/rcc
UI_DIR              = ../build/ui
DESTDIR             = ../bin

SOURCES += \
    aboutdialog.cpp \
    global.cpp \
    main.cpp \
    mainwindow.cpp
HEADERS += \
    aboutdialog.h \
    global.h \
    mainwindow.h
FORMS += mainwindow.ui \
    aboutdialog.ui \
    authenticationdialog.ui

RESOURCES += resources.qrc
TRANSLATIONS = lang/ru_RU.ts

exists(./gitversion.pri):include(./gitversion.pri)
exists(./myLibs.pri):include(./myLibs.pri)
exists(index.rc){
    win32|win64{
        RC_FILE=  index.rc
        OTHER_FILES+= index.rc
        DISTFILES += index.rc
    }
}
