QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    db/Db_original.cpp \
    db/triangulation.cpp \
    lib/sqlite3.c \
    main.cpp \
    mainwindow.cpp \
    schema/schema_original.cpp \
    schema/schema_triang.cpp \
    tabs/history.cpp \
    tabs/maps.cpp \
    tabs/settings.cpp \
    tabs/stats.cpp \
    tabs/time_lapse.cpp

HEADERS += \
    db/Db_original.h \
    db/best_k_mac.h \
    db/statistics.h \
    db/triangulation.h \
    lib/sqlite3.h \
    mainwindow.h \
    schema/helperclass.h \
    schema/mac_time.h \
    schema/schema_original.h \
    schema/schema_triang.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    configfile.py \
    mqtt.py \
    schema.sql
