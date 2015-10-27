#-------------------------------------------------
#
# Project created by QtCreator 2015-10-27T17:24:53
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = inst
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
QMAKE_CXX += -fopenmp
QMAKE_LIBS += -fopenmp

SOURCES += main.cpp
