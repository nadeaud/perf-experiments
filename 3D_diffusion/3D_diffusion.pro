#-------------------------------------------------
#
# Project created by QtCreator 2015-10-28T10:11:43
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 3D_diffusion
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXX += -fopenmp
QMAKE_LIBS += -fopenmp

SOURCES += main.cpp
