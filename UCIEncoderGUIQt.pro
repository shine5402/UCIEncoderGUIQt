#-------------------------------------------------
#
# Project created by QtCreator 2018-05-06T15:58:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UCIEncoderGUIQt
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS += icon.ico

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    public_variable.cpp \
    ../LeafPublicQtClasses/leaflogger.cpp \
    ../LeafPublicQtClasses/processmanager.cpp \
    qlabel_doubleclick.cpp \
    picturehelper.cpp

HEADERS += \
        mainwindow.h \
    publicdefines.h \
    public_variable.h \
    ../LeafPublicQtClasses/leaflogger.h \
    ../LeafPublicQtClasses/processmanager.h \
    qlabel_doubleclick.h \
    picturehelper.h

FORMS += \
        mainwindow.ui
CONFIG += c++1z

win32-msvc{
QMAKE_CXXFLAGS += /std:c++17
}

RESOURCES += \
    uciencoderguiqt.qrc
