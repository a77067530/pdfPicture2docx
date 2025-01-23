QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    watermarkdialog.cpp

HEADERS += \
    mainwindow.h \
    watermarkdialog.h

FORMS += \
    mainwindow.ui \
    watermarkdialog.ui


# 指定 poppler-qt6 的头文件和库路径
INCLUDEPATH += $$PWD/poppler_qt6/
LIBS += -L$$PWD/poppler_qt6/ -lpoppler-qt6

# 添加 Qt6 模块
QT += core gui
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
