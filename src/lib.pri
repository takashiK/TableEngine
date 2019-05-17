INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}
QT += core gui widgets

win32-msvc {
    INCLUDEPATH += ../support_package/include
    CONFIG(debug, debug|release){
        LIBS += $$shadowed($${PWD})/debug/tengined.lib
        LIBS += ../support_package/debug/archived.lib
        DLL_FILES += ../support_package/debug/archived.dll
        DLL_FILES += ../support_package/debug/liblzmad.dll
        DLL_FILES += ../support_package/debug/zlibd.dll
    }else{
        LIBS += $$shadowed($${PWD})/release/tengine.lib
        LIBS += ../support_package/release/archive.lib
        DLL_FILES += ../support_package/debug/archive.dll
        DLL_FILES += ../support_package/debug/liblzma.dll
        DLL_FILES += ../support_package/debug/zlib.dll
    }

}
