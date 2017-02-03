#-------------------------------------------------
#
# Project created by QtCreator 2016-11-06T11:35:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Spud_Model_Converter
TEMPLATE = app


SOURCES += main.cpp\
        MMainWindow.cpp \
    MOBJFile.cpp

HEADERS  += MMainWindow.h \
    MOBJFile.h

INCLUDEPATH += glm

FORMS    += MMainWindow.ui
