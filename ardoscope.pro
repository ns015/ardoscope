QT += widgets serialport
requires(qtConfig(combobox))

TARGET = arduscope
TEMPLATE = app

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mainwindowaddone.cpp \
    runtimedialog.cpp \
    settingsdialog.cpp

HEADERS += \
    mainwindow.h \
    runtimedialog.h \
    settingsdialog.h

FORMS += \
    mainwindow.ui \
    runtimedialog.ui \
    settingsdialog.ui

RESOURCES += \
    ardoscope.qrc
