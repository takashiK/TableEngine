INCLUDEPATH += $${PWD}/src

QT += core gui widgets webenginewidgets webchannel core5compat


win32-msvc {
    INCLUDEPATH += ../support_package/include
    CONFIG(debug, debug|release){
        LIBS += $$shadowed($${PWD})/debug/viewer_documentd.lib
    }else{
        LIBS += $$shadowed($${PWD})/release/viewer_document.lib
    }
    LIBS += icu.lib
}
