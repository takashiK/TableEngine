INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}
QT += core gui widgets

win32-msvc {
    INCLUDEPATH += ../support_package/include
    CONFIG(debug, debug|release){
        LIBS += $$shadowed($${PWD})/debug/tengined.lib
        LIBS += ../support_package/debug/archived.lib
    }else{
        LIBS += $$shadowed($${PWD})/release/tengine.lib
        LIBS += ../support_package/release/archive.lib
    }
}
