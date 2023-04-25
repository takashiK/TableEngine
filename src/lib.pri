INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}
QT += core gui widgets webenginecore webenginewidgets webchannel core5compat

win32-msvc {
    INCLUDEPATH += ../support_package/include
    CONFIG(debug, debug|release){
        LIBS += $$shadowed($${PWD})/debug/tengined.lib
        LIBS += ../support_package/debug/archive.lib
        LIBS += ../support_package/debug/libmagic.lib
        LIBS += icu.lib
        DLL_FILES += ../support_package/debug/archive.dll
        DLL_FILES += ../support_package/debug/libmagic.dll
        DLL_FILES += ../support_package/debug/magic.mgc
    }else{
        LIBS += $$shadowed($${PWD})/release/tengine.lib
        LIBS += ../support_package/release/archive.lib
        LIBS += ../support_package/release/libmagic.lib
        LIBS += icu.lib
        DLL_FILES += ../support_package/release/archive.dll
        DLL_FILES += ../support_package/release/libmagic.dll
        DLL_FILES += ../support_package/release/magic.mgc
    }

}
