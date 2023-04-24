INCLUDEPATH += $${PWD}/include
DEPENDPATH += $${PWD}/include
QT += core gui widgets


CONFIG(testcase){
    INCLUDEPATH += $${PWD}/src
}

win32-msvc {
    INCLUDEPATH += ../support_package/include
    CONFIG(debug, debug|release){
        LIBS += $$shadowed($${PWD})/debug/viewer_binaryd.lib
    }else{
        LIBS += $$shadowed($${PWD})/release/viewer_binary.lib
    }
}
