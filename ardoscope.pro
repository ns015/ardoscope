QT += widgets serialport
requires(qtConfig(combobox))

TARGET = ardoscope
TEMPLATE = app

CONFIG += c++11

QMAKE_LFLAGS += -no-pie

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
