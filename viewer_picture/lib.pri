INCLUDEPATH += $${PWD}/include
DEPENDPATH += $${PWD}/include
QT += core gui widgets

win32-msvc {
    INCLUDEPATH += ../support_package/include
    CONFIG(debug, debug|release){
        LIBS += $$shadowed($${PWD})/debug/viewer_pictured.lib
    }else{
        LIBS += $$shadowed($${PWD})/release/viewer_picture.lib
    }

}
