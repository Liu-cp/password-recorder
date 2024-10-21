QT       += core gui sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    custom_widget/customcombobox.cpp \
    database/database.cpp \
    main.cpp \
    ui/login.cpp \
    ui/signin.cpp \
    ui/uidatabaseset.cpp \
    ui/uimainwindow.cpp \
    common/uimanager.cpp \
    ui/uipwddetail.cpp

HEADERS += \
    custom_widget/customcombobox.h \
    database/database.h \
    ui/login.h \
    common/public.h \
    ui/signin.h \
    ui/uidatabaseset.h \
    ui/uimainwindow.h \
    common/uimanager.h \
    ui/uipwddetail.h

android {
    SOURCES += \
        database/jni_interface_mysql.cpp
    HEADERS += \
        database/jni_interface_mysql.h
}

FORMS += \
    ui/login.ui \
    ui/signin.ui \
    ui/uidatabaseset.ui \
    ui/uimainwindow.ui \
    ui/uipwddetail.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

DISTFILES += \
    android/src/com/mysql/MysqlConnector.java \
    android/src/com/public_utils/PublicUtils.java \
    createpackage.iss \
    icons/combo_arrow.webp \
    icons/password recorder.webp \
    icons/password-recorder.ico

RESOURCES += \
    icons/icons.qrc
