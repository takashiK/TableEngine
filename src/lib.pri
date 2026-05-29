INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}
QT += core gui widgets

win32-msvc {
    INCLUDEPATH += ../support_package/include
    CONFIG(debug, debug|release){
        LIBS += $$shadowed($${PWD})/debug/tengined.lib
        LIBS += ../support_package/debug/archive.lib
        LIBS += icu.lib
        DLL_FILES += ../support_package/debug/archive.dll
    }else{
        LIBS += $$shadowed($${PWD})/release/tengine.lib
        LIBS += ../support_package/release/archive.lib
        LIBS += icu.lib
        DLL_FILES += ../support_package/release/archive.dll
    }

}
