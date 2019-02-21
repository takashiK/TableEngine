win32-msvc:LIBS += $$shadowed($${PWD})/tengine.lib
unix:LIBS += -L$$shadowed($${PWD}) -ltengine
INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}
CONFIG +=  debug_and_release qt
QT += core gui widgets
