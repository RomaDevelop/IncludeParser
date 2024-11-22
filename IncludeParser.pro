QT       += core gui widgets

CONFIG += c++17

SOURCES += \
    filesitems.cpp \
    main.cpp \
    includeparser.cpp

HEADERS += \
    filesitems.h \
    includeparser.h

FORMS += \
    includeparser.ui

INCLUDEPATH += ../include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
