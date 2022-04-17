######################################################################
# Automatically generated by qmake (3.1) Sun Feb 17 22:12:19 2019
######################################################################

TEMPLATE = app
CONFIG += console testcase
TARGET = tst_viewer
QT += testlib

win32-msvc{
    CONFIG += windows
}


INCLUDEPATH += .

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += $$files(*.h)
HEADERS += $$files(document/*.h,true)
HEADERS += $$files(picture/*.h,true)

SOURCES += $$files(*.cpp)
SOURCES += $$files(document/*.cpp,true)
SOURCES += $$files(picture/*.cpp,true)

include(../viewer_document/lib.pri)
#include(../viewer_picture/lib.pri)

win32-msvc {
    INCLUDEPATH += ../support_package/include
    QMAKE_CFLAGS += /MP
    QMAKE_CXXFLAGS += /MP
    CONFIG(debug, debug|release){
        LIBS += ../support_package/debug/gmockd.lib
        COPYDIR = debug
    }else{
        LIBS += ../support_package/release/gmock.lib
        COPYDIR = release
    }

    COPYFILES = $${DLL_FILES}
    COPYFILES ~= s,/,\\,g
    COPYDIR   ~= s,/,\\,g

    for(FILE,COPYFILES){
        QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${COPYDIR}$$escape_expand(\n\t))
    }
}
