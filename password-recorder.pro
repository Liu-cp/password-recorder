QT       += core gui sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    database.cpp \
    main.cpp \
    login.cpp \
    signin.cpp \
    uidatabaseset.cpp \
    uimainwindow.cpp \
    uimanager.cpp \
    uipwddetail.cpp

HEADERS += \
    database.h \
    login.h \
    signin.h \
    uidatabaseset.h \
    uimainwindow.h \
    uimanager.h \
    uipwddetail.h

android {
    SOURCES += \
        jni_interface_mysql.cpp
    HEADERS += \
        jni_interface_mysql.h
}

FORMS += \
    login.ui \
    signin.ui \
    uidatabaseset.ui \
    uimainwindow.ui \
    uipwddetail.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

DISTFILES += \
    android/src/com/MysqlConnector.java
