#-------------------------------------------------
#
# Project created by QtCreator 2021-08-30T18:08:57
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SonyTOF_client
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32 {
OPENCV_SDK_DIR = D:/opencv
INCLUDEPATH += $$OPENCV_SDK_DIR/include

LIBS += -L$$OPENCV_SDK_DIR/x86/mingw/lib \
	-lopencv_core320        \
        -lopencv_highgui320     \
        -lopencv_imgcodecs320   \
        -lopencv_imgproc320     \
        -lopencv_features2d320  \
        -lopencv_calib3d320
}

unix {
#LIBS += -lopencv_core        \
#        -lopencv_highgui     \
#        -lopencv_imgcodecs   \
#        -lopencv_imgproc     \
#        -lopencv_features2d  \
#        -lopencv_calib3d
CONFIG += link_pkgconfig
PKGCONFIG += opencv4
}

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    imagerthread.cpp \
    filterthread.cpp \
    colorizerthread.cpp \
    startupdialog.cpp \
    splashscreen.cpp

HEADERS += \
        mainwindow.h \
    profile.h \
    imagerthread.h \
    freqmod.h \
    filterthread.h \
    colorizerthread.h \
    startupdialog.h \
    splashscreen.h

FORMS += \
        mainwindow.ui \
    startupdialog.ui

RESOURCES += \
    res/resource.qrc
